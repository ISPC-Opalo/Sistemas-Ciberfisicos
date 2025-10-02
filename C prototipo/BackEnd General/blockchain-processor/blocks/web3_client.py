"""
Cliente Web3 para interacción con la blockchain Ethereum
"""
import structlog
from web3 import Web3
from web3.exceptions import ContractLogicError, TransactionNotFound, TimeExhausted
from eth_account import Account
from typing import Dict, List, Optional, Tuple
from datetime import datetime
import time
from tenacity import retry, stop_after_attempt, wait_exponential
from ..app.config import config

logger = structlog.get_logger(__name__)

class EthereumClient:
    """Cliente para interactuar con Ethereum"""
    
    def __init__(self):
        self.w3 = Web3(Web3.HTTPProvider(config.ethereum.rpc_url))
        self.account = Account.from_key(config.ethereum.private_key)
        self.address = self.account.address
        
        # Validar conexión
        if not self.w3.is_connected():
            raise ConnectionError(f"No se pudo conectar a la red {config.ethereum.network}")
        
        logger.info("Ethereum client initialized", 
                   network=config.ethereum.network,
                   address=self.address)
    
    def get_current_block(self) -> int:
        """Obtener el bloque actual"""
        try:
            return self.w3.eth.block_number
        except Exception as e:
            logger.error("Error getting current block", error=str(e))
            return 0
    
    def get_network_info(self) -> Dict:
        """Obtener información de la red"""
        try:
            current_block = self.get_current_block()
            gas_price = self.get_current_gas_price()
            
            return {
                "network_id": self.w3.net.version,
                "current_block": current_block,
                "gas_price_gwei": Web3.from_wei(gas_price, 'gwei'),
                "block_time": self.estimate_block_time(),
                "is_synced": self.is_synced()
            }
        except Exception as e:
            logger.error("Error getting network info", error=str(e))
            return {}
    
    def get_current_gas_price(self) -> int:
        """Obtener el precio actual del gas"""
        try:
            return self.w3.eth.gas_price
        except Exception as e:
            logger.error("Error getting gas price", error=str(e))
            return config.ethereum.gas_price
    
    @retry(stop=stop_after_attempt(3), wait=wait_exponential(multiplier=1, min=4, max=10))
    def send_transaction(self, tx_data: Dict) -> Tuple[str, Dict]:
        """Enviar transacción a la red"""
        try:
            # Estimar gas si no se proporciona
            if 'gas' not in tx_data:
                gas_estimate = self.w3.eth.estimate_gas(tx_data)
                tx_data['gas'] = int(gas_estimate * 1.2)  # 20% buffer
            
            # Usar gas price recomendado si no se proporciona
            if 'gasPrice' not in tx_data:
                recommended_gas_price = self.get_recommended_gas_price()
                tx_data['gasPrice'] = recommended_gas_price
            
            # Enviar transacción
            signed_tx = self.w3.eth.account.sign_transaction(tx_data, config.ethereum.private_key)
            tx_hash = self.w3.eth.send_raw_transaction(signed_tx.rawTransaction)
            
            tx_hash_hex = tx_hash.hex()
            
            logger.info("Transaction sent successfully", 
                       tx_hash=tx_hash_hex,
                       gas_limit=tx_data['gas'],
                       gas_price=tx_data['gasPrice'])
            
            return tx_hash_hex, {
                'gas_limit': tx_data['gas'],
                'gas_price': tx_data['gasPrice'],
                'value': tx_data.get('value', 0),
                'to': tx_data.get('to', ''),
                'data': tx_data.get('data', '')
            }
            
        except ContractLogicError as e:
            error_msg = f"Contract logic error: {str(e)}"
            logger.error(error_msg, tx_data=tx_data)
            raise
            
        except Exception as e:
            error_msg = f"Error sending transaction: {str(e)}"
            logger.error(error_msg, tx_data=tx_data)
            raise
    
    def wait_for_transaction_receipt(self, tx_hash: str, timeout: int = 300) -> Dict:
        """Esperar confirmación de transacción"""
        start_time = time.time()
        
        try:
            receipt = self.w3.eth.wait_for_transaction_receipt(tx_hash, timeout=timeout)
            confirmation_time = int(time.time() - start_time)
            
            logger.info("Transaction confirmed", 
                       tx_hash=tx_hash,
                       block_number=receipt.blockNumber,
                       gas_used=receipt.gasUsed,
                       confirmation_time=confirmation_time)
            
            return {
                'block_number': receipt.blockNumber,
                'gas_used': receipt.gasUsed,
                'status': receipt.status,
                'confirmation_time': confirmation_time
            }
            
        except TimeExhausted:
            error_msg = f"Transaction timeout after {timeout}s"
            logger.error(error_msg, tx_hash=tx_hash)
            return {'status': 'timeout', 'error': error_msg}
            
        except Exception как e:
            error_msg = f"Error waiting for receipt: {str(e)}"
            logger.error(error_msg, tx_hash=tx_hash)
            return {'status': 'error', 'error': error_msg}
    
    def get_transaction_status(self, tx_hash: str) -> Dict:
        """Obtener estado de una transacción"""
        try:
            receipt = self.w3.eth.get_transaction_receipt(tx_hash)
            
            return {
                'status': receipt.status,
                'block_number': receipt.blockNumber,
                'gas_used': receipt.gasUsed,
                'confirmed': True
            }
            
        except TransactionNotFound:
            return {'status': 'pending', 'confirmed': False}
            
        except Exception as e:
            logger.error("Error getting transaction status", error=str(e), tx_hash=tx_hash)
            return {'status': 'error', 'error': str(e)}
    
    def get_recommended_gas_price(self) -> int:
        """Obtener precio recomendado del gas"""
        try:
            current_price = self.get_current_gas_price()
            # Agregar 10% para asegurar que la transacción sea incluida
            recommended = int(current_price * 1.1)
            
            logger.debug("Recommended gas price calculated", 
                        current_price=current_price,
                        recommended=recommended)
            
            return recommended
            
        except Exception as e:
            logger.error("Error calculating recommended gas price", error=str(e))
            return config.ethereum.gas_price
    
    def estimate_blocks_until_confirmation(self, tx_hash: str) -> int:
        """Estimar bloques hasta confirmación"""
        try:
            tx = self.w3.eth.get_transaction(tx_hash)
            current_block = self.get_current_block()
            
            if tx.blockNumber is not None:
                return 0  # Ya confirmada
            
            # Estimación basada en el tiempo promedio de bloque
            avg_block_time = self.estimate_block_time()
            blocks_to_wait = max(1, int(300 / avg_block_time))  # Esperar 5 minutos
            
            logger.debug("Estimated blocks until confirmation", 
                        tx_hash=tx_hash,
                        blocks=blocks_to_wait)
            
            return blocks_to_wait
            
        except Exception as e:
            logger.error("Error estimating confirmation", error=str(e), tx_hash=tx_hash)
            return 1
    
    def estimate_block_time(self) -> float:
        """Estimar tiempo promedio entre bloques"""
        try:
            current_block = self.get_current_block()
            if current_block < 2:
                return 12.0  # Default para Ethereum
            
            recent_block = self.w3.eth.get_block(current_block - 1)
            older_block = self.w3.eth.get_block(max(0, current_block - 10))
            
            time_diff = recent_block['timestamp'] - older_block['timestamp']
            block_diff = recent_block['number'] - older_block['number']
            
            if block_diff > 0:
                avg_block_time = time_diff / block_diff
                logger.debug("Calculated average block time", 
                           blocks=block_diff,
                           time_seconds=time_diff,
                           avg_block_time=avg_block_time)
                return float(avg_block_time)
            
            return 12.0
            
        except Exception as e:
            logger.error("Error estimating block time", error=str(e))
            return 12.0
    
    def is_synced(self) -> bool:
        """Verificar si el nodo está sincronizado"""
        try:
            syncing = self.w3.eth.syncing
            return syncing is False
            
        except Exception as e:
            logger.error("Error checking sync status", error=str(e))
            return False
    
    def get_balance(self) -> int:
        """Obtener balance de la cuenta"""
        try:
            return self.w3.eth.get_balance(self.address)
        except Exception as e:
            logger.error("Error getting balance", error=str(e))
            return 0
    
    def convert_wei_to_eth(self, wei_value: int) -> float:
        """Convertir Wei a ETH"""
        return Web3.from_wei(wei_value, 'ether')
    
    def convert_gwei_to_wei(self, gwei_value: float) -> int:
        """Convertir Gwei a Wei"""
        return Web3.to_wei(gwei_value, 'gwei')

