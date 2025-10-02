"""
Modelos de base de datos para el procesador blockchain GASLYT
"""
from sqlalchemy import Column, Integer, String, DateTime, Float, Boolean, Text, BigInteger, Index
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.dialects.mysql import JSON, LONGTEXT
from datetime import datetime
import uuid

Base = declarative_base()

class BlockchainTransaction(Base):
    """Registro de transacciones blockchain"""
    __tablename__ = 'blockchain_transactions'
    
    id = Column(Integer, primary_key=True, autoincrement=True)
    tx_hash = Column(String(66), unique=True, nullable=False, index=True)
    block_number = Column(BigInteger, nullable=True)
    gas_used = Column(BigInteger, nullable=False)
    gas_price = Column(BigInteger, nullable=False)
    gas_cost_eth = Column(Float, nullable=False)
    confirmation_time = Column(Integer, nullable=True)  # segundos para confirmar
    status = Column(String(20), nullable=False, default='pending')  # pending, confirmed, failed
    retry_count = Column(Integer, default=0)
    error_message = Column(Text, nullable=True)
    created_at = Column(DateTime, default=datetime.utcnow)
    confirmed_at = Column(DateTime, nullable=True)
    
    Index('idx_status', 'status')
    Index('idx_created_at', 'created_at')

class BlockchainRegistry(Base):
    """Registro de alarmas en blockchain"""
    __tablename__ = 'blockchain_registry'
    
    id = Column(Integer, primary_key=True, autoincrement=True)
    tx_hash = Column(String(66), nullable=False, index=True)
    fecha_registro = Column(DateTime, nullable=False, index=True)
    id_dispositivo = Column(String(100), nullable=False, index=True)
    id_cliente = Column(String(100), nullable=False, index=True)
    cantidad_alarmas = Column(Integer, nullable=False)
    concentracion_maxima = Column(Float, nullable=False)
    datos_hash = Column(String(66), nullable=False)  # Hash de los datos
    datos_json = Column(JSON, nullable=True)  # Datos completos para verificación
    verificado = Column(Boolean, default=False)
    
    created_at = Column(DateTime, default=datetime.utcnow)
    
    Index('idx_dispositivo_fecha', 'id_dispositivo', 'fecha_registro')
    Index('idx_cliente_fecha', 'id_cliente', 'fecha_registro')

class BlockchainMetrics(Base):
    """Métricas de la blockchain"""
    __tablename__ = 'blockchain_metrics'
    
    id = Column(Integer, primary_key=True, autoincrement=True)
    fecha_metrica = Column(DateTime, nullable=False, index=True)
    network_name = Column(String(50), nullable=False)
    
    # Métricas de transacciones
    tx_count_total = Column(Integer, default=0)
    tx_count_pending = Column(Integer, default=0)
    tx_count_confirmed = Column(Integer, default=0)
    tx_count_failed = Column(Integer, default=0)
    
    # Métricas de gas
    gas_price_avg = Column(Float, nullable=True)
    gas_price_min = Column(Float, nullable=True)
    gas_price_max = Column(Float, nullable=True)
    gas_used_total = Column(BigInteger, default=0)
    gas_cost_total_eth = Column(Float, default=0.0)
    
    # Métricas de rendimiento
    avg_confirmation_time = Column(Float, nullable=True)  # segundos
    blocks_mined = Column(Integer, default=0)
    network_difficulty = Column(BigInteger, nullable=True)
    
    # Métricas de negocio
    dispositivos_registrados = Column(Integer, default=0)
    alarmas_registradas = Column(Integer, default=0)
    
    created_at = Column(DateTime, default=datetime.utcnow)
    
    Index('idx_fecha_network', 'fecha_metrica', 'network_name')

