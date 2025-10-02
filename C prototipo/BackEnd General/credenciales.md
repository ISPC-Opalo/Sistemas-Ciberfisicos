# 🔐 Credenciales del Sistema GASLYT

## 📊 **Grafana - Dashboards y Monitoreo**
- **URL**: https://grafana.iot-opalo.work/
- **Usuario**: `admin`
- **Contraseña**: `grafanaadminpassword`
- **Descripción**: Dashboards para monitoreo de dispositivos GASLYT, métricas de sistema y alertas

---

## 🌐 **NGINX Proxy Manager - Gestión de Dominios**
- **URL**: https://npm-admin.iot-opalo.work/
- **Usuario**: `vittodutti@gmail.com`
- **Contraseña**: `adminpassword`
- **Descripción**: Gestión de dominios, SSL/TLS y proxy reverso
- **Recuperar clave**: [Guía de recuperación](https://github.com/NginxProxyManager/nginx-proxy-manager/discussions/1634)

---

## 🗄️ **Adminer - Administración de Base de Datos**
- **URL**: https://adminer.iot-opalo.work/
- **Usuario**: `root`
- **Contraseña**: `rootpassword`
- **Descripción**: Interfaz web para administrar MariaDB

### **MariaDB - Base de Datos Principal**
- **Base de datos**: `opalodb`
- **Usuario root**: `root` / `rootpassword`
- **Usuario aplicación**: `opalouser` / `opalopass`
- **Descripción**: Base de datos relacional para clientes, dispositivos, productos, técnicos, etc.

---

## 📈 **InfluxDB - Series Temporales**
- **URL**: https://influx.iot-opalo.work/
- **Usuario**: `admin`
- **Contraseña**: `influxadminpassword`
- **Token Admin**: `superinfluxadminpassword`
- **Organización**: `opalo-org`
- **Bucket**: `telemetria`
- **Descripción**: Almacenamiento de datos de sensores, alarmas y métricas en tiempo real

---

## 🔄 **Node-RED - Automatización y Flujos**
- **URL**: https://nodered.iot-opalo.work/
- **Usuario**: `(configurar)`
- **Contraseña**: `(configurar)`
- **Descripción**: Flujos de automatización para procesamiento de datos MQTT

---

## 💻 **Remix IDE - Desarrollo Blockchain**
- **URL**: http://remix.iot-opalo.work/
- **Descripción**: IDE web para desarrollo de contratos inteligentes

---

## 🔧 **MQTT Broker - Mosquitto**
- **Host**: `mosquitto` (interno) / `iot-opalo.work` (externo)
- **Puerto MQTT**: `1883`
- **Puerto WebSocket**: `9001`
- **Usuario**: `gaslyt_user`
- **Contraseña**: `gaslyt_password`
- **Descripción**: Broker MQTT para comunicación con dispositivos GASLYT

---

## 📡 **Telegraf - Procesamiento de Datos**
- **Configuración**: `telegraf.conf`
- **Intervalo**: `10s`
- **Descripción**: Recolecta datos MQTT y los envía a InfluxDB y MariaDB

---

## 🚀 **Portainer - Gestión de Contenedores**
- **URL**: https://portainer.iot-opalo.work/
- **Descripción**: Interfaz web para gestión de contenedores Docker

---

## 🔒 **Información de Seguridad**

### **Tokens y Claves**
- **JWT Secret**: `gaslyt_jwt_secret_key_2024`
- **Encryption Key**: `gaslyt_encryption_key_32_chars`
- **Cloudflare Tunnel**: `eyJhIjoiNzBlZGIxMTNhZWUwYzIwNzJmNDA2Y2Q1ZTk2OGE5NDciLCJ0IjoiMjA3NGY1MmEtZTUwMS00Y2ZmLWE5ZGUtZjc4M2QxZTI5OGZiIiwicyI6Ik1tTmtNakkwTWpRdFlUY3pNUzAwWkdVMExXRTFNalF0TnpSa016RXdOVEZoWkRaaiJ9`

### **Configuración de Red**
- **Dominio base**: `iot-opalo.work`
- **Zona horaria**: `America/Argentina/Cordoba`

---

## 📋 **Checklist de Acceso**

- [ ] Grafana - Dashboards operativos
- [ ] NGINX Proxy Manager - Dominios configurados
- [ ] Adminer - Conexión a MariaDB
- [ ] InfluxDB - Datos de sensores fluyendo
- [ ] Node-RED - Flujos configurados
- [ ] MQTT Broker - Dispositivos conectados
- [ ] Telegraf - Procesamiento de datos activo

---

## 🆘 **Soporte y Recuperación**

### **En caso de problemas de acceso:**
1. **Grafana**: Verificar variables de entorno `GRAFANA_ADMIN_*`
2. **NGINX**: Usar [guía de recuperación](https://github.com/NginxProxyManager/nginx-proxy-manager/discussions/1634)
3. **Bases de datos**: Verificar variables `MARIADB_*` e `INFLUXDB_*`
4. **MQTT**: Comprobar configuración en `mosquitto.conf`

### **Comandos útiles:**
```bash
# Verificar estado de contenedores
docker-compose ps

# Ver logs de un servicio
docker-compose logs -f [servicio]

# Reiniciar un servicio
docker-compose restart [servicio]
```

---

**📅 Última actualización**: Septiembre 2025
**🔖 Versión**: 1.0  
**👤 Mantenido por**: Equipo GASLYT

