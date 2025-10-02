"""
Configuración y manejo de base de datos para el procesador blockchain
"""
from sqlalchemy import create_engine, text
from sqlalchemy.orm import sessionmaker, Session
from sqlalchemy.exc import SQLAlchemyError
from contextlib import contextmanager
import structlog
from typing import Generator, Optional
from ..config import config
from .models import Base

logger = structlog.get_logger(__name__)

class DatabaseManager:
    """Manejo de la conexión y operaciones de base de datos"""
    
    def __init__(self):
        self.engine = create_engine(
            config.database.connection_string,
            pool_size=20,
            max_overflow=0,
            pool_pre_ping=True,
            pool_recycle=3600,
            echo=False
        )
        self.SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=self.engine)
        
    def create_tables(self):
        """Crear todas las tablas si no existen"""
        try:
            Base.metadata.create_all(self.engine)
            logger.info("Tables created successfully", tables=list(Base.metadata.tables.keys()))
        except SQLAlchemyError as e:
            logger.error("Error creating tables", error=str(e))
            raise
    
    @contextmanager
    def get_session(self) -> Generator[Session, None, None]:
        """Context manager para obtener una sesión de base de datos"""
        session = self.SessionLocal()
        try:
            yield session
            session.commit()
        except Exception as e:
            session.rollback()
            logger.error("Database session error", error=str(e))
            raise
        finally:
            session.close()
    
    def test_connection(self) -> bool:
        """Probar la conexión a la base de datos"""
        try:
            with self.get_session() as session:
                session.execute(text("SELECT 1"))
                logger.info("Database connection test successful")
                return True
        except Exception as e:
            logger.error("Database connection test failed", error=str(e))
            return False
    
    def get_connection_info(self) -> dict:
        """Obtener información de la conexión"""
        return {
            "host": config.database.host,
            "port": config.database.port,
            "database": config.database.database,
            "user": config.database.user,
            "connection_string": config.database.connection_string.replace(config.database.password, "***")
        }

class BlockchainRepository:
    """Repositorio para operaciones específicas de blockchain"""
    
    def __init__(self, db_manager: DatabaseManager):
        self.db_manager = db_manager
    
    def insert_transaction(self, tx_data: dict) -> Optional[int]:
        """Insertar una transacción blockchain"""
        from .models import BlockchainTransaction
        
        try:
            with self.db_manager.get_session() as session:
                transaction = BlockchainTransaction(**tx_data)
                session.add(transaction)
                session.flush()
                transaction_id = transaction.id
                session.commit()
                
                logger.info("Transaction inserted successfully", 
                          tx_id=transaction_id, 
                          tx_hash=tx_data.get('tx_hash'))
                return transaction_id
                
        except Exception as e:
            logger.error("Error inserting transaction", error=str(e), tx_data=tx_data)
            return None
    
    def update_transaction_status(self, tx_hash: str, status: str, 
                                 block_number: int = None, 
                                 gas_used: int = None,
                                 confirmation_time: int = None) -> bool:
        """Actualizar el estado de una transacción"""
        from .models import BlockchainTransaction
        
        try:
            with self.db_manager.get_session() as session:
                transaction = session.query(BlockchainTransaction)\
                                   .filter_by(tx_hash=tx_hash)\
                                   .first()
                
                if not transaction:
                    logger.warning("Transaction not found", tx_hash=tx_hash)
                    return False
                
                transaction.status = status
                
                if block_number:
                    transaction.block_number = block_number
                if gas_used:
                    transaction.gas_used = gas_used
                if confirmation_time:
                    transaction.confirmation_time = confirmation_time
                
                session.commit()
                logger.info("Transaction status updated", 
                          tx_hash=tx_hash, 
                          status=status)
                return True
                
        except Exception as e:
            logger.error("Error updating transaction status", 
                        error=str(e), 
                        tx_hash=tx_hash)
            return False
    
    def insert_registry(self, registry_data: dict) -> Optional[int]:
        """Insertar registro de alarma"""
        from .models import BlockchainRegistry
        
        try:
            with self.db_manager.get_session() as session:
                registry = BlockchainRegistry(**registry_data)
                session.add(registry)
                session.flush()
                registry_id = registry.id
                session.commit()
                
                logger.info("Registry inserted successfully", 
                          registry_id=registry_id, 
                          dispositivo_id=registry_data.get('id_dispositivo'))
                return registry_id
                
        except Exception as e:
            logger.error("Error inserting registry", error=str(e), registry_data=registry_data)
            return None
    
    def insert_error(self, error_data: dict) -> Optional[int]:
        """Insertar registro de error"""
        from .models import BlockchainError
        
        try:
            with self.db_manager.get_session() as session:
                error = BlockchainError(**error_data)
                session.add(error)
                session.flush()
                error_id = error.id
                session.commit()
                
                logger.error("Error logged successfully", 
                          error_id=error_id, 
                          error_type=error_data.get('error_type'))
                return error_id
                
        except Exception as e:
            logger.error("Error logging error", error=str(e), error_data=error_data)
            return None
    
    def insert_metrics(self, metrics_data: dict) -> Optional[int]:
        """Insertar métricas de la blockchain"""
        from .models import BlockchainMetrics
        
        try:
            with self.db_manager.get_session() as session:
                metrics = BlockchainMetrics(**metrics_data)
                session.add(metrics)
                session.flush()
                metrics_id = metrics.id
                session.commit()
                
                logger.info("Metrics inserted successfully", 
                          metrics_id=metrics_id)
                return metrics_id
                
        except Exception as e:
            logger.error("Error inserting metrics", error=str(e), metrics_data=metrics_data)
            return None
    
    def get_pending_transactions(self) -> list:
        """Obtener transacciones pendientes"""
        from .models import BlockchainTransaction
        
        try:
            with self.db_manager.get_session() as session:
                transactions = session.query(BlockchainTransaction)\
                                   .filter_by(status='pending')\
                                   .order_by(BlockchainTransaction.created_at.desc())\
                                   .all()
                
                return [{
                    'id': tx.id,
                    'tx_hash': tx.tx_hash,
                    'gas_used': tx.gas_used,
                    'gas_price': tx.gas_price,
                    'retry_count': tx.retry_count,
                    'created_at': tx.created_at
                } for tx in transactions]
                
        except Exception as e:
            logger.error("Error getting pending transactions", error=str(e))
            return []
    
    def get_failed_transactions(self) -> list:
        """Obtener transacciones fallidas"""
        from .models import BlockchainTransaction
        
        try:
            with self.db_manager.get_session() as session:
                transactions = session.query(BlockchainTransaction)\
                                   .filter_by(status='failed')\
                                   .order_by(BlockchainTransaction.created_at.desc())\
                                   .limit()

                return [{
                    'id': tx.id,
                    'tx_hash': tx.tx_hash,
                    'error_message': tx.error_message,
                    'retry_count': tx.retry_count,
                    'created_at': tx.created_at
                } for tx in transactions]
                
        except Exception as e:
            logger.error("Error getting failed transactions", error=str(e))
            return []

# Instancia global del manager de base de datos
db_manager = DatabaseManager()
blockchain_repo = BlockchainRepository(db_manager)
