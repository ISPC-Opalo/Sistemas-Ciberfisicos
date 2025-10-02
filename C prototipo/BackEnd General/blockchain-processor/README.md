# 🔗 Blockchain Processor - GASLYT

Procesador Python para registrar datos críticos de dispositivos GASLYT en blockchain Ethereum.

## 📁 Estructura del Proyecto

```
blockchain-processor/
├── app/
│   ├── config.py          # Configuración del sistema
│   └── main.py            # Cliente principal
├── blocks/
│   └── web3_client.py     # Cliente Ethereum Web3
├── core/
│   └── gaslyt_processor.py # Procesador principal
├── db/
│   ├── database.py        # Manager de base de datos
│   └── models.py          # Modelos SQLAlchemy
└── requirements.txt       # Dependencias Python
```

## 🚀 Inicio Rápido

### 1. Instalar Dependencias
```bash
pip install -r requirements.txt
```

### 2. Configurar Variables de Entorno
```bash
# Copiar archivo .env desde config/
cp ../config/.env .
```

### 3. Ejecutar Procesador
```bash
# Modo scheduler (recomendado para producción)
python app/main.py --modo scheduler

# Modo manual (para testing)
python app/main.py --modo manual --fecha 2024-01-15

# Verificar base de datos
python app/main.py --verificar-db
```

## 🔧 Configuración

### Variables de Entorno Requeridas
```bash
# Ethereum
ETH_NETWORK=sepolia
ETH_RPC_URL=https://sepolia.infura.io/v3/YOUR_ID
ETH_PRIVATE_KEY=0x...
CONTRACT_ADDRESS=0x...

# Base de datos
MARIADB_HOST=mariadb
MARIADB_USER=opalouser
MARIADB_PASSWORD=opalopass
MARIADB_DATABASE=opalodb

# InfluxDB
INFLUXDB_URL=http://influxdb:8086
INFLUXDB_ADMIN_TOKEN=superinfluxadminpassword
INFLUXDB_ORG=opalo-org
INFLUXDB_BUCKET=telemetria
```

## 📊 Funcionalidades

- ✅ **Procesamiento en lotes** con optimización de gas
- ✅ **Retry automático** con backoff exponencial
- ✅ **Logging estructurado** con métricas detalladas
- ✅ **Monitoreo de blockchain** en tiempo real
- ✅ **Dashboard Grafana** para visualización

## 🗃️ Base de Datos

El procesador crea automáticamente las siguientes tablas:
- `blockchain_transactions` - Transacciones blockchain
- `blockchain_registry` - Registros de alarmas
- `blockchain_metrics` - Métricas de red
- `network_health` - Salud de la red
- `blockchain_errors` - Log de errores
- `processing_batches` - Lotes procesados

## 📈 Dashboard

Importar `grafana-dashboards/blockchain-dashboard.json` en Grafana para visualizar:
- Estado de transacciones
- Métricas de gas y costo
- Top dispositivos con alarmas
- Errores del sistema

## 🔄 Flujo de Datos

```
Alarmas GASLYT → InfluxDB → Procesador Python → Blockchain Ethereum
                ↓
            MariaDB (metadatos y tracking)
```

## 📚 Documentación

- [Manual Técnico GASLYT](../MANUAL_TECNICO_GASLYT.md)
- [Mejoras de Despliegue](../docs/MEJORAS_DESPLIEGUE.md)
- [Credenciales del Sistema](../credenciales.md)

## 🆘 Soporte

Para problemas o consultas técnicas, revisar:
1. Logs del procesador en `/app/logs/blockchain-processor.log`
2. Estado de transacciones en tabla `blockchain_transactions`
3. Errores en tabla `blockchain_errors`

---

**Versión**: 1.0  
**Autor**: Equipo GASLYT  
**Licencia**: MIT
