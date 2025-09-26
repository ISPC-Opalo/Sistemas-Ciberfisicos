#include <Arduino.h>
#include <ArduinoJson.h>
#include "ConfigManager.h"
#include "GasSensor.h"
#include "WiFiManager.h"
#include "MQTTManager.h"
#include "SistemaAlarmas.h"
#include "SistemaLogging.h"
#include "ConfiguracionRemota.h"
#include "CertificadosManager.h"
#include "SistemaOTA.h"
#include "GestorActualizaciones.h"

// Instancias de las clases principales
ConfigManager* configManager;
GasSensor* sensorGas;
WiFiManagerCustom* wifiManager;
MQTTManager* mqttManager;
SistemaAlarmas* sistemaAlarmas;
SistemaLogging* logger;
ConfiguracionRemota* configuracionRemota;
CertificadosManager* certificadosManager;
SistemaOTA* sistemaOTA;
GestorActualizaciones* gestorActualizaciones;

// Variables de control
unsigned long ultimaMedicion = 0;
unsigned long ultimaVerificacionWifi = 0;
unsigned long ultimaVerificacionMQTT = 0;
unsigned long ultimaMetadata = 0;
bool primeraConexion = true;
bool alarmaActiva = false;

// Configuración de tiempos
const unsigned long INTERVALO_VERIFICACION_WIFI = 30000;  // 30 segundos
const unsigned long INTERVALO_VERIFICACION_MQTT = 10000;  // 10 segundos
const unsigned long INTERVALO_METADATA = 300000;          // 5 minutos

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== GASLYT - MÓDULO INTEGRADO ===");
  Serial.println("Iniciando sistema...");
  
  // Inicializar sistema de logging
  logger = &SistemaLoggingSingleton::getInstance();
  logger->inicializar();
  logger->info("SISTEMA", "Sistema GASLYT iniciando...");
  
  // Inicializar gestor de configuración
  configManager = new ConfigManager();
  if (!configManager->inicializar()) {
    logger->error("SISTEMA", "Error al inicializar ConfigManager");
    return;
  }
  
  logger->info("SISTEMA", "ConfigManager inicializado correctamente");
  configManager->imprimirConfiguracion();
  
  // Inicializar sensor de gas
  sensorGas = new GasSensor(configManager->obtenerPinSensorGas(), "MQ-2");
  if (!sensorGas->inicializar()) {
    logger->error("SISTEMA", "Error al inicializar sensor de gas");
    return;
  }
  
  logger->info("SISTEMA", "Sensor de gas inicializado correctamente");
  
  // Configurar umbral del sensor
  sensorGas->establecerUmbral(configManager->obtenerUmbralAlarma());
  logger->info("SENSOR", "Umbral de alarma configurado: " + String(configManager->obtenerUmbralAlarma()) + " ppm");
  
  // Inicializar sistema de alarmas
  sistemaAlarmas = new SistemaAlarmas(
    configManager->obtenerPinLED(),
    configManager->obtenerPinBuzzer(),
    configManager->obtenerPinExtractor(),
    configManager->esExtractorAlambrico()
  );
  
  if (!sistemaAlarmas->inicializar()) {
    logger->error("SISTEMA", "Error al inicializar sistema de alarmas");
    return;
  }
  
  logger->info("SISTEMA", "Sistema de alarmas inicializado correctamente");
  
  // Inicializar WiFi Manager
  wifiManager = new WiFiManagerCustom();
  if (!wifiManager->inicializar()) {
    logger->error("SISTEMA", "Error al inicializar WiFiManager");
    return;
  }
  
  logger->info("SISTEMA", "WiFiManager inicializado correctamente");
  
  // Intentar conectar a WiFi
  if (wifiManager->conectar()) {
    logger->info("WIFI", "WiFi conectado exitosamente");
    logger->info("WIFI", "SSID: " + wifiManager->obtenerSSID());
    logger->info("WIFI", "IP: " + wifiManager->obtenerIP());
    logger->info("WIFI", "RSSI: " + String(wifiManager->obtenerRSSI()) + " dBm");
    sistemaAlarmas->actualizarEstado(SistemaAlarmas::NORMAL);
  } else {
    logger->warning("WIFI", "No se pudo conectar a WiFi, iniciando portal cautivo");
    sistemaAlarmas->actualizarEstado(SistemaAlarmas::SIN_WIFI);
    wifiManager->iniciarPortalCautivo();
  }
  
  // Inicializar MQTT Manager
  mqttManager = new MQTTManager();
  mqttManager->establecerIdDispositivo(configManager->obtenerIdDispositivo());
  
  logger->info("SISTEMA", "MQTTManager inicializado correctamente");
  logger->info("MQTT", "ID Dispositivo: " + configManager->obtenerIdDispositivo());
  
  // Configurar MQTT según el modo
  if (configManager->esModoAWS()) {
    logger->info("MQTT", "Configurando modo AWS IoT Core");
    // Configurar AWS IoT Core (certificados deben estar en el código)
    mqttManager->configurarAWS(
      configManager->obtenerBrokerMQTT(),
      "", // Certificado (debe ser configurado)
      "", // Clave privada (debe ser configurada)
      ""  // Certificado CA (debe ser configurado)
    );
  } else {
    logger->info("MQTT", "Configurando modo DEV");
    // Configurar modo DEV
    mqttManager->configurarDEV(
      configManager->obtenerBrokerMQTT(),
      configManager->obtenerPuertoMQTT(),
      "", // Usuario (opcional)
      ""  // Password (opcional)
    );
  }
  
  if (!mqttManager->inicializar()) {
    logger->error("SISTEMA", "Error al inicializar MQTTManager");
    return;
  }
  
  // Conectar a MQTT
  if (mqttManager->conectar()) {
    logger->info("MQTT", "MQTT conectado exitosamente");
    logger->info("MQTT", "Broker: " + mqttManager->obtenerBroker());
    logger->info("MQTT", "Puerto: " + String(mqttManager->obtenerPuerto()));
    enviarMetadataInicial();
  } else {
    logger->error("MQTT", "No se pudo conectar a MQTT");
  }
  
  // Inicializar configuración remota
  configuracionRemota = new ConfiguracionRemota();
  if (!configuracionRemota->inicializar(configManager, sensorGas, sistemaAlarmas, logger)) {
    logger->error("SISTEMA", "Error al inicializar configuración remota");
    return;
  }
  
  // Inicializar gestor de certificados
  certificadosManager = new CertificadosManager();
  if (!certificadosManager->inicializar()) {
    logger->error("SISTEMA", "Error al inicializar gestor de certificados");
    return;
  }
  
  // Inicializar sistema OTA
  sistemaOTA = new SistemaOTA();
  if (!sistemaOTA->inicializar()) {
    logger->error("SISTEMA", "Error al inicializar sistema OTA");
    return;
  }
  
  // Inicializar gestor de actualizaciones
  gestorActualizaciones = new GestorActualizaciones();
  if (!gestorActualizaciones->inicializar(certificadosManager, sistemaOTA, mqttManager, logger)) {
    logger->error("SISTEMA", "Error al inicializar gestor de actualizaciones");
    return;
  }
  
  // Configurar callbacks MQTT
  mqttManager->establecerCallbackConfiguracion([](const String& payload) {
    configuracionRemota->procesarMensajeConfiguracion(payload);
  });
  
  mqttManager->establecerCallbackActualizaciones([](const String& payload) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      gestorActualizaciones->procesarComandoActualizacion(doc.as<JsonObject>());
    }
  });
  
  logger->info("SISTEMA", "Sistema inicializado correctamente");
  logger->info("SISTEMA", "ID Dispositivo: " + configManager->obtenerIdDispositivo());
  logger->info("SISTEMA", "Intervalo de medición: " + String(configManager->obtenerIntervaloMedicion()) + " segundos");
  logger->info("SISTEMA", "Umbral de alarma: " + String(configManager->obtenerUmbralAlarma()) + " ppm");
  
  // Imprimir estado completo del sistema
  logger->logEstadoSistema();
  logger->logEstadoSensor();
  logger->logEstadoWiFi();
  logger->logEstadoMQTT();
  logger->logEstadoAlarmas();
  logger->logEstadoConfiguracion();
  
  // Imprimir estado de actualizaciones
  if (gestorActualizaciones) {
    gestorActualizaciones->imprimirEstado();
    gestorActualizaciones->imprimirConfiguracion();
  }
  
  if (certificadosManager) {
    certificadosManager->imprimirEstado();
  }
  
  if (sistemaOTA) {
    sistemaOTA->imprimirEstado();
  }
}

