#include "ConfiguracionRemota.h"

ConfiguracionRemota::ConfiguracionRemota() : 
    configManager(nullptr), sensorGas(nullptr), sistemaAlarmas(nullptr), logger(nullptr),
    topicConfiguracion(""), configuracionRecibida(false), ultimaConfiguracion(0),
    callbackConfiguracionCambiada(nullptr) {
}

ConfiguracionRemota::~ConfiguracionRemota() {
}

bool ConfiguracionRemota::inicializar(ConfigManager* config, GasSensor* sensor, 
                                    SistemaAlarmas* alarmas, SistemaLogging* log) {
    if (!config || !sensor || !alarmas || !log) {
        return false;
    }
    
    configManager = config;
    sensorGas = sensor;
    sistemaAlarmas = alarmas;
    logger = log;
    
    // Configurar topic de configuración
    topicConfiguracion = "/" + configManager->obtenerIdDispositivo() + "/configuracion";
    
    logger->info("CONFIG_REMOTA", "Sistema de configuración remota inicializado");
    logger->info("CONFIG_REMOTA", "Topic configuración: " + topicConfiguracion);
    
    return true;
}

void ConfiguracionRemota::procesarMensajeConfiguracion(const String& payload) {
    if (!configManager || !logger) {
        return;
    }
    
    logger->info("CONFIG_REMOTA", "Procesando mensaje de configuración remota");
    logger->debug("CONFIG_REMOTA", "Payload recibido: " + payload);
    
    // Parsear JSON
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
        logger->error("CONFIG_REMOTA", "Error al parsear JSON: " + String(error.c_str()));
        enviarConfirmacionConfiguracion("JSON", false, "Error de formato JSON");
        return;
    }
    
    JsonObject config = doc.as<JsonObject>();
    ultimaConfiguracion = millis();
    configuracionRecibida = true;
    
    // Procesar configuración completa
    if (config.containsKey("configuracion_completa")) {
        procesarConfiguracionCompleta(config["configuracion_completa"]);
        return;
    }
    
    // Procesar configuraciones individuales
    bool exito = true;
    String parametrosProcesados = "";
    
    if (config.containsKey("intervalo_medicion")) {
        int intervalo = config["intervalo_medicion"];
        if (procesarIntervaloMedicion(intervalo)) {
            parametrosProcesados += "intervalo_medicion ";
        } else {
            exito = false;
        }
    }
    
    if (config.containsKey("umbral_alarma")) {
        float umbral = config["umbral_alarma"];
        if (procesarUmbralAlarma(umbral)) {
            parametrosProcesados += "umbral_alarma ";
        } else {
            exito = false;
        }
    }
    
    if (config.containsKey("modo_aws")) {
        bool modo = config["modo_aws"];
        if (procesarModoAWS(modo)) {
            parametrosProcesados += "modo_aws ";
        } else {
            exito = false;
        }
    }
    
    if (config.containsKey("broker_mqtt")) {
        String broker = config["broker_mqtt"];
        if (procesarBrokerMQTT(broker)) {
            parametrosProcesados += "broker_mqtt ";
        } else {
            exito = false;
        }
    }
    
    if (config.containsKey("puerto_mqtt")) {
        int puerto = config["puerto_mqtt"];
        if (procesarPuertoMQTT(puerto)) {
            parametrosProcesados += "puerto_mqtt ";
        } else {
            exito = false;
        }
    }
    
    if (config.containsKey("extractor_alambrico")) {
        bool alambrico = config["extractor_alambrico"];
        if (procesarExtractorAlambrico(alambrico)) {
            parametrosProcesados += "extractor_alambrico ";
        } else {
            exito = false;
        }
    }
    
    if (config.containsKey("pin_extractor")) {
        int pin = config["pin_extractor"];
        if (procesarPinExtractor(pin)) {
            parametrosProcesados += "pin_extractor ";
        } else {
            exito = false;
        }
    }
    
    if (config.containsKey("nivel_logging")) {
        String nivel = config["nivel_logging"];
        if (procesarNivelLogging(nivel)) {
            parametrosProcesados += "nivel_logging ";
        } else {
            exito = false;
        }
    }
    
    // Enviar confirmación
    if (exito) {
        logger->info("CONFIG_REMOTA", "Configuración aplicada exitosamente: " + parametrosProcesados);
        enviarConfirmacionConfiguracion("CONFIGURACION", true, "Parámetros aplicados: " + parametrosProcesados);
    } else {
        logger->warning("CONFIG_REMOTA", "Algunos parámetros no pudieron ser aplicados");
        enviarConfirmacionConfiguracion("CONFIGURACION", false, "Algunos parámetros fallaron");
    }
    
    // Notificar callback
    if (callbackConfiguracionCambiada) {
        callbackConfiguracionCambiada("CONFIGURACION", exito ? "EXITOSO" : "FALLIDO");
    }
}

