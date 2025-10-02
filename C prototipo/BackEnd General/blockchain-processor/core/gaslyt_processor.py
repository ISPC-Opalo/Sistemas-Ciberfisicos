"""
Procesador principal para GASLYT blockchain
"""
import structlog
from datetime import datetime, timedelta
from typing import List, Dict, Optional
import hashlib
import json
import time
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

from ..app.config import config
from ..db.database import blockchain_repo, db_manager
from ..blocks.web3_client import ethereum_client, gaslyt_contract

logger = structlog.get_logger(__name__)

class DataProcessor:
    """Procesador de datos InfluxDB"""
    
    def __init__(self):
        self.client = InfluxDBClient(
            url=config.influxdb.url,
            token=config.influxdb.token,
            org=config.influxdb.org
        )
        self.write_api = self.client.write_api(write_options=SYNCHRONOUS)
    
    def get_daily_alarms(self, fecha: datetime) -> List[Dict]:
        """Obtener alarmas del día desde InfluxDB"""
        try:
            start_time = fecha.replace(hour=0, minute=0, second=0, microsecond=0)
            end_time = fecha.replace(hour=23, minute=59, second=59, microsecond=999999)
            
            # Consulta Flux
            query = f'''
            from(bucket: "{config.influxdb.bucket}!")
                |> range(start: {start_time.strftime('%Y-%m-%dT%H:%M:%S.%fZ')}, 
                        stop: {end_time.strftime('%Y-%m-%dT%H:%M:%S.%fZ')})
                |> filter(fn: (r) => r._measurement == "gaslyt_measurements")
                |> filter(fn: (r) => r._field == "concentracion")
                |> filter(fn: (r) => r.measurement_type == "alarm")
                |> group(columns: ["idDispositivo"])
                |> aggregateWindow(every: 1d, fn: count, createEmpty: false)
                |> yield(name: "count_alarms")
            
            from(bucket: "{config.influxdb.bucket}")
                |> range(start: {start_time.strftime('%Y-%m-%dT%H:%M:%S.%fZ')}, 
                        stop: {end_time.strftime('%Y-%m-%dT%H:%M:%S.%fZ')})
                |> filter(fn: (r) => r._measurement == "gaslyt_measurements")
                |> filter(fn: (r) => r._field == "concentracion")
                |> filter(fn: (r) => r.measurement_type == "alarm")
                |> group(columns: ["idDispositivo"])
                |> aggregateWindow(every: 1d, fn: max, createEmpty: false)
                |> yield(name: "max_concentration")
            '''
            
            result = self.client.query_api().query(query)
            
            alarm_summary = {}
            for table in result:
                for row in table.records:
                    device_id = row.values.get('idDispositivo')
                    if device_id:
                        if device_id not in alarm_summary:
                            alarm_summary[device_id] = {
                                'id_dispositivo': device_id,
                                'cantidad_alarmas': 0,
                                'concentracion_maxima': 0.0,
                                'fecha': fecha
                            }
                        
                        if row.values.get('_measurement') == "count_alarms":
                            alarm_summary[device_id]['cantidad_alarmas'] = row.values.get('_value', 0)
                        elif row.values.get('_measurement') == "max_concentration":
                            alarm_summary[device_id]['concentracion_maxima'] = row.values.get('_value', 0.0)
            
            logger.info("Daily alarms retrieved successfully", 
                       fecha=fecha.strftime('%Y-%m-%d'),
                       devices_count=len(alarm_summary))
            
            return list(alarm_summary.values())
            
        except Exception as e:
            logger.error("Error getting daily alarms", error=str(e), fecha=fecha.strftime('%Y-%m-%d'))
            return []
    
    def close(self):
        """Cerrar conexión"""
        self.client.close()

