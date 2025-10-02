"""
Cliente principal para el procesador blockchain GASLYT
"""
import structlog
import schedule
import time
import sys
import os
from datetime import datetime, timedelta
from typing import Dict, Optional
import argparse
import logging

# Configurar logging
logging.basicConfig(level=logging.INFO)
logger = structlog.get_logger(__name__)

# Configurar structlog
structlog.configure(
    processors=[
        structlog.stdlib.add_logger_name,
        structlog.stdlib.PositionalArgumentsFormatter(),
        structlog.processors.TimeStamper(fmt="iso"),
        structlog.processors.StackInfoRenderer(),
        structlog.processors.format_exc_info,
        structlog.processors.UnicodeDecoder(),
        structlog.processors.JSONRenderer() if os.getenv('LOG_FORMAT') == 'json' else structlog.dev.ConsoleRenderer(),
    ],
    context_class=dict,
    logger_factory=structlog.stdlib.LoggerFactory(),
    wrapper_class=structlog.stdlib.BoundLogger,
    cache_logger_on_first_use=True,
)

from .config import config
from ..db.database import db_manager
from ..core.gaslyt_processor import process_daily_alarms, monitor_blockchain

def setup_database():
    """Configurar base de datos"""
    try:
        logger.info("Setting up database", config=config.database.get_connection_info())
        
        if not db_manager.test_connection():
            raise Exception("Database connection test failed")
        
        db_manager.create_tables()
        logger.info("Database setup completed successfully")
        return True
        
    except Exception as e:
        logger.error("Database setup failed", error=str(e))
        return False

def run_daily_processing(fecha: Optional[datetime] = None):
    """Ejecutar procesamiento diario"""
    try:
        logger.info("Starting daily processing", 
                   fecha=fecha.strftime('%Y-%m-%d') if fecha else "yesterday")
        
        result = process_daily_alarms(fecha)
        
        logger.info("Daily processing completed", 
                   batch_result=result)
        
        return result
        
    except Exception as e:
        logger.error("Daily processing failed", error=str(e))
        return None

def run_monitoring():
    """Ejecutar monitoreo"""
    try:
        logger.info("Starting blockchain monitoring")
        
        result = monitor_blockchain()
        
        logger.info("Blockchain monitoring completed", 
                   monitoring_result=result)
        
        return result
        
    except Exception as e:
        logger.error("Blockchain monitoring failed", error=str(e))
        return None

def run_manual_processing(args_parser):
    """Ejecutar procesamiento manual"""
    fecha = None
    
    if args.fecha:
        try:
            fecha = datetime.strptime(args.fecha, '%Y-%m-%d')
        except ValueError:
            logger.error("Invalid date format", fecha_arg=args.fecha)
            return 1
    
    result = run_daily_processing(fecha)
    
    if result and result.get('estado') == 'completed':
        logger.info("Manual processing completed successfully")
        return 0
    else:
        logger.error("Manual processing failed", result=result)
        return 1

def run_scheduler():
    """Ejecutar programador"""
    logger.info("Starting blockchain processor scheduler")
    
    # Programar procesamiento diario
    schedule.every().day.at(f"{config.processing.execution_hour:02d}:{config.processing.execution_minute:02d}").do(run_daily_processing)
    
    # Programar monitoreo cada hora
    schedule.every().hour.do(run_monitoring)
    
    logger.info("Scheduler configured", 
                daily_execution_time=f"{config.processing.execution_hour:02d}:{config.processing.execution_minute:02d}",
                monitoring_interval="hourly")
    
    while True:
        try:
            schedule.run_pending()
            
            # Health check cada 5 minutos
            if int(datetime.now().strftime('%M')) % 5 == 0:
                network_health = monitor_blockchain().get('health', {})
                if not network_health.get('error'):
                    logger.info("Health check passed", **network_health)
                else:
                    logger.warning("Health check failed", error=network_health.get('error'))
            
            time.sleep(60)  # Esperar 1 minuto
            
        except KeyboardInterrupt:
            logger.info("Scheduler stopped by user")
            break
        except Exception as e:
            logger.error("Scheduler error", error=str(e))
            time.sleep(60)

def main():
    """Función principal"""
    parser = argparse.ArgumentParser(description='GASLYT Blockchain Processor')
    
    parser.add_argument('--modo', 
                       choices=['manual', 'scheduler', 'monitor'], 
                       default='scheduler',
                       help='Modo de ejecución')
    
    parser.add_argument('--fecha', 
                       type=str, 
                       help='Fecha para procesamiento manual (YYYY-MM-DD)')
    
    parser.add_argument('--verificar-db', 
                       action='store_true', 
                       help='Solo verificar conexión de base de datos')
    
    args = parser.parse_args()
    
    try:
        # Configurar logging
        if not os.path.exists(os.path.dirname(config.logging.file_path)):
            os.makedirs(os.path.dirname(config.logging.file_path))
        
        # Verificar configuración
        try:
            config.validate()
            logger.info("Configuration validated successfully")
        except ValueError as e:
            logger.error("Configuration validation failed", error=str(e))
            return 1
        
        # Verificar base de datos si se solicita
        if args.verificar_db:
            if setup_database():
                logger.info("Database verification completed successfully")
                return 0
            else:
                logger.error("Database verification failed")
                return 1
        
        # Configurar base de datos para otros modos
        if not setup_database():
            logger.error("Cannot proceed without database connection")
            return 1
        
        # Ejecutar según modo
        if args.modo == 'manual':
            logger.info("Running in manual mode")
            return run_manual_processing(args)
            
        elif args.modo == 'monitor':
            logger.info("Running in monitor mode")
            result = run_monitoring()
            return 0 if result else 1
            
        elif args.modo == 'scheduler':
            logger.info("Running in scheduler mode")
            run_scheduler()
            return 0
            
    except KeyboardInterrupt:
        logger.info("Application stopped by user")
        return 0
    except Exception as e:
        logger.error("Application error", error=str(e))
        return 1

if __name__ == '__main__':
    sys.exit(main())