void ConfiguracionRemota::establecerTopicConfiguracion(const String& topic) {
    topicConfiguracion = topic;
    if (logger) {
        logger->info("CONFIG_REMOTA", "Topic de configuración establecido: " + topic);
    }
}

void ConfiguracionRemota::establecerCallback(void (*callback)(const String&, const String&)) {
    callbackConfiguracionCambiada = callback;
}

bool ConfiguracionRemota::procesarIntervaloMedicion(int intervalo) {
    if (!validarIntervaloMedicion(intervalo)) {
        logger->warning("CONFIG_REMOTA", "Intervalo de medición inválido: " + String(intervalo));
        return false;
    }
    
    configManager->establecerIntervaloMedicion(intervalo);
    logger->info("CONFIG_REMOTA", "Intervalo de medición actualizado a: " + String(intervalo) + " segundos");
    
    if (callbackConfiguracionCambiada) {
        callbackConfiguracionCambiada("intervalo_medicion", String(intervalo));
    }
    
    return true;
}

bool ConfiguracionRemota::procesarUmbralAlarma(float umbral) {
    if (!validarUmbralAlarma(umbral)) {
        logger->warning("CONFIG_REMOTA", "Umbral de alarma inválido: " + String(umbral));
        return false;
    }
    
    configManager->establecerUmbralAlarma(umbral);
    sensorGas->establecerUmbral(umbral);
    logger->info("CONFIG_REMOTA", "Umbral de alarma actualizado a: " + String(umbral) + " ppm");
    
    if (callbackConfiguracionCambiada) {
        callbackConfiguracionCambiada("umbral_alarma", String(umbral));
    }
    
    return true;
}

bool ConfiguracionRemota::procesarModoAWS(bool modo) {
    configManager->establecerModoAWS(modo);
    logger->info("CONFIG_REMOTA", "Modo AWS " + String(modo ? "habilitado" : "deshabilitado"));
    
    if (callbackConfiguracionCambiada) {
        callbackConfiguracionCambiada("modo_aws", String(modo ? "true" : "false"));
    }
    
    return true;
}

bool ConfiguracionRemota::procesarBrokerMQTT(const String& broker) {
    if (broker.isEmpty()) {
        logger->warning("CONFIG_REMOTA", "Broker MQTT no puede estar vacío");
        return false;
    }
    
    configManager->establecerBrokerMQTT(broker);
    logger->info("CONFIG_REMOTA", "Broker MQTT actualizado a: " + broker);
    
    if (callbackConfiguracionCambiada) {
        callbackConfiguracionCambiada("broker_mqtt", broker);
    }
    
    return true;
}