class GaslytContract:
    """Interfaz para el contrato inteligente Gaslyt"""
    
    CONTRACT_ABI = [
        {
            "inputs": [
                {"internalType": "uint256", "name": "_fecha", "type": "uint256"},
                {"internalType": "string", "name": "_idDispositivo", "type": "string"},
                {"internalType": "string", "name": "_idCliente", "type": "string"},
                {"internalType": "uint256", "name": "_cantidadAlarmas", "type": "uint256"},
                {"internalType": "uint256", "name": "_concentracionMax", "type": "uint256"},
                {"internalType": "bytes32", "name": "_hashDatos", "type": "bytes32"}
            ],
            "name": "registrarAlarmaDiaria",
            "outputs": [
                {"internalType": "bytes32", "name": "", "type": "bytes32"}
            ],
            "stateMutability": "nonpayable",
            "type": "function"
        },
        {
            "inputs": [],
            "name": "owner",
            "outputs": [
                {"internalType": "address", "name": "", "type": "address"}
            ],
            "stateMutability": "view",
            "type": "function"
        }
    ]
    
    def __init__(self, web3_client: EthereumClient):
        self.web3_client = web3_client
        self.w3 = web3_client.w3
        self.contract = self.w3.eth.contract(
            address=config.ethereum.contract_address,
            abi=self.CONTRACT_ABI
        )
        
        logger.info("Gaslyt contract initialized", 
                   contract_address=config.ethereum.contract_address)
    
    def register_daily_alarm(self, fecha: int, id_dispositivo: str, 
                           id_cliente: str, cantidad_alarmas: int, 
                           concentracion_max: float, hash_datos: str) -> Tuple[str, Dict]:
        """Registrar alarma diaria en el contrato"""
        try:
            # Convertir hash a bytes32
            hash_bytes = bytes.fromhex(hash_datos[2:] if hash_datos.startswith('0x') else hash_datos)
            
            # Construir transacción
            tx = self.contract.functions.registrarAlarmaDiaria(
                fecha, id_dispositivo, id_cliente, cantidad_alarmas, 
                concentracion_max, hash_bytes
            ).build_transaction({
                'from': self.web3_client.address,
                'nonce': self.w3.eth.get_transaction_count(self.web3_client.address)
            })
            
            # Enviar transacción
            tx_hash, tx_info = self.web3_client.send_transaction(tx)
            
            logger.info("Daily alarm registration initiated", 
                       tx_hash=tx_hash,
                       dispositivo=id_dispositivo,
                       cantidad_alarmas=cantidad_alarmas)
            
            return tx_hash, tx_info
            
        except Exception as e:
            error_msg = f"Error registering daily alarm: {str(e)}"
            logger.error(error_msg, 
                        fecha=fecha,
                        dispositivo=id_dispositivo)
            raise Exception(error_msg)

# Instancias globales
ethereum_client = EthereumClient()
gaslyt_contract = GaslytContract(ethereum_client)
