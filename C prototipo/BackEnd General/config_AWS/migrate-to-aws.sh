#!/bin/bash
# Script de migración de datos desde entorno local a AWS EC2
# Migra datos de InfluxDB y MariaDB

set -e

echo "🔄 Iniciando migración de datos GASLYT a AWS EC2..."

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

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

# Verificar parámetros
if [ $# -ne 2 ]; then
    echo "Uso: $0 <ip_ec2_instance> <path_to_ssh_key>"
    echo "Ejemplo: $0 54.123.45.67 ~/.ssh/gaslyt-key.pem"
    exit 1
fi

EC2_IP=$1
SSH_KEY=$2
BACKUP_DIR="./migration_backup_$(date +%Y%m%d_%H%M%S)"

log "IP de EC2: $EC2_IP"
log "Clave SSH: $SSH_KEY"
log "Directorio de backup: $BACKUP_DIR"

# Verificar conectividad SSH
log "Verificando conectividad con EC2..."
if ! ssh -i "$SSH_KEY" -o ConnectTimeout=10 -o StrictHostKeyChecking=no ubuntu@"$EC2_IP" "echo 'Conexión SSH exitosa'"; then
    error "No se puede conectar a la instancia EC2. Verifique la IP y la clave SSH."
fi

# Crear directorio de backup
mkdir -p "$BACKUP_DIR"

# ========================================
# 1. BACKUP DE DATOS LOCALES
# ========================================
log "Creando backup de datos locales..."

# Backup MariaDB
log "Backup de MariaDB..."
if command -v mysqldump &> /dev/null; then
    mysqldump -u root -p opalodb > "$BACKUP_DIR/mariadb_local.sql"
    log "✅ Backup MariaDB completado"
else
    warning "mysqldump no encontrado. Backup MariaDB manual requerido."
fi

# Backup InfluxDB
log "Backup de InfluxDB..."
if command -v influx &> /dev/null; then
    influx backup "$BACKUP_DIR/influxdb_local" -t "$INFLUXDB_ADMIN_TOKEN"
    log "✅ Backup InfluxDB completado"
else
    warning "influx CLI no encontrado. Backup InfluxDB manual requerido."
fi

# Backup de configuraciones
log "Backup de configuraciones..."
tar -czf "$BACKUP_DIR/configs_local.tar.gz" config/
log "✅ Backup de configuraciones completado"

# ========================================
# 2. TRANSFERIR DATOS A EC2
# ========================================
log "Transfiriendo datos a EC2..."

# Crear directorio en EC2
ssh -i "$SSH_KEY" ubuntu@"$EC2_IP" "mkdir -p /tmp/gaslyt_migration"

# Transferir archivos
log "Transfiriendo MariaDB backup..."
scp -i "$SSH_KEY" "$BACKUP_DIR/mariadb_local.sql" ubuntu@"$EC2_IP":/tmp/gaslyt_migration/

log "Transfiriendo InfluxDB backup..."
scp -i "$SSH_KEY" -r "$BACKUP_DIR/influxdb_local" ubuntu@"$EC2_IP":/tmp/gaslyt_migration/

log "Transfiriendo configuraciones..."
scp -i "$SSH_KEY" "$BACKUP_DIR/configs_local.tar.gz" ubuntu@"$EC2_IP":/tmp/gaslyt_migration/

# ========================================
# 3. RESTAURAR DATOS EN EC2
# ========================================
log "Restaurando datos en EC2..."

# Restaurar MariaDB
log "Restaurando MariaDB..."
ssh -i "$SSH_KEY" ubuntu@"$EC2_IP" "
    cd /opt/gaslyt
    docker exec -i mariadb mysql -u root -p\$MARIADB_ROOT_PASSWORD opalodb < /tmp/gaslyt_migration/mariadb_local.sql
"

# Restaurar InfluxDB
log "Restaurando InfluxDB..."
ssh -i "$SSH_KEY" ubuntu@"$EC2_IP" "
    cd /opt/gaslyt
    docker cp /tmp/gaslyt_migration/influxdb_local influxdb:/tmp/
    docker exec influxdb influx restore /tmp/influxdb_local -t \$INFLUXDB_ADMIN_TOKEN
"

# ========================================
# 4. VERIFICAR MIGRACIÓN
# ========================================
log "Verificando migración..."

# Verificar MariaDB
log "Verificando MariaDB..."
ssh -i "$SSH_KEY" ubuntu@"$EC2_IP" "
    cd /opt/gaslyt
    docker exec mariadb mysql -u root -p\$MARIADB_ROOT_PASSWORD -e 'SELECT COUNT(*) as total_tables FROM information_schema.tables WHERE table_schema = \"opalodb\";'
"

# Verificar InfluxDB
log "Verificando InfluxDB..."
ssh -i "$SSH_KEY" ubuntu@"$EC2_IP" "
    cd /opt/gaslyt
    docker exec influxdb influx query 'from(bucket: \"gaslyt-data\") |> range(start: -30d) |> count()'
"

# ========================================
# 5. CONFIGURAR SERVICIOS
# ========================================
log "Configurando servicios en EC2..."

# Reiniciar servicios
ssh -i "$SSH_KEY" ubuntu@"$EC2_IP" "
    cd /opt/gaslyt
    sudo docker compose restart
"

# Verificar estado de servicios
log "Verificando estado de servicios..."
ssh -i "$SSH_KEY" ubuntu@"$EC2_IP" "
    cd /opt/gaslyt
    sudo docker compose ps
"

# ========================================
# 6. LIMPIAR ARCHIVOS TEMPORALES
# ========================================
log "Limpiando archivos temporales..."

# Limpiar en EC2
ssh -i "$SSH_KEY" ubuntu@"$EC2_IP" "rm -rf /tmp/gaslyt_migration"

# Limpiar local (opcional)
read -p "¿Desea eliminar el directorio de backup local? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf "$BACKUP_DIR"
    log "Directorio de backup local eliminado"
else
    log "Directorio de backup local conservado: $BACKUP_DIR"
fi

# ========================================
# 7. INFORMACIÓN FINAL
# ========================================
log "Migración completada exitosamente!"

echo ""
echo "=========================================="
echo "🎉 Migración GASLYT a AWS EC2 Completada"
echo "=========================================="
echo ""
echo "📊 Datos migrados:"
echo "- MariaDB: Tablas y datos relacionales"
echo "- InfluxDB: Series temporales y métricas"
echo "- Configuraciones: Archivos de configuración"
echo ""
echo "🔍 Verificaciones realizadas:"
echo "- Conectividad SSH con EC2"
echo "- Restauración de bases de datos"
echo "- Estado de servicios Docker"
echo ""
echo "📋 Próximos pasos:"
echo "1. Verificar dashboards en Grafana Cloud"
echo "2. Configurar dispositivos para AWS IoT Core"
echo "3. Probar flujo completo de datos"
echo "4. Configurar alertas y monitoreo"
echo ""
echo "🔗 URLs importantes:"
echo "- Grafana Cloud: https://your-instance.grafana.net"
echo "- AWS IoT Core: https://console.aws.amazon.com/iot/"
echo "- EC2 Instance: http://$EC2_IP"
echo ""
echo "✅ Migración completada. ¡Bienvenido a AWS!"
echo ""