class NetworkHealth(Base):
    """Salud de la red blockchain"""
    __tablename__ = 'network_health'
    
    id = Column(Integer, primary_key=True, autoincrement=True)
    fecha_check = Column(DateTime, nullable=False, index=True)
    network_name = Column(String(50), nullable=False)
    
    # Conectividad
    rpc_endpoint_status = Column(String(20), nullable=False)  # up, down, slow
    rpc_response_time = Column(Float, nullable=True)  # ms
    websocket_status = Column(String(20), nullable=True)
    
    # Estado de la red
    latest_block = Column(BigInteger, nullable=True)
    block_time_avg = Column(Float, nullable=True)  # segundos
    network_hashrate = Column(String(50), nullable=True)
    
    # Sinchronización
    is_synced = Column(Boolean, nullable=True)
    sync_percentage = Column(Float, nullable=True)
    
    # Configuración de transacciones
    recommended_gas_price = Column(BigInteger, nullable=True)
    max_gas_limit = Column(BigInteger, nullable=True)
    
    created_at = Column(DateTime, default=datetime.utcnow)
    
    Index('idx_fecha_check', 'fecha_check')

class BlockchainError(Base):
    """Log de errores del procesador blockchain"""
    __tablename__ = 'blockchain_errors'
    
    id = Column(Integer, primary_key=True, autoincrement=True)
    error_uuid = Column(String(36), default=lambda: str(uuid.uuid4()), unique=True)
    fecha_ocurrencia = Column(DateTime, nullable=False, index=True, default=datetime.utcnow)
    
    # Información del error
    error_type = Column(String(100), nullable=False)  # connection, gas, contract, etc
    error_message = Column(Text, nullable=False)
    error_code = Column(String(20), nullable=True)
    stack_trace = Column(Text, nullable=True)
    
    # Contexto del error
    tx_hash = Column(String(66), nullable=True)
    block_number = Column(BigInteger, nullable=True)
    gas_price = Column(BigInteger, nullable=True)
    retry_attempt = Column(Integer, default=0)
    
    # Resolución
    resolved = Column(Boolean, default=False)
    resolution_method = Column(String(100), nullable=True)
    resolved_at = Column(DateTime, nullable=True)
    
    # Información adicional
    client_ip = Column(String(45), nullable=True)
    user_agent = Column(String(255), nullable=True)
    extra_data = Column(JSON, nullable=True)
    
    Index('idx_error_type', 'error_type')
    Index('idx_fecha_ocurrencia', 'fecha_ocurrencia')
    Index('idx_resolved', 'resolved')

class ProcessingBatch(Base):
    """Lotes de procesamiento"""
    __tablename__ = 'processing_batches'
    
    id = Column(Integer, primary_key=True, autoincrement=True)
    batch_uuid = Column(String(36), default=lambda: str(uuid.uuid4()), unique=True)
    fecha_procesamiento = Column(DateTime, nullable=False, index=True)
    
    # Información del lote
    registros_count = Column(Integer, nullable=False)
    batch_size = Column(Integer, nullable=False)
    fecha_datos = Column(DateTime, nullable=False)
    
    # Resultados
    tx_hashes = Column(JSON, nullable=True)  # Lista de hashes de transacciones
    registros_exitosos = Column(Integer, default=0)
    registros_fallidos = Column(Integer, default=0)
    
    # Métricas
    gas_total_usado = Column(BigInteger, default=0)
    gas_costo_total_eth = Column(Float, default=0.0)
    tiempo_procesamiento_segundos = Column(Float, nullable=True)
    
    # Estado
    estado = Column(String(20), default='processing')  # processing, completed, failed
    
    created_at = Column(DateTime, default=datetime.utcnow)
    completed_at = Column(DateTime, nullable=True)
    
    Index('idx_fecha_procesamiento', 'fecha_procesamiento')
    Index('idx_estado', 'estado')

class BlockchainConfig(Base):
    """Configuración de la blockchain"""
    __tablename__ = 'blockchain_config'
    
    id = Column(Integer, primary_key=True, autoincrement=True)
    config_key = Column(String(100), unique=True, nullable=False)
    config_value = Column(LONGTEXT, nullable=True)
    config_type = Column(String(20), default 'string')  # string, number, boolean, json
    description = Column(Text, nullable=True)
    
    # Metadatos
    created_at = Column(DateTime, default=datetime.utcnow)
    updated_at = Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)
    updated_by = Column(String(100), nullable=True)
    
    Index('idx_config_key', 'config_key')