class BatchProcessor:
    """Procesador de lotes para blockchain"""
    
    def __init__(self):
        self.data_processor = DataProcessor()
        self.retry_count = 0
        self.max_retries = config.processing.max_retries
    
    def process_daily_batch(self, fecha: datetime = None) -> Dict:
        """Procesar lote diario de alarmas"""
        if fecha is None:
            fecha = datetime.now() - timedelta(days=1)  # Procesar día anterior
        
        batch_id = self._generate_batch_id()
        start_time = time.time()
        
        logger.info("Starting daily batch processing", 
                   batch_id=batch_id,
                   fecha=fecha.strftime('%Y-%m-%d'))
        
        try:
            # 1. Obtener alarmas del día
            alarms = self.data_processor.get_daily_alarms(fecha)
            if not alarms:
                logger.info("No alarms found for date", fecha=fecha.strftime('%Y-%m-%d'))
                return self._create_batch_result(batch_id, fecha, [], 'completed', 0, 0, 0, 0)
            
            # 2. Procesar en lotes
            batches = self._create_batches(alarms)
            processed_txs = []
            failed_txs = []
            
            for batch_idx, batch in enumerate(batches):
                logger.info("Processing batch", 
                           batch_id=batch_id,
                           batch_index=batch_idx + 1,
                           batch_size=len(batch))
                
                try:
                    tx_hashes = self._process_batch(batch, fecha)
                    processed_txs.extend(tx_hashes)
                    
                except Exception as e:
                    logger.error("Batch processing failed", 
                               batch_id=batch_id,
                               batch_index=batch_idx + 1,
                               error=str(e))
                    failed_txs.append({'batch': batch, 'error': str(e)})
            
            # 3. Generar métricas del lote
            processing_time = time.time() - start_time
            gas_metrics = self._calculate_batch_gas_metrics(processed_txs)
            
            # 4. Registrar resultado del lote
            batch_result = {
                'batch_id': batch_id,
                'fecha_procesamiento': datetime.utcnow(),
                'fecha_datos': fecha,
                'registros_count': len(alarms),
                'registros_exitosos': len(processed_txs),
                'registros_fallidos': len(failed_txs),
                'gas_total_usado': gas_metrics['total_gas_used'],
                'gas_costo_total_eth': gas_metrics['total_cost_eth'],
                'tiempo_procesamiento_segundos': processing_time,
                'estado': 'completed' if not failed_txs else 'partial_failure'
            }
            
            logger.info("Batch processing completed", 
                       batch_id=batch_id,
                       **batch_result)
            
            return batch_result
            
        except Exception as e:
            logger.error("Critical error in batch processing", 
                        batch_id=batch_id,
                        error=str(e))
            return self._create_batch_result(batch_id, fecha, [], 'failed', 0, 0, 0, len(alarms))
    
    def _generate_batch_id(self) -> str:
        """Generar ID único para el lote"""
        return f"batch_{datetime.now().strftime('%Y%m%d_%H%M%S')}_{hashlib.md5(str(time.time()).encode()).hexdigest()[:8]}"
    
    def _create_batches(self, alarms: List[Dict]) -> List[List[Dict]]:
        """Crear lotes de alarmas"""
        batch_size = config.processing.batch_size
        batches = []
        
        for i in range(0, len(alarms), batch_size):
            batch = alarms[i:i + batch_size]
            batches.append(batch)
        
        logger.info("Batches created", 
                   total_alarms=len(alarms),
                   batch_size=batch_size,
                   number_of_batches=len(batches))
        
        return batches
    
    def _process_batch(self, batch: List[Dict], fecha: datetime) -> List[str]:
        """Procesar un lote individual"""
        tx_hashes = []
        
        for alarm in batch:
            try:
                # 1. Generar hash de verificación
                hash_datos = self._generate_data_hash(alarm, fecha)
                
                # 2. Registrar en blockchain
                tx_hash, tx_info = gaslyt_contract.register_daily_alarm(
                    fecha=int(fecha.timestamp()),
                    id_dispositivo=alarm['id_dispositivo'],
                    id_cliente=alarm.get('id_cliente', 'unknown'),
                    cantidad_alarmas=alarm['cantidad_alarmas'],
                    concentracion_max=int(alarm['concentracion_maxima']),
                    hash_datos=hash_datos
                )
                
                # 3. Registrar transacción en DB
                registry_id = blockchain_repo.insert_registry({
                    'tx_hash': tx_hash,
                    'fecha_registro': datetime.utcnow(),
                    'id_dispositivo': alarm['id_dispositivo'],
                    'id_cliente': alarm.get('id_cliente', 'unknown'),
                    'cantidad_alarmas': alarm['cantidad_alarmas'],
                    'concentracion_maxima': alarm['concentracion_maxima'],
                    'datos_hash': hash_datos,
                    'datos_json': alarm
                })
                
                # 4. Registrar transacción blockchain
                transaction_id = blockchain_repo.insert_transaction({
                    'tx_hash': tx_hash,
                    'block_number': None,
                    'gas_used': tx_info['gas_limit'],
                    'gas_price': tx_info['gas_price'],
                    'gas_cost_eth': ethereum_client.convert_wei_to_eth(tx_info['gas_limit'] * tx_info['gas_price']),
                    'confirmation_time': None,
                    'status': 'pending',
                    'retry_count': 0
                })
                
                tx_hashes.append(tx_hash)
                
                logger.info("Alarm registered successfully", 
                           dispositivo_id=alarm['id_dispositivo'],
                           tx_hash=tx_hash,
                           registry_id=registry_id,
                           transaction_id=transaction_id)
                
            except Exception as e:
                logger.error("Error processing alarm", 
                           alarm=alarm,
                           error=str(e))
                
                # Registrar error
                blockchain_repo.insert_error({
                    'error_type': 'alarm_processing',
                    'error_message': str(e),
                    'tx_hash': None,
                    'retry_attempt': 0,
                    'extra_data': {'alarm_data': alarm}
                })
        
        return tx_hashes
    
    def _generate_data_hash(self, alarm: Dict, fecha: datetime) -> str:
        """Generar hash de verificación para los datos"""
        data_string = json.dumps({
            'fecha': fecha.strftime('%Y-%m-%d'),
            'id_dispositivo': alarm['id_dispositivo'],
            'cantidad_alarmas': alarm['cantidad_alarmas'],
            'concentracion_maxima': alarm['concentracion_maxima'],
            'timestamp': datetime.utcnow().isoformat()
        }, sort_keys=True)
        
        return '0x' + hashlib.sha256(data_string.encode()).hexdigest()
    
    def _calculate_batch_gas_metrics(self, tx_hashes: List[str]) -> Dict:
        """Calcular métricas de gas para el lote"""
        total_gas_used = 0
        total_cost_eth = 0.0
        gas_prices = []
        
        for tx_hash in tx_hashes:
            try:
                tx_status = ethereum_client.get_transaction_status(tx_hash)
                if tx_status.get('confirmed'):
                    total_gas_used += tx_status['gas_used']
                    gas_prices.append(tx_status.get('gas_price', 0))
            except Exception as e:
                logger.error("Error getting transaction metrics", 
                           tx_hash=tx_hash,
                           error=str(e))
        
        if gas_prices:
            avg_gas_price = sum(gas_prices) / len(gas_prices)
            total_cost_eth = ethereum_client.convert_wei_to_eth(total_gas_used * avg_gas_price)
        
        return {
            'total_gas_used': total_gas_used,
            'total_cost_eth': total_cost_eth,
            'avg_gas_price': gas_prices[0] if gas_prices else 0,
            'transaction_count': len(tx_hashes)
        }
    
    def _create_batch_result(self, batch_id: str, fecha: datetime, 
                           tx_hashes: List[str], status: str,
                           gas_used: int, cost_eth: float, 
                           successful: int, failed: int) -> Dict:
        """Crear resultado del lote"""
        return {
            'batch_id': batch_id,
            'fecha_procesamiento': datetime.utcnow(),
            'fecha_datos': fecha,
            'registros_count': successful + failed,
            'registros_exitosos': successful,
            'registros_fallidos': failed,
            'gas_total_usado': gas_used,
            'gas_costo_total_eth': cost_eth,
            'estado': status
        }
    
    def cleanup(self):
        """Limpiar recursos"""
        self.data_processor.close()

