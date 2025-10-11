"""
Cliente Redis para cache y colas del procesador blockchain GASLYT
"""
import redis
import json
import structlog
from ..app.config import config

logger = structlog.get_logger(__name__)

class RedisClient:
    """Cliente Redis para cache y colas"""
    
    def __init__(self):
        self.client = redis.Redis(
            host=config.redis.host,
            port=config.redis.port,
            password=config.redis.password,
            db=config.redis.database,
            decode_responses=config.redis.decode_responses
        )
        
        # Verificar conexión
        try:
            self.client.ping()
            logger.info("Redis connection established")
        except Exception as e:
            logger.error("Redis connection failed", error=str(e))
            raise
    
    def cache_transaction_status(self, tx_hash: str, status: dict, ttl: int = 3600):
        """Cachear estado de transacción"""
        key = f"tx_status:{tx_hash}"
        self.client.setex(key, ttl, json.dumps(status))
    
    def get_transaction_status(self, tx_hash: str) -> dict:
        """Obtener estado de transacción desde cache"""
        key = f"tx_status:{tx_hash}"
        data = self.client.get(key)
        return json.loads(data) if data else None
    
    def queue_batch_processing(self, batch_data: dict):
        """Encolar lote para procesamiento"""
        self.client.lpush("batch_queue", json.dumps(batch_data))
    
    def get_pending_batches(self) -> list:
        """Obtener lotes pendientes"""
        batches = []
        while True:
            batch = self.client.rpop("batch_queue")
            if not batch:
                break
            batches.append(json.loads(batch))
        return batches

# Instancia global
redis_client = RedisClient()