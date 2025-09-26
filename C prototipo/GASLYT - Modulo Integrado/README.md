# GASLYT - Módulo Integrado

Sistema de monitoreo de gas inflamable basado en ESP32 con sensores MQ, comunicación WiFi/MQTT y optimización de energía para funcionamiento con batería.

## Características

- **Sensor de Gas**: Soporte para sensores MQ (MQ-2, MQ-3, MQ-4, MQ-5, MQ-6, MQ-7, MQ-8, MQ-9, MQ-135)
- **Comunicación**: WiFi con portal cautivo y MQTT (AWS IoT Core y modo DEV)
- **Alarmas**: LED RGB, buzzer y control de extractor
- **Optimización de Energía**: Funcionamiento optimizado para batería
- **Configuración**: Portal cautivo web para configuración
- **Persistencia**: Configuraciones guardadas en Preferences

## Hardware Requerido

- ESP32 DevKit
- Sensor MQ (recomendado MQ-2 para gas inflamable)
- LED RGB (WS2812B)
- Buzzer
- Extractor (opcional, alámbrico o inalámbrico)
- Fuente de alimentación (batería recomendada)

## Conexiones

### Pines por Defecto
- **Sensor MQ**: Pin 36 (ADC1_CH0)
- **LED RGB**: Pin 4
- **Buzzer**: Pin 5
- **Extractor**: Pin 2 (si es alámbrico)

### Sensor MQ
- **VCC**: 3.3V o 5V (según sensor)
- **GND**: GND
- **A0**: Pin 36 (ADC)

## Configuración

### Portal Cautivo
1. Conecte el dispositivo a la alimentación
2. Busque la red WiFi "GASLYT-Config"
3. Conéctese y abra el navegador
4. Configure los parámetros:
   - Intervalo de medición (10-60 segundos)
   - Umbral de alarma (ppm)
   - Modo AWS o DEV
   - Configuración MQTT
   - Configuración del extractor

### Modo AWS IoT Core
1. Configure certificados en el código
2. Establezca el endpoint AWS
3. Habilite modo AWS en el portal

### Modo DEV
1. Configure broker MQTT local
2. Establezca puerto (1883 para MQTT, 8883 para MQTTs)
3. Configure usuario/password si es necesario

## Topics MQTT

- `/{ID_DISPOSITIVO}/lecturas` - Mediciones normales (QoS 0)
- `/{ID_DISPOSITIVO}/alarmas` - Alarmas (QoS 2)
- `/{ID_DISPOSITIVO}/metadata` - Metadata del dispositivo (QoS 2)
- `/{ID_DISPOSITIVO}/configuracion` - Configuración remota (QoS 2)

## Formato de Mensajes

### Lectura
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

### Alarma
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

### Configuración Remota
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

## Estados del Sistema

- **Normal**: LED verde, sin sonido
- **Advertencia**: LED amarillo, sonido intermitente
- **Alarma**: LED rojo, sonido continuo, extractor activado
- **Sin WiFi**: LED azul, sonido característico
- **Error**: LED magenta, sonido de error

## Optimización de Energía

- Frecuencia de CPU ajustable (40-240 MHz)
- Deshabilitación automática de WiFi en inactividad
- Modo sueño profundo
- Control de brillo LED
- Monitoreo de batería

## Sistema de Logging

- **Niveles**: DEBUG, INFO, WARNING, ERROR
- **Componentes**: SISTEMA, SENSOR, WIFI, MQTT, ALARMAS, CONFIG, ENERGIA
- **Formato**: [TIMESTAMP] [NIVEL] [COMPONENTE] MENSAJE
- **Colores**: Cyan (DEBUG), Verde (INFO), Amarillo (WARNING), Rojo (ERROR)
- **Configuración remota**: Nivel de logging configurable vía MQTT

## Configuración Remota

- **Topic**: `/{ID_DISPOSITIVO}/configuracion`
- **Parámetros configurables**:
  - `intervalo_medicion`: 10-60 segundos
  - `umbral_alarma`: 0-10000 ppm
  - `modo_aws`: true/false
  - `broker_mqtt`: string
  - `puerto_mqtt`: 1-65535
  - `extractor_alambrico`: true/false
  - `pin_extractor`: 0-39
  - `nivel_logging`: DEBUG/INFO/WARNING/ERROR
- **Validación**: Todos los parámetros son validados antes de aplicar
- **Confirmación**: Respuesta automática con estado de la configuración

## Compilación

```bash
# Instalar dependencias
pio lib install

# Compilar
pio run

# Subir al dispositivo
pio run --target upload

# Monitorear serial
pio device monitor
```

## Dependencias

- `miguel5612/MQSensorsLib@^3.0.0` - Librería para sensores MQ
- `knolleary/PubSubClient@^2.8` - Cliente MQTT
- `tzapu/WiFiManager@^2.0.16` - Portal cautivo WiFi
- `bblanchon/ArduinoJson@^6.21.3` - Manejo de JSON
- `adafruit/Adafruit NeoPixel@^1.10.6` - Control LED RGB

## Estructura del Proyecto

```
├── include/           # Headers de clases
├── src/              # Implementación de clases
├── platformio.ini    # Configuración PlatformIO
├── partitions.csv    # Particiones para certificados AWS
└── README.md         # Este archivo
```

## Licencia

MIT License

## Soporte

Para soporte técnico o consultas, contactar al equipo de desarrollo.
