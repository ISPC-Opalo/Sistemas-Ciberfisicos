"""
Configuración del procesador blockchain GASLYT
"""
import os
from dataclasses import dataclass
from typing import Dict, Any
from dotenv import load_dotenv

load_dotenv()

@dataclass
class DatabaseConfig:
    """Configuración de base de datos MariaDB"""
    host: str = os.getenv('MARIADB_HOST', 'mariadb')
    port: int = int(os.getenv('MARIADB_PORT', '3306'))
    user: str = os.getenv('MARIADB_USER', 'opalouser')
    password: str = os.getenv('MARIADB_PASSWORD', 'opalopass')
    database: str = os.getenv('MARIADB_DATABASE', 'opalodb')
    
    @property
    def connection_string(self) -> str:
        return f"mysql+pymysql://{self.user}:{self.password}@{self.host}:{self.port}/{self.database}"

@dataclass
class InfluxDBConfig:
    """Configuración de InfluxDB"""
    url: str = os.getenv('INFLUXDB_URL', 'http://influxdb:8086')
    token: str = os.getenv('INFLUXDB_ADMIN_TOKEN', 'superinfluxadminpassword')
    org: str = os.getenv('INFLUXDB_ORG', 'opalo-org')
    bucket: str = os.getenv('INFLUXDB_BUCKET', 'telemetria')

@dataclass
class EthereumConfig:
    """Configuración de Ethereum"""
    network: str = os.getenv('ETH_NETWORK', 'sepolia')
    rpc_url: str = os.getenv('ETH_RPC_URL', 'https://sepolia.infura.io/v3/YOUR_PROJECT_ID')
    private_key: str = os.getenv('ETH_PRIVATE_KEY', '')
    contract_address: str = os.getenv('CONTRACT_ADDRESS', '')
    gas_limit: int = int(os.getenv('ETH_GAS_LIMIT', '500000'))
    gas_price: int = int(os.getenv('ETH_GAS_PRICE', '20000000000'))  # 20 gwei
    
@dataclass
class ProcessingConfig:
    """Configuración de procesamiento"""
    batch_size: int = int(os.getenv('BATCH_SIZE', '10'))
    max_retries: int = int(os.getenv('MAX_RETRIES', '3'))
    retry_delay: int = int(os.getenv('RETRY_DELAY', '5'))
    execution_hour: int = int(os.getenv('EXECUTION_HOUR', '23'))
    execution_minute: int = int(os.getenv('EXECUTION_MINUTE', '59'))
    
@dataclass
class LoggingConfig:
    """Configuración de logging"""
    level: str = os.getenv('LOG_LEVEL', 'INFO')
    format: str = os.getenv('LOG_FORMAT', 'json')
    file_path: str = os.getenv('LOG_FILE_PATH', '/app/logs/blockchain-processor.log')
    
class Config:
    """Configuración principal del sistema"""
    
    def __init__(self):
        self.database = DatabaseConfig()
        self.influxdb = InfluxDBConfig()
        self.ethereum = EthereumConfig()
        self.processing = ProcessingConfig()
        self.logging = LoggingConfig()
        self.timezone = os.getenv('TZ', 'America/Argentina/Cordoba')
        
    def validate(self) -> bool:
        """Valida la configuración"""
        required_vars = [
            ('ETH_PRIVATE_KEY', self.ethereum.private_key),
            ('CONTRACT_ADDRESS', self.ethereum.contract_address),
            ('ETH_RPC_URL', self.ethereum.rpc_url)
        ]
        
        missing_vars = [var for var, value in required_vars if not value]
        
        if missing_vars:
            raise ValueError(f"Variables de entorno faltantes: {', '.join(missing_vars)}")
            
        return True
    
    def get_rpc_endpoints(self) -> Dict[str, str]:
        """Retorna endpoints RPC por red"""
        return {
            'mainnet': 'https://mainnet.infura.io/v3/',
            'sepolia': 'https://sepolia.infura.io/v3/',
            'goerli': 'https://goerli.infura.io/v3/',
            'polygon': 'https://polygon-rpc.com',
            'mumbai': 'https://rpc-mumbai.maticvigil.com',
            'bsc': 'https://bsc-dataseed.binance.org/',
            'bsc-testnet': 'https://data-seed-prebsc-1-s1.binance.org:8545/'
        }
    
    def get_gas_recommendations(self) -> Dict[str, Dict[str, int]]:
        """Retorna recomendaciones de gas por tipo de transacción"""
        return {
            'erc20_transfer': {'limit': 21000, 'price': 20000000000},
            'token_deployment': {'limit': 2000000, 'price': 20000000000},
            'contract_interaction': {'limit': 100000, 'price': 20000000000},
            'gaslyt_register': {'limit': 150000, 'price': 20000000000}
        }

# Instancia global de configuración
config = Config()

# Validar configuración al importar
try:
    config.validate()
except ValueError as e:
    print(f"⚠️  Error de configuración: {e}")
    print("📝 Configure las variables de entorno necesarias en el archivo .env")
