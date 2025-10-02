# ⚙️ Configuración - BackEnd General

Este directorio contiene todos los archivos de configuración para el sistema GASLYT.

## 📁 Contenido

```
config/
├── docker-compose.yml     # Orquestación de servicios Docker
├── .env                  # Variables de entorno (⚠️ NO COMMIT)
├── mosquitto.conf        # Configuración broker MQTT
├── telegraf.conf         # Configuración procesador datos
├── init-mariadb.sql      # Script inicialización MariaDB
└── README.md            # Este archivo
```

## 🔧 Configuración Inicial

### 1. Variables de Entorno

Copiar `.env.example` a `.env` y editar:

```bash
# Ejemplo de variables críticas
MARIADB_ROOT_PASSWORD=your_secure_password
ETH_PRIVATE_KEY=0x...
CONTRACT_ADDRESS=0x...
INFLUXDB_ADMIN_TOKEN=your_token
```

### 2. Configurar MQTT Broker

El archivo `mosquitto.conf` configura:
- Puerto 1883 (MQTT estándar)
- Puerto 9001 (WebSocket)
- Persistencia habilitada
- Conexiones anónimas (desarrollo)

### 3. Configurar Procesador de Datos

El archivo `telegraf.conf` configura:
- Recepción MQTT vía WebSocket
- Procesamiento de datos JSON
- Envío a InfluxDB y MariaDB
- Categorización automática

### 4. Script de Base de Datos

El archivo `init-mariadb.sql` contiene:
- Tablas para clientes, dispositivos, productos
- Tablas blockchain (transacciones, métricas)
- Datos iniciales de ejemplo
- Vistas y triggers útiles

## 🚀 Despliegue

```bash
# Desde este directorio
docker-compose up -d

# Verificar estado
docker-compose ps

# Ver logs
docker-compose logs -f [servicio]
```

## 🔒 Seguridad

⚠️ **Importante**: 
- Nunca hacer commit del archivo `.env` 
- Cambiar todas las contraseñas por defecto
- Usar certificados SSL en producción
- Configurar firewall apropiadamente

## 📊 Monitoreo

Los servicios exponen métricas en:
- **Grafana**: https://grafana.iot-opalo.work/
- **Portainer**: https://portainer.iot-opalo.work/
- **Adminer**: https://adminer.iot-opalo.work/

## 🆘 Troubleshooting

### Problema: Servicio no inicia
```bash
# Ver logs detallados
docker-compose logs [servicio]

# Reiniciar específico
docker-compose restart [servicio]
```

### Problema: Puerto ocupado
```bash
# Verificar puertos en uso
netstat -tulpn | grep :[puerto]

# Cambiar puerto en docker-compose.yml
```

### Problema: Base de datos corrupta
```bash
# Restaurar desde backup
docker-compose exec mariadb mysql -u root -p opalodb < backup.sql

# Reinicializar (⚠️ PERDERÁ DATOS)
docker-compose down
docker volume rm [volumen_mariadb]
docker-compose up -d
```

---

**Equipo**: GASLYT Development Team