void loop() {
  unsigned long tiempoActual = millis();
  
  // Verificar conexión WiFi periódicamente
  if (tiempoActual - ultimaVerificacionWifi >= INTERVALO_VERIFICACION_WIFI) {
    ultimaVerificacionWifi = tiempoActual;
    
    if (!wifiManager->verificarConexion()) {
      sistemaAlarmas->actualizarEstado(SistemaAlarmas::SIN_WIFI);
      logger->warning("WIFI", "WiFi desconectado");
    } else {
      if (sistemaAlarmas->obtenerEstadoActual() == SistemaAlarmas::SIN_WIFI) {
        sistemaAlarmas->actualizarEstado(SistemaAlarmas::NORMAL);
        logger->info("WIFI", "WiFi reconectado");
      }
    }
  }
  
  // Verificar conexión MQTT periódicamente
  if (tiempoActual - ultimaVerificacionMQTT >= INTERVALO_VERIFICACION_MQTT) {
    ultimaVerificacionMQTT = tiempoActual;
    
    if (wifiManager->estaConectado()) {
      if (!mqttManager->verificarConexion()) {
        mqttManager->reconectar();
      }
      mqttManager->procesarMensajes();
    }
  }
  
  // Realizar medición según el intervalo configurado
  if (tiempoActual - ultimaMedicion >= (configManager->obtenerIntervaloMedicion() * 1000)) {
    ultimaMedicion = tiempoActual;
    logger->debug("SENSOR", "Iniciando medición programada");
    realizarMedicion();
  }
  
  // Enviar metadata periódicamente
  if (tiempoActual - ultimaMetadata >= INTERVALO_METADATA) {
    ultimaMetadata = tiempoActual;
    enviarMetadata();
  }
  
  // Procesar alarmas
  sistemaAlarmas->procesarAlarmas();
  
  // Verificar actualizaciones periódicas
  if (gestorActualizaciones) {
    gestorActualizaciones->verificarActualizacionesPeriodicas();
  }
  
  // Pequeña pausa para evitar sobrecarga
  delay(100);
}