class BlockchainMonitor:
    """Monitor de operaciones blockchain"""
    
    def __init__(self):
        self.client = ethereum_client
    
    def check_pending_transactions(self) -> Dict:
        """Verificar transacciones pendientes"""
        try:
            pending_txs = blockchain_repo.get_pending_transactions()
            
            confirmed_count = 0
            failed_count = 0
            
            for tx in pending_txs:
                status = self.client.get_transaction_status(tx['tx_hash'])
                
                if status.get('confirmed'):
                    blockchain_repo.update_transaction_status(
                        tx['tx_hash'], 'confirmed',
                        status.get('block_number'),
                        status.get('gas_used'),
                        status.get('confirmation_time')
                    )
                    confirmed_count += 1
                    
                elif status.get('status') == 'error':
                    blockchain_repo.update_transaction_status(
                        tx['tx_hash'], 'failed',
                        error_message=status.get('error')
                    )
                    failed_count += 1
            
            result = {
                'checked_count': len(pending_txs),
                'confirmed_count': confirmed_count,
                'failed_count': failed_count,
                'still_pending': len(pending_txs) - confirmed_count - failed_count
            }
            
            logger.info("Pending transactions check completed", **result)
            return result
            
        except Exception as e:
            logger.error("Error checking pending transactions", error=str(e))
            return {'error'. str(e)}
    
    def get_network_health(self) -> Dict:
        """Obtener salud de la red"""
        try:
            network_info = self.client.get_network_info()
            current_block = self.client.get_current_block()
            
            health_data = {
                'fecha_check': datetime.utcnow(),
                'network_name': config.ethereum.network,
                'rpc_endpoint_status': 'up' if network_info else 'down',
                'latest_block': current_block,
                'block_time_avg': network_info.get('block_time', 12.0),
                'is_synced': network_info.get('is_synced', False),
                'recommended_gas_price': self.client.get_recommended_gas_price()
            }
            
            # Registrar en base de datos
            blockchain_repo.insert_metrics({
                'fecha_metrica': datetime.utcnow(),
                'network_name': config.ethereum.network,
                **health_data
            })
            
            return health_data
            
        except Exception as e:
            logger.error("Error getting network health", error=str(e))
            return {'error': str(e)}

# Funciones principales
def process_daily_alarms(fecha: datetime = None) -> Dict:
    """Función principal para procesar alarmas diarias"""
    processor = BatchProcessor()
    try:
        return processor.process_daily_batch(fecha)
    finally:
       处理器.cleanup()

def monitor_blockchain():
    """Función de monitoreo de blockchain"""
    monitor = BlockchainMonitor()
    monitor_result = monitor.check_pending_transactions()
    health_result = monitor.get_network_health()
    
    return {
        'monitoring': monitor_result,
        'health': health_result
    }