bool ConfiguracionRemota::procesarPuertoMQTT(int puerto) {
    if (!validarPuertoMQTT(puerto)) {
        logger->warning("CONFIG_REMOTA", "Puerto MQTT inválido: " + String(puerto));
        return false;
    }
    
    configManager->establecerPuertoMQTT(puerto);
    logger->info("CONFIG_REMOTA", "Puerto MQTT actualizado a: " + String(puerto));
    
    if (callbackConfiguracionCambiada) {
        callbackConfiguracionCambiada("puerto_mqtt", String(puerto));
    }
    
    return true;
}

bool ConfiguracionRemota::procesarExtractorAlambrico(bool alambrico) {
    configManager->establecerExtractorAlambrico(alambrico);
    sistemaAlarmas->establecerExtractorAlambrico(alambrico);
    logger->info("CONFIG_REMOTA", "Extractor " + String(alambrico ? "alámbrico" : "inalámbrico") + " configurado");
    
    if (callbackConfiguracionCambiada) {
        callbackConfiguracionCambiada("extractor_alambrico", String(alambrico ? "true" : "false"));
    }
    
    return true;
}

bool ConfiguracionRemota::procesarPinExtractor(int pin) {
    if (!validarPinExtractor(pin)) {
        logger->warning("CONFIG_REMOTA", "Pin extractor inválido: " + String(pin));
        return false;
    }
    
    configManager->establecerPinExtractor(pin);
    sistemaAlarmas->establecerPinExtractor(pin);
    logger->info("CONFIG_REMOTA", "Pin extractor actualizado a: " + String(pin));
    
    if (callbackConfiguracionCambiada) {
        callbackConfiguracionCambiada("pin_extractor", String(pin));
    }
    
    return true;
}

bool ConfiguracionRemota::procesarNivelLogging(const String& nivel) {
    if (!validarNivelLogging(nivel)) {
        logger->warning("CONFIG_REMOTA", "Nivel de logging inválido: " + nivel);
        return false;
    }
    
    SistemaLogging::NivelLog nivelLog;
    if (nivel == "DEBUG") {
        nivelLog = SistemaLogging::DEBUG;
    } else if (nivel == "INFO") {
        nivelLog = SistemaLogging::INFO;
    } else if (nivel == "WARNING") {
        nivelLog = SistemaLogging::WARNING;
    } else if (nivel == "ERROR") {
        nivelLog = SistemaLogging::ERROR;
    }
    
    logger->establecerNivel(nivelLog);
    logger->info("CONFIG_REMOTA", "Nivel de logging actualizado a: " + nivel);
    
    if (callbackConfiguracionCambiada) {
        callbackConfiguracionCambiada("nivel_logging", nivel);
    }
    
    return true;
}

bool ConfiguracionRemota::procesarConfiguracionCompleta(const JsonObject& config) {
    logger->info("CONFIG_REMOTA", "Procesando configuración completa");
    
    bool exito = true;
    int parametrosProcesados = 0;
    
    if (config.containsKey("intervalo_medicion")) {
        if (procesarIntervaloMedicion(config["intervalo_medicion"])) {
            parametrosProcesados++;
        } else {
            exito = false;
        }
    }
    
    if (config.containsKey("umbral_alarma")) {
        if (procesarUmbralAlarma(config["umbral_alarma"])) {
            parametrosProcesados++;
        } else {
            exito = false;
        }
    }
    
    if (config.containsKey("modo_aws")) {
        if (procesarModoAWS(config["modo_aws"])) {
            parametrosProcesados++;
        } else {
            exito = false;
        }
    }
    
    if (config.containsKey("broker_mqtt")) {
        if (procesarBrokerMQTT(config["broker_mqtt"])) {
            parametrosProcesados++;
        } else {
            exito = false;
        }
    }
    
    if (config.containsKey("puerto_mqtt")) {
        if (procesarPuertoMQTT(config["puerto_mqtt"])) {
            parametrosProcesados++;
        } else {
            exito = false;
        }
    }
    
    if (config.containsKey("extractor_alambrico")) {
        if (procesarExtractorAlambrico(config["extractor_alambrico"])) {
            parametrosProcesados++;
        } else {
            exito = false;
        }
    }
    
    if (config.containsKey("pin_extractor")) {
        if (procesarPinExtractor(config["pin_extractor"])) {
            parametrosProcesados++;
        } else {
            exito = false;
        }
    }
    
    if (config.containsKey("nivel_logging")) {
        if (procesarNivelLogging(config["nivel_logging"])) {
            parametrosProcesados++;
        } else {
            exito = false;
        }
    }
    
    logger->info("CONFIG_REMOTA", "Configuración completa procesada: " + String(parametrosProcesados) + " parámetros");
    enviarConfirmacionConfiguracion("CONFIGURACION_COMPLETA", exito, 
                                   "Procesados " + String(parametrosProcesados) + " parámetros");
    
    return exito;
}