void realizarMedicion() {
  if (!sensorGas) {
    logger->error("SENSOR", "Sensor de gas no inicializado");
    return;
  }
  
  logger->debug("SENSOR", "Realizando medición de gas...");
  
  // Leer concentración de gas
  float concentracion = sensorGas->leerConcentracion();
  
  if (concentracion < 0) {
    logger->error("SENSOR", "Error en la lectura del sensor");
    sistemaAlarmas->actualizarEstado(SistemaAlarmas::ERROR_SENSOR);
    return;
  }
  
  logger->info("SENSOR", "Concentración medida: " + String(concentracion) + " ppm");
  
  // Verificar umbral
  bool superaUmbral = sensorGas->verificarUmbral();
  
  // Actualizar estado del sistema
  if (superaUmbral) {
    if (!alarmaActiva) {
      alarmaActiva = true;
      sistemaAlarmas->actualizarEstado(SistemaAlarmas::ALARMA);
      logger->warning("ALARMAS", "¡ALARMA! Concentración de gas supera el umbral: " + String(concentracion) + " ppm");
    }
  } else {
    if (alarmaActiva) {
      alarmaActiva = false;
      sistemaAlarmas->actualizarEstado(SistemaAlarmas::NORMAL);
      logger->info("ALARMAS", "Concentración de gas normalizada: " + String(concentracion) + " ppm");
    }
  }
  
  // Imprimir lectura detallada
  sensorGas->imprimirLectura();
  
  // Enviar lectura por MQTT
  if (wifiManager->estaConectado() && mqttManager->estaConectado()) {
    logger->debug("MQTT", "Enviando lectura por MQTT");
    enviarLectura(concentracion, superaUmbral);
  } else {
    logger->warning("MQTT", "No se puede enviar lectura - WiFi o MQTT desconectado");
  }
}

void enviarLectura(float concentracion, bool alarma) {
  if (!mqttManager) {
    logger->error("MQTT", "MQTTManager no inicializado");
    return;
  }
  
  logger->debug("MQTT", "Preparando envío de lectura");
  
  // Crear JSON con los datos de la lectura
  DynamicJsonDocument doc(1024);
  doc["timestamp"] = wifiManager->obtenerTimestamp();
  doc["fecha"] = wifiManager->obtenerHoraActual();
  doc["concentracion"] = concentracion;
  doc["unidad"] = "ppm";
  doc["umbral"] = configManager->obtenerUmbralAlarma();
  doc["alarma"] = alarma;
  doc["idDispositivo"] = configManager->obtenerIdDispositivo();
  doc["rssi"] = wifiManager->obtenerRSSI();
  
  // Publicar lectura
  JsonObject obj = doc.as<JsonObject>();
  if (mqttManager->publicarLectura(obj)) {
    logger->info("MQTT", "Lectura enviada exitosamente: " + String(concentracion) + " ppm");
  } else {
    logger->error("MQTT", "Error al enviar lectura");
  }
  
  // Si hay alarma, enviar también al topic de alarmas
  if (alarma) {
    logger->warning("MQTT", "Enviando alarma por MQTT");
    enviarAlarma(concentracion);
  }
}

