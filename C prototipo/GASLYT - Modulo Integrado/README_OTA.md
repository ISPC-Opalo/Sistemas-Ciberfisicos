# Sistema de Gestión de Certificados y Actualizaciones OTA

## Descripción

Sistema completo para la gestión de certificados AWS IoT Core y actualizaciones OTA (Over-The-Air) del firmware en dispositivos ESP32. Permite actualizaciones remotas seguras con validación de integridad y rollback automático.

## Características

### 🔐 Gestión de Certificados
- Almacenamiento seguro en particiones dedicadas
- Backup automático de certificados
- Validación de integridad y formato
- Verificación de firma digital
- Rollback automático ante fallos

### 📱 Actualizaciones OTA
- Descarga e instalación de firmware
- Verificación de hash y firma digital
- Rollback automático ante fallos
- Progreso en tiempo real
- Gestión de versiones

### 🔄 Gestión Centralizada
- Coordinador principal (GestorActualizaciones)
- Integración con MQTT para comandos remotos
- Logging completo de operaciones
- Configuración remota

## Estructura de Particiones

```csv
# Name,        Type, SubType, Offset,   Size,     Flags
nvs,           data, nvs,     0x9000,   0x6000,
phy_init,      data, phy,     0xf000,   0x1000,
factory,       app,  factory, 0x10000,  0x180000,  # 1.5MB para firmware
ota_0,         app,  ota_0,   0x190000, 0x180000,  # 1.5MB para OTA
ota_1,         app,  ota_1,   0x310000, 0x180000,  # 1.5MB para OTA
certs_current, data, 0x40,    0x490000, 0x20000,   # 128KB para certificados actuales
certs_backup,  data, 0x41,    0x4B0000, 0x20000,   # 128KB para backup de certificados
config,        data, 0x42,    0x4D0000, 0x10000,   # 64KB para configuración
ota_data,      data, 0x43,    0x4E0000, 0x2000,    # 8KB para datos OTA
spiffs,        data, spiffs,  0x4E2000, 0x1E000,   # 120KB para sistema de archivos
```

## Clases Principales

### 1. CertificadosManager
Gestión de certificados AWS IoT Core:
- Carga/guardado desde particiones
- Validación de formato e integridad
- Backup y restauración
- Configuración de WiFiClientSecure

### 2. SistemaOTA
Gestión de actualizaciones de firmware:
- Verificación de actualizaciones disponibles
- Descarga e instalación
- Verificación de integridad
- Rollback automático

### 3. GestorActualizaciones
Coordinador principal:
- Procesamiento de comandos MQTT
- Gestión de actualizaciones de certificados y firmware
- Notificaciones de estado y progreso
- Configuración remota

## Topics MQTT

### Comandos de Actualización
- `/{ID_DISPOSITIVO}/actualizaciones/certificados` - Comandos de certificados
- `/{ID_DISPOSITIVO}/actualizaciones/firmware` - Comandos de firmware

### Notificaciones
- `/{ID_DISPOSITIVO}/actualizaciones/estado` - Estado de actualizaciones
- `/{ID_DISPOSITIVO}/actualizaciones/progreso` - Progreso de descarga
- `/{ID_DISPOSITIVO}/actualizaciones/confirmacion` - Confirmaciones

## Comandos de Actualización

### Actualización de Certificados
```json
{
  "comando": "actualizar_certificados",
  "version": "1.2.0",
  "certificado": "-----BEGIN CERTIFICATE-----...",
  "clave_privada": "-----BEGIN PRIVATE KEY-----...",
  "certificado_ca": "-----BEGIN CERTIFICATE-----...",
  "endpoint": "a1b2c3d4e5f6g7.iot.us-east-1.amazonaws.com",
  "hash": "sha256:abc123...",
  "firma": "rsa:def456...",
  "critica": false
}
```

### Actualización de Firmware
```json
{
  "comando": "actualizar_firmware",
  "version": "2.1.0",
  "url": "https://updates.gaslyt.com/firmware/v2.1.0.bin",
  "hash": "sha256:xyz789...",
  "tamaño": 1048576,
  "descripcion": "Corrección de bugs y nuevas funcionalidades",
  "critica": true,
  "firma": "rsa:ghi789..."
}
```

### Comandos de Verificación
```json
{
  "comando": "verificar_certificados"
}
```

```json
{
  "comando": "verificar_firmware"
}
```

### Comando de Rollback
```json
{
  "comando": "rollback"
}
```

## Flujo de Actualización

### Certificados
1. **Verificación**: Validar nueva versión y firma digital
2. **Backup**: Crear backup de certificados actuales
3. **Actualización**: Guardar nuevos certificados en partición
4. **Validación**: Verificar integridad de nuevos certificados
5. **Confirmación**: Notificar éxito o fallo
6. **Rollback**: Restaurar desde backup si falla

