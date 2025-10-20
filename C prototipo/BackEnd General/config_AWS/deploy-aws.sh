#!/bin/bash
# Script de despliegue para GASLYT en AWS EC2
# Configuración automática del entorno AWS

set -e

echo "🚀 Iniciando despliegue de GASLYT en AWS EC2..."

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Función para logging
log() {
    echo -e "${GREEN}[$(date +'%Y-%m-%d %H:%M:%S')]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Verificar que estamos en Ubuntu/Debian
if ! command -v apt-get &> /dev/null; then
    error "Este script está diseñado para Ubuntu/Debian. Para otras distribuciones, adapte los comandos de instalación."
fi

# Verificar permisos de root
if [[ $EUID -ne 0 ]]; then
   error "Este script debe ejecutarse como root (use sudo)"
fi

# ========================================
# 1. INSTALACIÓN DE DEPENDENCIAS
# ========================================
log "Instalando dependencias del sistema..."

# Actualizar sistema
apt-get update
apt-get upgrade -y

# Instalar dependencias básicas
apt-get install -y \
    curl \
    wget \
    git \
    unzip \
    software-properties-common \
    apt-transport-https \
    ca-certificates \
    gnupg \
    lsb-release \
    htop \
    vim \
    ufw

# ========================================
# 2. INSTALACIÓN DE DOCKER
# ========================================
log "Instalando Docker..."

# Instalar Docker
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | tee /etc/apt/sources.list.d/docker.list > /dev/null
apt-get update
apt-get install -y docker-ce docker-ce-cli containerd.io docker-compose-plugin

# Iniciar Docker
systemctl start docker
systemctl enable docker

# Agregar usuario ubuntu al grupo docker
usermod -aG docker ubuntu

# ========================================
# 3. INSTALACIÓN DE AWS CLI
# ========================================
log "Instalando AWS CLI..."

curl "https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip" -o "awscliv2.zip"
unzip awscliv2.zip
./aws/install
rm -rf aws awscliv2.zip

# ========================================
# 4. INSTALACIÓN DE CLOUDWATCH AGENT
# ========================================
log "Instalando CloudWatch Agent..."

wget https://s3.amazonaws.com/amazoncloudwatch-agent/ubuntu/amd64/latest/amazon-cloudwatch-agent.deb
dpkg -i amazon-cloudwatch-agent.deb
rm amazon-cloudwatch-agent.deb

# ========================================
# 5. CONFIGURACIÓN DE FIREWALL
# ========================================
log "Configurando firewall..."

# Configurar UFW
ufw --force enable
ufw allow ssh
ufw allow 80/tcp
ufw allow 443/tcp
ufw allow 1883/tcp  # MQTT (si necesitas acceso directo)
ufw allow 9001/tcp  # WebSocket (si necesitas acceso directo)

# ========================================
# 6. CREACIÓN DE DIRECTORIOS
# ========================================
log "Creando estructura de directorios..."

# Crear directorios para la aplicación
mkdir -p /opt/gaslyt
mkdir -p /opt/gaslyt/logs
mkdir -p /opt/gaslyt/backups
mkdir -p /opt/gaslyt/ssl
mkdir -p /opt/gaslyt/config

# ========================================
# 7. CONFIGURACIÓN DE SSL PARA AWS IOT
# ========================================
log "Configurando certificados SSL para AWS IoT..."

# Descargar certificado raíz de Amazon
wget -O /opt/gaslyt/ssl/AmazonRootCA1.pem https://www.amazontrust.com/repository/AmazonRootCA1.pem

# Crear directorio para certificados de dispositivos
mkdir -p /opt/gaslyt/ssl/devices

# ========================================
# 8. CONFIGURACIÓN DE LOGROTATE
# ========================================
log "Configurando rotación de logs..."

cat > /etc/logrotate.d/gaslyt << EOF
/opt/gaslyt/logs/*.log {
    daily
    missingok
    rotate 30
    compress
    delaycompress
    notifempty
    create 644 ubuntu ubuntu
    postrotate
        docker-compose -f /opt/gaslyt/docker-compose.yml restart telegraf blockchain-processor
    endscript
}
EOF

# ========================================
# 9. CONFIGURACIÓN DE SYSTEMD SERVICE
# ========================================
log "Creando servicio systemd..."

cat > /etc/systemd/system/gaslyt.service << EOF
[Unit]
Description=GASLYT Backend Services
Requires=docker.service
After=docker.service

[Service]
Type=oneshot
RemainAfterExit=yes
WorkingDirectory=/opt/gaslyt
ExecStart=/usr/bin/docker compose up -d
ExecStop=/usr/bin/docker compose down
TimeoutStartSec=0

[Install]
WantedBy=multi-user.target
EOF

# Recargar systemd
systemctl daemon-reload
systemctl enable gaslyt.service

# ========================================
# 10. CONFIGURACIÓN DE BACKUP AUTOMÁTICO
# ========================================
log "Configurando backup automático..."

cat > /opt/gaslyt/backup.sh << 'EOF'
#!/bin/bash
# Script de backup automático para GASLYT

DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_DIR="/opt/gaslyt/backups"
LOG_FILE="/opt/gaslyt/logs/backup.log"

echo "[$(date)] Iniciando backup..." >> $LOG_FILE

# Backup de base de datos
docker exec mariadb mysqldump -u root -p$MARIADB_ROOT_PASSWORD opalodb > $BACKUP_DIR/mariadb_$DATE.sql

# Backup de InfluxDB
docker exec influxdb influx backup /tmp/influx_backup_$DATE -t $INFLUXDB_ADMIN_TOKEN
docker cp influxdb:/tmp/influx_backup_$DATE $BACKUP_DIR/

# Backup de configuraciones
tar -czf $BACKUP_DIR/configs_$DATE.tar.gz /opt/gaslyt/config/

# Limpiar backups antiguos (mantener 30 días)
find $BACKUP_DIR -name "*.sql" -mtime +30 -delete
find $BACKUP_DIR -name "*.tar.gz" -mtime +30 -delete
find $BACKUP_DIR -name "influx_backup_*" -mtime +30 -exec rm -rf {} \;

echo "[$(date)] Backup completado" >> $LOG_FILE
EOF

chmod +x /opt/gaslyt/backup.sh

# Configurar cron para backup diario
echo "0 2 * * * /opt/gaslyt/backup.sh" | crontab -u root -

# ========================================
# 11. CONFIGURACIÓN DE MONITOREO
# ========================================
log "Configurando monitoreo básico..."

# Crear script de health check
cat > /opt/gaslyt/health-check.sh << 'EOF'
#!/bin/bash
# Health check para servicios GASLYT

LOG_FILE="/opt/gaslyt/logs/health-check.log"

check_service() {
    local service=$1
    if docker ps --format "table {{.Names}}" | grep -q "^$service$"; then
        echo "[$(date)] ✅ $service: OK" >> $LOG_FILE
        return 0
    else
        echo "[$(date)] ❌ $service: FAILED" >> $LOG_FILE
        return 1
    fi
}

# Verificar servicios críticos
check_service "mariadb"
check_service "influxdb"
check_service "telegraf"
check_service "blockchain-processor"

# Verificar conectividad AWS IoT
if curl -s --connect-timeout 5 https://$AWS_IOT_ENDPOINT:8443 > /dev/null; then
    echo "[$(date)] ✅ AWS IoT Core: OK" >> $LOG_FILE
else
    echo "[$(date)] ❌ AWS IoT Core: FAILED" >> $LOG_FILE
fi
EOF

chmod +x /opt/gaslyt/health-check.sh

# Configurar cron para health check cada 5 minutos
echo "*/5 * * * * /opt/gaslyt/health-check.sh" | crontab -u root -

# ========================================
# 12. CONFIGURACIÓN DE VARIABLES DE ENTORNO
# ========================================
log "Configurando variables de entorno..."

# Crear archivo .env si no existe
if [ ! -f /opt/gaslyt/.env ]; then
    warning "Archivo .env no encontrado. Copiando desde env.example..."
    if [ -f /opt/gaslyt/env.example ]; then
        cp /opt/gaslyt/env.example /opt/gaslyt/.env
        warning "IMPORTANTE: Edite /opt/gaslyt/.env con sus valores reales antes de iniciar los servicios"
    else
        error "Archivo env.example no encontrado. Cree el archivo .env manualmente."
    fi
fi

# ========================================
# 13. CONFIGURACIÓN DE PERMISOS
# ========================================
log "Configurando permisos..."

# Cambiar propietario de directorios
chown -R ubuntu:ubuntu /opt/gaslyt
chmod -R 755 /opt/gaslyt

# ========================================
# 14. INFORMACIÓN FINAL
# ========================================
log "Despliegue completado exitosamente!"

echo ""
echo "=========================================="
echo "🎉 GASLYT AWS EC2 Setup Completado"
echo "=========================================="
echo ""
echo "📁 Directorio de la aplicación: /opt/gaslyt"
echo "📝 Logs: /opt/gaslyt/logs/"
echo "💾 Backups: /opt/gaslyt/backups/"
echo "🔧 Configuración: /opt/gaslyt/config/"
echo ""
echo "📋 Próximos pasos:"
echo "1. Editar /opt/gaslyt/.env con sus valores reales"
echo "2. Copiar certificados SSL de dispositivos a /opt/gaslyt/ssl/devices/"
echo "3. Ejecutar: cd /opt/gaslyt && docker compose up -d"
echo "4. Verificar servicios: systemctl status gaslyt"
echo ""
echo "🔍 Comandos útiles:"
echo "- Ver logs: docker compose logs -f [servicio]"
echo "- Reiniciar: systemctl restart gaslyt"
echo "- Health check: /opt/gaslyt/health-check.sh"
echo "- Backup manual: /opt/gaslyt/backup.sh"
echo ""
echo "⚠️  IMPORTANTE: Configure AWS IoT Core y Grafana Cloud antes de iniciar"
echo ""

# Mostrar estado de servicios
log "Estado actual de Docker:"
docker --version
docker compose version

log "Estado de AWS CLI:"
aws --version

log "Estado de CloudWatch Agent:"
systemctl status amazon-cloudwatch-agent --no-pager -l

echo ""
echo "✅ Setup completado. ¡Bienvenido a GASLYT en AWS!"