void enviarAlarma(float concentracion) {
  if (!mqttManager) {
    logger->error("MQTT", "MQTTManager no inicializado para alarma");
    return;
  }
  
  logger->warning("MQTT", "Preparando envío de alarma");
  
  // Crear JSON con los datos de la alarma
  DynamicJsonDocument doc(1024);
  doc["timestamp"] = wifiManager->obtenerTimestamp();
  doc["fecha"] = wifiManager->obtenerHoraActual();
  doc["tipo"] = "GAS_INFLAMABLE";
  doc["concentracion"] = concentracion;
  doc["umbral"] = configManager->obtenerUmbralAlarma();
  doc["severidad"] = "ALTA";
  doc["idDispositivo"] = configManager->obtenerIdDispositivo();
  doc["accion"] = "EXTRACTOR_ACTIVADO";
  
  // Publicar alarma
  JsonObject obj = doc.as<JsonObject>();
  if (mqttManager->publicarAlarma(obj)) {
    logger->warning("MQTT", "Alarma enviada exitosamente: " + String(concentracion) + " ppm");
  } else {
    logger->error("MQTT", "Error al enviar alarma");
  }
}

void enviarMetadataInicial() {
  if (!mqttManager) {
    logger->error("MQTT", "MQTTManager no inicializado para metadata");
    return;
  }
  
  logger->info("MQTT", "Preparando envío de metadata inicial");
  
  // Crear JSON con metadata inicial
  DynamicJsonDocument doc(1024);
  doc["timestamp"] = wifiManager->obtenerTimestamp();
  doc["fecha"] = wifiManager->obtenerHoraActual();
  doc["tipo"] = "INICIO_SISTEMA";
  doc["idDispositivo"] = configManager->obtenerIdDispositivo();
  doc["mac"] = WiFi.macAddress();
  doc["version"] = "1.0.0";
  doc["pinSensorGas"] = configManager->obtenerPinSensorGas();
  doc["pinLED"] = configManager->obtenerPinLED();
  doc["pinBuzzer"] = configManager->obtenerPinBuzzer();
  doc["pinExtractor"] = configManager->obtenerPinExtractor();
  doc["extractorAlambrico"] = configManager->esExtractorAlambrico();
  doc["intervaloMedicion"] = configManager->obtenerIntervaloMedicion();
  doc["umbralAlarma"] = configManager->obtenerUmbralAlarma();
  doc["modoAWS"] = configManager->esModoAWS();
  doc["brokerMQTT"] = configManager->obtenerBrokerMQTT();
  doc["puertoMQTT"] = configManager->obtenerPuertoMQTT();
  doc["estadoFabrica"] = false; // Cambiar a true si es necesario
  
  // Publicar metadata
  JsonObject obj = doc.as<JsonObject>();
  if (mqttManager->publicarMetadata(obj)) {
    logger->info("MQTT", "Metadata inicial enviada exitosamente");
  } else {
    logger->error("MQTT", "Error al enviar metadata inicial");
  }
}

void enviarMetadata() {
  if (!mqttManager) {
    logger->error("MQTT", "MQTTManager no inicializado para metadata periódica");
    return;
  }
  
  logger->debug("MQTT", "Preparando envío de metadata periódica");
  
  // Crear JSON con metadata periódica
  DynamicJsonDocument doc(1024);
  doc["timestamp"] = wifiManager->obtenerTimestamp();
  doc["fecha"] = wifiManager->obtenerHoraActual();
  doc["tipo"] = "METADATA_PERIODICA";
  doc["idDispositivo"] = configManager->obtenerIdDispositivo();
  doc["uptime"] = millis() / 1000;
  doc["rssi"] = wifiManager->obtenerRSSI();
  doc["ip"] = wifiManager->obtenerIP();
  doc["estadoWifi"] = wifiManager->estaConectado();
  doc["estadoMQTT"] = mqttManager->estaConectado();
  doc["estadoAlarma"] = alarmaActiva;
  doc["ultimaLectura"] = sensorGas->obtenerUltimaLectura();
  
  // Publicar metadata
  JsonObject obj = doc.as<JsonObject>();
  if (mqttManager->publicarMetadata(obj)) {
    logger->info("MQTT", "Metadata periódica enviada exitosamente");
  } else {
    logger->error("MQTT", "Error al enviar metadata periódica");
  }
}