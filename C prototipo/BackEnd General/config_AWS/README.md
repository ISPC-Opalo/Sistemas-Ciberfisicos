# 🚀 Migración a AWS EC2 - GASLYT Backend

## 📋 Resumen de Cambios

Este directorio contiene la configuración adaptada para migrar el backend GASLYT a AWS EC2, eliminando servicios locales y utilizando servicios cloud de AWS.

## 🔄 Cambios Principales

### ❌ Servicios Eliminados
- **Mosquitto MQTT Broker** → Reemplazado por **AWS IoT Core**
- **Grafana Local** → Reemplazado por **Grafana Cloud**
- **Cloudflare Tunnel** → Reemplazado por **AWS Load Balancer/API Gateway**

### ✅ Servicios Agregados
- **AWS IoT Core** → Broker MQTT gestionado
- **CloudWatch Agent** → Monitoreo de métricas AWS
- **AWS CLI** → Gestión de recursos AWS
- **Grafana Cloud Integration** → Dashboards en la nube

### 🔧 Servicios Modificados
- **Telegraf** → Configurado para AWS IoT Core
- **Docker Compose** → Optimizado para AWS EC2
- **Blockchain Processor** → Adaptado para entorno AWS

## 📁 Estructura de Archivos

```
config_AWS/
├── docker-compose.yml          # Orquestación optimizada para AWS
├── telegraf.conf               # Configuración para AWS IoT Core
├── cloudwatch-config.json      # Configuración CloudWatch Agent
├── init-mariadb.sql           # Script de inicialización DB
├── env.example                # Variables de entorno para AWS
├── deploy-aws.sh              # Script de despliegue automático
├── Dockerfile.blockchain-processor  # Dockerfile optimizado
└── README.md                  # Este archivo
```

## 🚀 Proceso de Migración

### 1. Preparación de AWS

#### Crear Recursos AWS:
```bash
# 1. EC2 Instance (t3.medium o superior)
# 2. AWS IoT Core Thing
# 3. Certificados SSL para dispositivos
# 4. Grafana Cloud Account
# 5. IAM Role con permisos necesarios
```

#### Permisos IAM Requeridos:
```json
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Effect": "Allow",
            "Action": [
                "iot:Connect",
                "iot:Publish",
                "iot:Subscribe",
                "iot:Receive"
            ],
            "Resource": "*"
        },
        {
            "Effect": "Allow",
            "Action": [
                "cloudwatch:PutMetricData",
                "cloudwatch:GetMetricStatistics",
                "logs:CreateLogGroup",
                "logs:CreateLogStream",
                "logs:PutLogEvents"
            ],
            "Resource": "*"
        }
    ]
}
```

### 2. Configuración de AWS IoT Core

#### Crear Thing y Certificados:
```bash
# Crear Thing
aws iot create-thing --thing-name "gaslyt-device-001"

# Crear certificados
aws iot create-keys-and-certificate \
    --set-as-active \
    --certificate-pem-outfile device-certificate.pem.crt \
    --public-key-outfile public.pem.key \
    --private-key-outfile private.pem.key

# Crear política
aws iot create-policy \
    --policy-name "GaslytDevicePolicy" \
    --policy-document '{
        "Version": "2012-10-17",
        "Statement": [
            {
                "Effect": "Allow",
                "Action": "iot:Connect",
                "Resource": "arn:aws:iot:*:*:client/gaslyt-*"
            },
            {
                "Effect": "Allow",
                "Action": "iot:Publish",
                "Resource": "arn:aws:iot:*:*:topic/gaslyt/*"
            },
            {
                "Effect": "Allow",
                "Action": "iot:Subscribe",
                "Resource": "arn:aws:iot:*:*:topicfilter/gaslyt/*"
            }
        ]
    }'

# Adjuntar política al certificado
aws iot attach-policy \
    --policy-name "GaslytDevicePolicy" \
    --target "arn:aws:iot:*:*:cert/*"
```

### 3. Configuración de Grafana Cloud

#### Conectar InfluxDB a Grafana Cloud:
1. Crear cuenta en Grafana Cloud
2. Obtener URL y token de InfluxDB
3. Configurar datasource en Grafana Cloud
4. Importar dashboards desde `grafana-dashboards/`

### 4. Despliegue en EC2

#### Ejecutar Script de Despliegue:
```bash
# 1. Conectar a EC2
ssh -i your-key.pem ubuntu@your-ec2-ip

# 2. Descargar configuración
git clone https://github.com/your-repo/gaslyt-backend.git
cd gaslyt-backend/config_AWS

# 3. Ejecutar script de despliegue
sudo chmod +x deploy-aws.sh
sudo ./deploy-aws.sh

# 4. Configurar variables de entorno
sudo nano /opt/gaslyt/.env

# 5. Iniciar servicios
cd /opt/gaslyt
sudo docker compose up -d
```

## 🔧 Configuración Detallada

### Variables de Entorno Críticas

```bash
# AWS IoT Core
AWS_IOT_ENDPOINT=your-endpoint.iot.us-east-1.amazonaws.com
AWS_REGION=us-east-1
AWS_ACCESS_KEY_ID=your_access_key
AWS_SECRET_ACCESS_KEY=your_secret_key

# Grafana Cloud
GRAFANA_CLOUD_URL=https://your-instance.grafana.net
GRAFANA_CLOUD_TOKEN=your_grafana_token

# Base de datos (mantener configuración local)
MARIADB_ROOT_PASSWORD=secure_password
INFLUXDB_ADMIN_TOKEN=your_influx_token
```