### Firmware
1. **Verificación**: Validar nueva versión y firma digital
2. **Descarga**: Descargar firmware desde URL
3. **Validación**: Verificar hash del firmware descargado
4. **Instalación**: Instalar en partición OTA
5. **Verificación**: Validar integridad del firmware instalado
6. **Reinicio**: Reiniciar con nueva versión
7. **Rollback**: Restaurar versión anterior si falla

## Seguridad

### Firma Digital
- RSA 2048 para firmas
- SHA-256 para hashes
- Verificación de cadena de confianza
- Timestamps para evitar replay

### Validación de Integridad
- Checksums de archivos
- Verificación de particiones
- Validación de certificados
- Pruebas de arranque

## Configuración

### platformio.ini
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600

; Dependencias del proyecto
lib_deps = 
    https://github.com/miguel5612/MQSensorsLib
    knolleary/PubSubClient@^2.8
    tzapu/WiFiManager@^2.0.16
    bblanchon/ArduinoJson@^6.21.3
    adafruit/Adafruit NeoPixel@^1.10.6
    espressif/esp32-arduino-ota@^1.0.0

; Configuración de compilación
build_flags = 
    -DCORE_DEBUG_LEVEL=3
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DOTA_ENABLED=1
    -DCERTIFICADOS_REMOTOS=1

; Configuración de particiones para OTA
board_build.partitions = partitions_ota.csv

; Configuración OTA (opcional)
; upload_protocol = espota
; upload_port = 192.168.1.100
; upload_flags = --auth=password123
```

## Uso

### Inicialización
```cpp
// En setup()
CertificadosManager* certificadosManager = new CertificadosManager();
SistemaOTA* sistemaOTA = new SistemaOTA();
GestorActualizaciones* gestorActualizaciones = new GestorActualizaciones();

// Inicializar componentes
certificadosManager->inicializar();
sistemaOTA->inicializar();
gestorActualizaciones->inicializar(certificadosManager, sistemaOTA, mqttManager, logger);

// Configurar callbacks MQTT
mqttManager->establecerCallbackActualizaciones([](const String& payload) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
        gestorActualizaciones->procesarComandoActualizacion(doc.as<JsonObject>());
    }
});
```

### Verificación Periódica
```cpp
// En loop()
gestorActualizaciones->verificarActualizacionesPeriodicas();
```

### Configuración Remota
```cpp
// Configurar servidor de actualizaciones
gestorActualizaciones->establecerServidorActualizaciones("https://updates.gaslyt.com");
gestorActualizaciones->establecerTokenAutenticacion("token123");
gestorActualizaciones->habilitarActualizacionesAutomaticas(true);
```

## Estados del Sistema

### Certificados
- **VÁLIDO**: Certificados correctos y funcionando
- **INVÁLIDO**: Certificados corruptos o expirados
- **ACTUALIZANDO**: Proceso de actualización en curso
- **ERROR**: Error en la actualización

### Firmware
- **ACTUALIZADO**: Firmware actualizado
- **DISPONIBLE**: Actualización disponible
- **DESCARGANDO**: Descarga en progreso
- **INSTALANDO**: Instalación en progreso
- **COMPLETADO**: Actualización completada
- **ERROR**: Error en la actualización
- **ROLLBACK**: Restauración de versión anterior

## Logging

El sistema incluye logging completo de todas las operaciones:

```
[12:34:56] [INFO ] [ACTUALIZACIONES] Gestor de actualizaciones inicializado correctamente
[12:34:56] [INFO ] [ACTUALIZACIONES] Topics configurados para dispositivo: ESP32-GASLYT-123456
[12:34:57] [INFO ] [ACTUALIZACIONES] Verificando actualizaciones periódicas...
[12:34:58] [INFO ] [ACTUALIZACIONES] Actualización de firmware disponible: 2.1.0
```

## Consideraciones

### Memoria
- 3 particiones de firmware (4.5MB total)
- 2 particiones de certificados (256KB total)
- Partición de configuración (64KB)
- Partición de datos OTA (8KB)

### Red
- Descargas grandes requieren WiFi estable
- Timeout de 30 segundos para descargas
- Reintentos automáticos en caso de fallo

### Batería
- Actualizaciones consumen energía
- Modo ahorro durante descargas
- Notificaciones de estado de batería

### Tiempo
- OTA puede tardar varios minutos
- Verificación de integridad adicional
- Reinicio automático tras actualización

## Troubleshooting

### Certificados No Válidos
1. Verificar formato de certificados
2. Comprobar firma digital
3. Restaurar desde backup
4. Reconfigurar endpoint

### Error en OTA
1. Verificar espacio disponible
2. Comprobar integridad de descarga
3. Verificar hash del firmware
4. Realizar rollback si es necesario

### Problemas de Conexión
1. Verificar conectividad WiFi
2. Comprobar servidor de actualizaciones
3. Validar token de autenticación
4. Revisar logs de error

## Soporte

Para soporte técnico o consultas sobre el sistema de actualizaciones OTA, contactar al equipo de desarrollo.

## Licencia

MIT License