// Validaciones
bool ConfiguracionRemota::validarIntervaloMedicion(int intervalo) {
    return intervalo >= 10 && intervalo <= 60;
}

bool ConfiguracionRemota::validarUmbralAlarma(float umbral) {
    return umbral > 0 && umbral <= 10000;
}

bool ConfiguracionRemota::validarPuertoMQTT(int puerto) {
    return puerto > 0 && puerto <= 65535;
}

bool ConfiguracionRemota::validarPinExtractor(int pin) {
    return pin >= 0 && pin <= 39;
}

bool ConfiguracionRemota::validarNivelLogging(const String& nivel) {
    return nivel == "DEBUG" || nivel == "INFO" || nivel == "WARNING" || nivel == "ERROR";
}

void ConfiguracionRemota::enviarConfirmacionConfiguracion(const String& parametro, bool exito, const String& mensaje) {
    // Esta función debería enviar la confirmación por MQTT
    // Por ahora solo logueamos
    if (exito) {
        logger->info("CONFIG_REMOTA", "Confirmación: " + parametro + " - " + mensaje);
    } else {
        logger->error("CONFIG_REMOTA", "Error: " + parametro + " - " + mensaje);
    }
}

void ConfiguracionRemota::enviarEstadoConfiguracion() {
    logger->info("CONFIG_REMOTA", "Enviando estado de configuración actual");
    // Implementar envío de estado por MQTT
}

// Utilidades
String ConfiguracionRemota::obtenerTopicConfiguracion() const {
    return topicConfiguracion;
}

bool ConfiguracionRemota::esConfiguracionRecibida() const {
    return configuracionRecibida;
}

unsigned long ConfiguracionRemota::obtenerTiempoUltimaConfiguracion() const {
    return ultimaConfiguracion;
}

void ConfiguracionRemota::imprimirConfiguracionesDisponibles() const {
    logger->info("CONFIG_REMOTA", "Configuraciones disponibles:");
    logger->info("CONFIG_REMOTA", "- intervalo_medicion: 10-60 segundos");
    logger->info("CONFIG_REMOTA", "- umbral_alarma: 0-10000 ppm");
    logger->info("CONFIG_REMOTA", "- modo_aws: true/false");
    logger->info("CONFIG_REMOTA", "- broker_mqtt: string");
    logger->info("CONFIG_REMOTA", "- puerto_mqtt: 1-65535");
    logger->info("CONFIG_REMOTA", "- extractor_alambrico: true/false");
    logger->info("CONFIG_REMOTA", "- pin_extractor: 0-39");
    logger->info("CONFIG_REMOTA", "- nivel_logging: DEBUG/INFO/WARNING/ERROR");
}

// Getters
ConfigManager* ConfiguracionRemota::obtenerConfigManager() const {
    return configManager;
}

GasSensor* ConfiguracionRemota::obtenerSensorGas() const {
    return sensorGas;
}

SistemaAlarmas* ConfiguracionRemota::obtenerSistemaAlarmas() const {
    return sistemaAlarmas;
}

SistemaLogging* ConfiguracionRemota::obtenerLogger() const {
    return logger;
}
