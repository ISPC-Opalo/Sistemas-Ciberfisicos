# Manual Técnico - GASLYT Módulo Integrado

## Índice
1. [Introducción](#introducción)
2. [Arquitectura del Sistema](#arquitectura-del-sistema)
3. [Componentes Principales](#componentes-principales)
4. [Protocolo MQTT](#protocolo-mqtt)
5. [Formatos JSON](#formatos-json)
6. [Configuración Remota](#configuración-remota)
7. [Sistema OTA y Actualizaciones](#sistema-ota-y-actualizaciones)
8. [Gestión de Certificados](#gestión-de-certificados)
9. [Estados del Sistema](#estados-del-sistema)
10. [Integración y Desarrollo](#integración-y-desarrollo)

---

## Introducción

GASLYT es un sistema de monitoreo de gas inflamable basado en ESP32 que utiliza sensores MQ para detectar concentraciones peligrosas de gas. El sistema incluye comunicación WiFi/MQTT, alarmas visuales y sonoras, control de extractores, y capacidades de actualización remota (OTA).

### Características Principales
- **Monitoreo continuo** de gas inflamable con sensores MQ
- **Comunicación bidireccional** vía MQTT (AWS IoT Core y modo DEV)
- **Alarmas multisensoriales** (LED RGB, buzzer, extractor)
- **Configuración remota** en tiempo real
- **Actualizaciones OTA** seguras con rollback automático
- **Gestión de certificados** AWS IoT Core
- **Optimización de energía** para funcionamiento con batería

---

## Arquitectura del Sistema

### Diagrama de Componentes

```
┌─────────────────────────────────────────────────────────────┐
│                    GASLYT - ESP32                          │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐        │
│  │ GasSensor   │  │SistemaAlarmas│  │Optimizacion │        │
│  │   (MQ-2)    │  │  (LED/Buzzer)│  │  Energia    │        │
│  └─────────────┘  └─────────────┘  └─────────────┘        │
│         │                │                │                │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐        │
│  │ConfigManager│  │SistemaLogging│  │WiFiManager  │        │
│  │             │  │             │  │             │        │
│  └─────────────┘  └─────────────┘  └─────────────┘        │
│         │                │                │                │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐        │
│  │MQTTManager  │  │Configuracion│  │SistemaOTA   │        │
│  │             │  │   Remota    │  │             │        │
│  └─────────────┘  └─────────────┘  └─────────────┘        │
│         │                │                │                │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐        │
│  │Certificados │  │GestorActuali│  │   main.cpp  │        │
│  │   Manager   │  │  zaciones   │  │             │        │
│  └─────────────┘  └─────────────┘  └─────────────┘        │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    MQTT Broker                             │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐        │
│  │ AWS IoT Core│  │   Local     │  │   Cloud     │        │
│  │             │  │   MQTT      │  │   Services  │        │
│  └─────────────┘  └─────────────┘  └─────────────┘        │
└─────────────────────────────────────────────────────────────┘
```

### Flujo de Datos

1. **Captura**: Sensor MQ lee concentración de gas
2. **Procesamiento**: Sistema evalúa umbrales y activa alarmas
3. **Comunicación**: Datos enviados vía MQTT
4. **Configuración**: Parámetros ajustables remotamente
5. **Actualización**: Firmware y certificados actualizables OTA

---

## Componentes Principales

### 1. GasSensor
**Archivo**: `include/GasSensor.h`, `src/GasSensor.cpp`

Maneja la lectura y calibración de sensores MQ (MQ-2, MQ-3, MQ-4, etc.).

**Funciones principales**:
- `leerConcentracion()`: Obtiene lectura en PPM
- `verificarUmbral()`: Evalúa si supera umbral de alarma
- `calibrar()`: Calibración en aire limpio
- `establecerUmbral()`: Configura umbral de alarma

### 2. SistemaAlarmas
**Archivo**: `include/SistemaAlarmas.h`, `src/SistemaAlarmas.cpp`

Controla LED RGB, buzzer y extractor según el estado del sistema.

**Estados**:
- `NORMAL`: LED verde, sin sonido
- `ADVERTENCIA`: LED amarillo, sonido intermitente
- `ALARMA`: LED rojo, sonido continuo, extractor activado
- `SIN_WIFI`: LED azul, sonido característico
- `ERROR`: LED magenta, sonido de error

### 3. MQTTManager
**Archivo**: `include/MQTTManager.h`, `src/MQTTManager.cpp`

Gestiona la comunicación MQTT con soporte para AWS IoT Core y modo DEV.

**Características**:
- Conexión segura con certificados
- Publicación de lecturas, alarmas y metadata
- Suscripción a configuración y actualizaciones
- Reconexión automática

### 4. ConfigManager
**Archivo**: `include/ConfigManager.h`, `src/ConfigManager.cpp`

Almacena y gestiona la configuración del sistema usando Preferences.

**Parámetros configurables**:
- ID del dispositivo
- Intervalo de medición (10-60 segundos)
- Umbral de alarma
- Configuración MQTT
- Pines de hardware

### 5. SistemaOTA
**Archivo**: `include/SistemaOTA.h`, `src/SistemaOTA.cpp`

Maneja actualizaciones de firmware con rollback automático.

**Características**:
- Descarga e instalación de firmware
- Verificación de integridad
- Rollback automático ante fallos
- Gestión de particiones OTA

### 6. CertificadosManager
**Archivo**: `include/CertificadosManager.h`, `src/CertificadosManager.cpp`

Gestiona certificados AWS IoT Core con backup automático.

**Funciones**:
- Almacenamiento en particiones dedicadas
- Validación de formato e integridad
- Backup y restauración
- Configuración de WiFiClientSecure

---

## Protocolo MQTT

### Estructura de Topics

Todos los topics siguen el patrón: `/{ID_DISPOSITIVO}/{TIPO}`

Donde `ID_DISPOSITIVO` = `ESP32-GASLYT-{MAC_LAST_6}`

### Topics Principales

#### 1. Lecturas
**Topic**: `/{ID_DISPOSITIVO}/lecturas`
- **QoS**: 0
- **Frecuencia**: Configurable (10-60 segundos)
- **Contenido**: Mediciones normales de gas

#### 2. Alarmas
**Topic**: `/{ID_DISPOSITIVO}/alarmas`
- **QoS**: 2
- **Frecuencia**: Evento (cuando se activa alarma)
- **Contenido**: Alertas de concentración peligrosa

#### 3. Metadata
**Topic**: `/{ID_DISPOSITIVO}/metadata`
- **QoS**: 2
- **Frecuencia**: Inicial y periódica
- **Contenido**: Información del dispositivo y estado

#### 4. Configuración
**Topic**: `/{ID_DISPOSITIVO}/configuracion`
- **QoS**: 2
- **Frecuencia**: Comando (entrada)
- **Contenido**: Parámetros de configuración

### Topics de Actualizaciones

#### 5. Certificados
**Topic**: `/{ID_DISPOSITIVO}/actualizaciones/certificados`
- **QoS**: 2
- **Frecuencia**: Comando
- **Contenido**: Comandos de actualización de certificados

#### 6. Firmware
**Topic**: `/{ID_DISPOSITIVO}/actualizaciones/firmware`
- **QoS**: 2
- **Frecuencia**: Comando
- **Contenido**: Comandos de actualización de firmware

#### 7. Estado de Actualizaciones
**Topic**: `/{ID_DISPOSITIVO}/actualizaciones/estado`
- **QoS**: 2
- **Frecuencia**: Evento
- **Contenido**: Estado de procesos de actualización

#### 8. Progreso de Actualizaciones
**Topic**: `/{ID_DISPOSITIVO}/actualizaciones/progreso`
- **QoS**: 2
- **Frecuencia**: Durante actualización
- **Contenido**: Progreso de descarga/instalación

#### 9. Confirmación de Actualizaciones
**Topic**: `/{ID_DISPOSITIVO}/actualizaciones/confirmacion`
- **QoS**: 2
- **Frecuencia**: Evento
- **Contenido**: Confirmaciones de comandos ejecutados

---

## Formatos JSON

### 1. Lectura de Gas

**Topic**: `/{ID_DISPOSITIVO}/lecturas`

```json
{
  "timestamp": 1640995200,
  "fecha": "2024-01-01 12:00:00",
  "concentracion": 150.5,
  "unidad": "ppm",
  "umbral": 1000.0,
  "alarma": false,
  "idDispositivo": "ESP32-GASLYT-123456",
  "rssi": -45
}
```

**Campos**:
- `timestamp`: Unix timestamp
- `fecha`: Fecha legible (YYYY-MM-DD HH:MM:SS)
- `concentracion`: Valor en PPM
- `unidad`: Siempre "ppm"
- `umbral`: Umbral configurado
- `alarma`: true si supera umbral
- `idDispositivo`: ID único del dispositivo
- `rssi`: Señal WiFi en dBm

### 2. Alarma de Gas

**Topic**: `/{ID_DISPOSITIVO}/alarmas`

```json
{
  "timestamp": 1640995200,
  "fecha": "2024-01-01 12:00:00",
  "tipo": "GAS_INFLAMABLE",
  "concentracion": 1200.0,
  "umbral": 1000.0,
  "severidad": "ALTA",
  "idDispositivo": "ESP32-GASLYT-123456",
  "accion": "EXTRACTOR_ACTIVADO"
}
```

**Campos**:
- `tipo`: Tipo de alarma ("GAS_INFLAMABLE")
- `concentracion`: Valor que activó la alarma
- `severidad`: Nivel de severidad ("BAJA", "MEDIA", "ALTA")
- `accion`: Acción tomada ("EXTRACTOR_ACTIVADO", "ALARMA_SONORA")

### 3. Metadata Inicial

**Topic**: `/{ID_DISPOSITIVO}/metadata`

```json
{
  "timestamp": 1640995200,
  "fecha": "2024-01-01 12:00:00",
  "tipo": "INICIO_SISTEMA",
  "idDispositivo": "ESP32-GASLYT-123456",
  "mac": "AA:BB:CC:DD:EE:FF",
  "version": "1.0.0",
  "pinSensorGas": 36,
  "pinLED": 4,
  "pinBuzzer": 5,
  "pinExtractor": 2,
  "extractorAlambrico": true,
  "intervaloMedicion": 30,
  "umbralAlarma": 1000.0,
  "modoAWS": true,
  "brokerMQTT": "a1b2c3d4e5f6g7.iot.us-east-1.amazonaws.com",
  "puertoMQTT": 8883,
  "estadoFabrica": false
}
```

### 4. Metadata Periódica

**Topic**: `/{ID_DISPOSITIVO}/metadata`

```json
{
  "timestamp": 1640995200,
  "fecha": "2024-01-01 12:00:00",
  "tipo": "METADATA_PERIODICA",
  "idDispositivo": "ESP32-GASLYT-123456",
  "uptime": 3600,
  "rssi": -45,
  "ip": "192.168.1.100",
  "estadoWifi": true,
  "estadoMQTT": true,
  "estadoAlarma": false,
  "ultimaLectura": 150.5
}
```

**Campos**:
- `uptime`: Tiempo de funcionamiento en segundos
- `ip`: Dirección IP del dispositivo
- `estadoWifi`: Estado de conexión WiFi
- `estadoMQTT`: Estado de conexión MQTT
- `estadoAlarma`: Estado actual de alarma
- `ultimaLectura`: Última lectura del sensor

---

## Configuración Remota

### Comandos de Configuración

**Topic**: `/{ID_DISPOSITIVO}/configuracion`

#### Configuración Individual

```json
{
  "intervalo_medicion": 30,
  "umbral_alarma": 1000.0,
  "modo_aws": true,
  "broker_mqtt": "broker.ejemplo.com",
  "puerto_mqtt": 8883,
  "extractor_alambrico": true,
  "pin_extractor": 2,
  "nivel_logging": "INFO"
}
```

#### Configuración Completa

```json
{
  "configuracion_completa": {
    "intervalo_medicion": 30,
    "umbral_alarma": 1000.0,
    "modo_aws": true,
    "broker_mqtt": "broker.ejemplo.com",
    "puerto_mqtt": 8883,
    "extractor_alambrico": true,
    "pin_extractor": 2,
    "nivel_logging": "INFO"
  }
}
```

### Parámetros Configurables

| Parámetro | Tipo | Rango | Descripción |
|-----------|------|-------|-------------|
| `intervalo_medicion` | int | 10-60 | Segundos entre mediciones |
| `umbral_alarma` | float | 0-10000 | PPM para activar alarma |
| `modo_aws` | bool | true/false | Usar AWS IoT Core |
| `broker_mqtt` | string | - | Dirección del broker |
| `puerto_mqtt` | int | 1-65535 | Puerto MQTT |
| `extractor_alambrico` | bool | true/false | Tipo de extractor |
| `pin_extractor` | int | 0-39 | Pin GPIO del extractor |
| `nivel_logging` | string | DEBUG/INFO/WARNING/ERROR | Nivel de logs |

### Respuesta de Configuración

El sistema responde automáticamente con confirmación:

```json
{
  "parametro": "CONFIGURACION",
  "exito": true,
  "mensaje": "Parámetros aplicados: intervalo_medicion umbral_alarma",
  "timestamp": 1640995200
}
```

---

## Sistema OTA y Actualizaciones

### Comandos de Actualización

#### 1. Actualización de Certificados

**Topic**: `/{ID_DISPOSITIVO}/actualizaciones/certificados`

```json
{
  "comando": "actualizar_certificados",
  "version": "1.2.0",
  "certificado": "-----BEGIN CERTIFICATE-----\nMIID...\n-----END CERTIFICATE-----",
  "clave_privada": "-----BEGIN PRIVATE KEY-----\nMIIE...\n-----END PRIVATE KEY-----",
  "certificado_ca": "-----BEGIN CERTIFICATE-----\nMIID...\n-----END CERTIFICATE-----",
  "endpoint": "a1b2c3d4e5f6g7.iot.us-east-1.amazonaws.com",
  "hash": "sha256:abc123def456...",
  "firma": "rsa:def456ghi789...",
  "critica": false
}
```

#### 2. Actualización de Firmware

**Topic**: `/{ID_DISPOSITIVO}/actualizaciones/firmware`

```json
{
  "comando": "actualizar_firmware",
  "version": "2.1.0",
  "url": "https://updates.gaslyt.com/firmware/v2.1.0.bin",
  "hash": "sha256:xyz789abc123...",
  "tamaño": 1048576,
  "descripcion": "Corrección de bugs y nuevas funcionalidades",
  "critica": true,
  "firma": "rsa:ghi789jkl012..."
}
```

#### 3. Verificación de Certificados

```json
{
  "comando": "verificar_certificados"
}
```

#### 4. Verificación de Firmware

```json
{
  "comando": "verificar_firmware"
}
```

#### 5. Rollback

```json
{
  "comando": "rollback"
}
```

### Notificaciones de Estado

#### Estado de Actualización

**Topic**: `/{ID_DISPOSITIVO}/actualizaciones/estado`

```json
{
  "estado": "FIRMWARE_ACTUALIZANDO",
  "mensaje": "Actualizando firmware...",
  "timestamp": 1640995200,
  "dispositivo": "ESP32-GASLYT-123456"
}
```

#### Progreso de Actualización

**Topic**: `/{ID_DISPOSITIVO}/actualizaciones/progreso`

```json
{
  "progreso": 45,
  "descripcion": "Descargando firmware...",
  "timestamp": 1640995200,
  "dispositivo": "ESP32-GASLYT-123456"
}
```

#### Confirmación de Comando

**Topic**: `/{ID_DISPOSITIVO}/actualizaciones/confirmacion`

```json
{
  "comando": "actualizar_firmware",
  "exito": true,
  "mensaje": "Firmware actualizado correctamente",
  "timestamp": 1640995200,
  "dispositivo": "ESP32-GASLYT-123456"
}
```

### Estados del Sistema OTA

| Estado | Descripción |
|--------|-------------|
| `OTA_DISPONIBLE` | Actualización disponible |
| `OTA_DESCARGANDO` | Descargando firmware |
| `OTA_INSTALANDO` | Instalando firmware |
| `OTA_COMPLETADO` | Actualización completada |
| `OTA_ERROR` | Error en la actualización |
| `OTA_ROLLBACK` | Restaurando versión anterior |

---

## Gestión de Certificados

### Estructura de Particiones

```
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

### Flujo de Actualización de Certificados

1. **Verificación**: Validar nueva versión y firma digital
2. **Backup**: Crear backup de certificados actuales
3. **Actualización**: Guardar nuevos certificados en partición
4. **Validación**: Verificar integridad de nuevos certificados
5. **Confirmación**: Notificar éxito o fallo
6. **Rollback**: Restaurar desde backup si falla

### Validación de Certificados

- **Formato**: Verificación de headers PEM
- **Integridad**: Hash SHA-256
- **Firma Digital**: RSA 2048
- **Cadena de Confianza**: Verificación de CA

---

## Estados del Sistema

### Estados de Alarma

| Estado | LED | Sonido | Extractor | Descripción |
|--------|-----|--------|-----------|-------------|
| `NORMAL` | Verde | Ninguno | Inactivo | Funcionamiento normal |
| `ADVERTENCIA` | Amarillo | Intermitente | Inactivo | Concentración elevada |
| `ALARMA` | Rojo | Continuo | Activo | Concentración peligrosa |
| `SIN_WIFI` | Azul | Característico | Inactivo | Sin conexión WiFi |
| `ERROR` | Magenta | Error | Inactivo | Error del sensor |

### Estados de Conexión

| Estado | WiFi | MQTT | Descripción |
|--------|------|------|-------------|
| `CONECTADO` | ✓ | ✓ | Funcionamiento completo |
| `WIFI_ONLY` | ✓ | ✗ | Solo WiFi conectado |
| `DESCONECTADO` | ✗ | ✗ | Sin conexión |
| `RECONECTANDO` | ⚠ | ⚠ | Intentando reconectar |

### Estados de Actualización

| Estado | Descripción | Acción |
|--------|-------------|--------|
| `IDLE` | Sin actualizaciones | Monitoreo normal |
| `VERIFICANDO` | Verificando actualizaciones | Consulta servidor |
| `DESCARGANDO` | Descargando firmware | Progreso visible |
| `INSTALANDO` | Instalando firmware | Reinicio pendiente |
| `COMPLETADO` | Actualización exitosa | Sistema reiniciado |
| `ERROR` | Error en actualización | Rollback automático |

---

## Integración y Desarrollo

### Configuración de Desarrollo

#### 1. Modo DEV
```json
{
  "modo_aws": false,
  "broker_mqtt": "192.168.1.100",
  "puerto_mqtt": 1883,
  "usar_websocket": false
}
```

#### 2. Modo AWS IoT Core
```json
{
  "modo_aws": true,
  "broker_mqtt": "a1b2c3d4e5f6g7.iot.us-east-1.amazonaws.com",
  "puerto_mqtt": 8883,
  "usar_websocket": false
}
```

### Flujo de Integración

#### 1. Configuración Inicial
1. Conectar dispositivo a alimentación
2. Buscar red WiFi "GASLYT-Config"
3. Configurar parámetros vía portal cautivo
4. Dispositivo se conecta a MQTT

#### 2. Monitoreo
1. Dispositivo envía lecturas periódicas
2. Sistema evalúa umbrales
3. Activa alarmas si es necesario
4. Envía metadata del estado

#### 3. Configuración Remota
1. Enviar comando JSON al topic de configuración
2. Dispositivo valida y aplica cambios
3. Responde con confirmación
4. Cambios se aplican inmediatamente

#### 4. Actualizaciones
1. Enviar comando de actualización
2. Dispositivo descarga e instala
3. Verifica integridad
4. Reinicia con nueva versión
5. Rollback automático si falla

### Ejemplos de Integración

#### Python - Cliente MQTT
```python
import paho.mqtt.client as mqtt
import json

def on_connect(client, userdata, flags, rc):
    print(f"Conectado con código {rc}")
    client.subscribe("ESP32-GASLYT-123456/lecturas")
    client.subscribe("ESP32-GASLYT-123456/alarmas")

def on_message(client, userdata, msg):
    data = json.loads(msg.payload.decode())
    print(f"Topic: {msg.topic}")
    print(f"Datos: {data}")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("broker.ejemplo.com", 1883, 60)
client.loop_forever()
```

#### Node.js - Configuración Remota
```javascript
const mqtt = require('mqtt');

const client = mqtt.connect('mqtt://broker.ejemplo.com');

client.on('connect', () => {
    const config = {
        intervalo_medicion: 15,
        umbral_alarma: 800.0,
        nivel_logging: "DEBUG"
    };
    
    client.publish('ESP32-GASLYT-123456/configuracion', JSON.stringify(config));
});

client.on('message', (topic, message) => {
    if (topic.includes('confirmacion')) {
        const response = JSON.parse(message.toString());
        console.log('Configuración aplicada:', response.exito);
    }
});
```

### Consideraciones de Seguridad

#### 1. Certificados AWS IoT Core
- Almacenamiento seguro en particiones dedicadas
- Validación de firma digital
- Backup automático
- Rotación de certificados

#### 2. Comunicación MQTT
- TLS/SSL obligatorio en producción
- Autenticación con certificados
- QoS 2 para comandos críticos
- Validación de mensajes JSON

#### 3. Actualizaciones OTA
- Verificación de hash SHA-256
- Firma digital RSA 2048
- Rollback automático
- Particiones separadas

### Troubleshooting

#### Problemas Comunes

1. **Dispositivo no conecta a WiFi**
   - Verificar credenciales en portal cautivo
   - Comprobar señal WiFi
   - Reiniciar dispositivo

2. **MQTT no conecta**
   - Verificar broker y puerto
   - Comprobar certificados AWS
   - Revisar logs del sistema

3. **Sensor no responde**
   - Verificar conexiones hardware
   - Calibrar sensor en aire limpio
   - Comprobar alimentación

4. **Actualización OTA falla**
   - Verificar espacio disponible
   - Comprobar integridad de descarga
   - Realizar rollback manual

#### Logs del Sistema

```
[12:34:56] [INFO ] [SISTEMA] Sistema inicializado correctamente
[12:34:56] [INFO ] [WIFI] Conectado a red: MiWiFi
[12:34:57] [INFO ] [MQTT] Conectado a broker: broker.ejemplo.com
[12:34:58] [INFO ] [SENSOR] Lectura: 150.5 ppm
[12:35:00] [INFO ] [MQTT] Lectura enviada exitosamente
```

### Métricas y Monitoreo

#### KPIs del Sistema
- **Uptime**: Tiempo de funcionamiento
- **Conectividad**: Porcentaje de tiempo conectado
- **Precisión**: Desviación de lecturas
- **Respuesta**: Tiempo de respuesta a comandos
- **Actualizaciones**: Éxito de actualizaciones OTA

#### Alertas Recomendadas
- Dispositivo desconectado > 5 minutos
- Concentración > umbral configurado
- Error en actualización OTA
- Batería baja (< 20%)
- Sensor no responde

---

## Conclusión

GASLYT es un sistema robusto y escalable para monitoreo de gas inflamable con capacidades avanzadas de comunicación, configuración remota y actualizaciones seguras. Su arquitectura modular permite fácil integración y mantenimiento, mientras que sus características de seguridad garantizan un funcionamiento confiable en entornos industriales.

Para soporte técnico o consultas sobre integración, contactar al equipo de desarrollo.

---

**Versión del Manual**: 1.0  
**Fecha**: Enero 2024  
**Proyecto**: GASLYT - Módulo Integrado  
**Licencia**: MIT