### Topics MQTT en AWS IoT Core

Los dispositivos deben publicar en estos topics:
- `gaslyt/{device_id}/lecturas` - Lecturas de sensores
- `gaslyt/{device_id}/alarmas` - Alarmas de gas
- `gaslyt/{device_id}/metadata` - Metadatos del dispositivo
- `gaslyt/{device_id}/actualizaciones/estado` - Estado de actualizaciones

### Configuración de Telegraf

El archivo `telegraf.conf` está configurado para:
- Conectar a AWS IoT Core vía SSL
- Procesar datos JSON de dispositivos GASLYT
- Enviar a InfluxDB local y Grafana Cloud
- Enviar métricas a CloudWatch

## 📊 Monitoreo y Observabilidad

### CloudWatch Metrics
- Métricas de EC2 (CPU, RAM, disco, red)
- Métricas personalizadas de GASLYT
- Logs centralizados en CloudWatch Logs

### Grafana Cloud Dashboards
- Dashboards de dispositivos IoT
- Métricas de blockchain
- Alertas en tiempo real

### Health Checks
- Script automático cada 5 minutos
- Verificación de servicios Docker
- Conectividad con AWS IoT Core

## 🔒 Seguridad

### Certificados SSL
- Certificados de dispositivos en `/opt/gaslyt/ssl/devices/`
- Certificado raíz de Amazon incluido
- Rotación automática de certificados

### Firewall
- UFW configurado con puertos mínimos
- Solo SSH, HTTP, HTTPS, MQTT abiertos
- Acceso restringido por IP si es necesario

### Backup Automático
- Backup diario a las 2 AM
- Retención de 30 días
- Backup de MariaDB, InfluxDB y configuraciones

## 🆘 Troubleshooting

### Problemas Comunes

#### 1. Error de conexión AWS IoT Core
```bash
# Verificar certificados
ls -la /opt/gaslyt/ssl/devices/

# Verificar conectividad
curl -v https://your-endpoint.iot.us-east-1.amazonaws.com:8443

# Verificar logs de Telegraf
docker logs telegraf
```

#### 2. Error de Grafana Cloud
```bash
# Verificar token
curl -H "Authorization: Bearer $GRAFANA_CLOUD_TOKEN" \
     $GRAFANA_CLOUD_URL/api/org

# Verificar datasource
# Ir a Grafana Cloud → Data Sources → InfluxDB
```

#### 3. Servicios no inician
```bash
# Verificar estado
systemctl status gaslyt

# Ver logs
docker compose logs -f

# Reiniciar servicios
systemctl restart gaslyt
```

### Comandos Útiles

```bash
# Ver estado de todos los servicios
docker compose ps

# Ver logs en tiempo real
docker compose logs -f telegraf blockchain-processor

# Ejecutar health check manual
/opt/gaslyt/health-check.sh

# Backup manual
/opt/gaslyt/backup.sh

# Ver métricas CloudWatch
aws cloudwatch get-metric-statistics \
    --namespace "GASLYT/System" \
    --metric-name "CPUUtilization" \
    --start-time 2024-01-01T00:00:00Z \
    --end-time 2024-01-01T23:59:59Z \
    --period 3600 \
    --statistics Average
```

## 📈 Optimizaciones de Costos

### EC2 Instance Types Recomendados
- **Desarrollo**: t3.small (2 vCPU, 2 GB RAM)
- **Producción pequeña**: t3.medium (2 vCPU, 4 GB RAM)
- **Producción media**: t3.large (2 vCPU, 8 GB RAM)
- **Producción grande**: t3.xlarge (4 vCPU, 16 GB RAM)

### Estrategias de Ahorro
- Usar Spot Instances para desarrollo
- Configurar Auto Scaling Groups
- Usar EBS GP3 para almacenamiento
- Implementar Lifecycle Policies en S3

## 🔄 Migración desde Entorno Local

### Pasos de Migración
1. **Backup de datos locales**
2. **Configurar AWS IoT Core**
3. **Configurar Grafana Cloud**
4. **Desplegar en EC2**
5. **Migrar datos de InfluxDB**
6. **Migrar datos de MariaDB**
7. **Configurar dispositivos para AWS IoT Core**
8. **Verificar funcionamiento**

### Script de Migración de Datos
```bash
# Backup desde entorno local
mysqldump -u root -p opalodb > backup_local.sql
influx backup /tmp/influx_backup -t $TOKEN

# Restore en AWS EC2
mysql -u root -p opalodb < backup_local.sql
influx restore /tmp/influx_backup -t $TOKEN
```

## 📚 Recursos Adicionales

- [AWS IoT Core Documentation](https://docs.aws.amazon.com/iot/)
- [Grafana Cloud Documentation](https://grafana.com/docs/grafana-cloud/)
- [Telegraf AWS IoT Plugin](https://github.com/influxdata/telegraf/tree/master/plugins/inputs/mqtt_consumer)
- [Docker Compose AWS Best Practices](https://docs.docker.com/cloud/ecs-integration/)

---

**Versión**: 1.0  
**Última actualización**: Octubre 2024  
**Mantenido por**: Equipo GASLYT
