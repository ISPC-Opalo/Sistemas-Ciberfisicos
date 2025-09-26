#include "ConfigManager.h"

ConfigManager::ConfigManager() : configuracionCargada(false) {
    // Valores por defecto
    configuracion.idDispositivo = "";
    configuracion.intervaloMedicion = 30; // 30 segundos por defecto
    configuracion.umbralAlarma = 1000.0; // ppm por defecto
    configuracion.modoAWS = true;
    configuracion.brokerMQTT = "";
    configuracion.puertoMQTT = 8883;
    configuracion.usarWebSocket = false;
    configuracion.extractorAlambrico = true;
    configuracion.pinExtractor = 2;
    configuracion.pinSensorGas = 36; // ADC1_CH0 en ESP32
    configuracion.pinLED = 4;
    configuracion.pinBuzzer = 5;
}

ConfigManager::~ConfigManager() {
    if (preferences.isOpen()) {
        preferences.end();
    }
}

bool ConfigManager::inicializar() {
    if (!preferences.begin("gaslyt", false)) {
        Serial.println("Error al inicializar Preferences");
        return false;
    }
    
    return cargarConfiguracion();
}

bool ConfigManager::cargarConfiguracion() {
    // Cargar configuración desde Preferences
    configuracion.idDispositivo = preferences.getString("idDispositivo", "");
    configuracion.intervaloMedicion = preferences.getInt("intervaloMed", 30);
    configuracion.umbralAlarma = preferences.getFloat("umbralAlarma", 1000.0);
    configuracion.modoAWS = preferences.getBool("modoAWS", true);
    configuracion.brokerMQTT = preferences.getString("brokerMQTT", "");
    configuracion.puertoMQTT = preferences.getInt("puertoMQTT", 8883);
    configuracion.usarWebSocket = preferences.getBool("usarWebSocket", false);
    configuracion.extractorAlambrico = preferences.getBool("extractorAlambrico", true);
    configuracion.pinExtractor = preferences.getInt("pinExtractor", 2);
    configuracion.pinSensorGas = preferences.getInt("pinSensorGas", 36);
    configuracion.pinLED = preferences.getInt("pinLED", 4);
    configuracion.pinBuzzer = preferences.getInt("pinBuzzer", 5);
    
    // Generar ID del dispositivo si no existe
    if (configuracion.idDispositivo.isEmpty()) {
        configuracion.idDispositivo = generarIdDispositivo();
        guardarConfiguracion();
    }
    
    configuracionCargada = true;
    Serial.println("Configuración cargada exitosamente");
    return true;
}

bool ConfigManager::guardarConfiguracion() {
    if (!preferences.isOpen()) {
        return false;
    }
    
    preferences.putString("idDispositivo", configuracion.idDispositivo);
    preferences.putInt("intervaloMed", configuracion.intervaloMedicion);
    preferences.putFloat("umbralAlarma", configuracion.umbralAlarma);
    preferences.putBool("modoAWS", configuracion.modoAWS);
    preferences.putString("brokerMQTT", configuracion.brokerMQTT);
    preferences.putInt("puertoMQTT", configuracion.puertoMQTT);
    preferences.putBool("usarWebSocket", configuracion.usarWebSocket);
    preferences.putBool("extractorAlambrico", configuracion.extractorAlambrico);
    preferences.putInt("pinExtractor", configuracion.pinExtractor);
    preferences.putInt("pinSensorGas", configuracion.pinSensorGas);
    preferences.putInt("pinLED", configuracion.pinLED);
    preferences.putInt("pinBuzzer", configuracion.pinBuzzer);
    
    Serial.println("Configuración guardada exitosamente");
    return true;
}

void ConfigManager::resetearConfiguracion() {
    preferences.clear();
    configuracionCargada = false;
    
    // Restaurar valores por defecto
    configuracion.idDispositivo = generarIdDispositivo();
    configuracion.intervaloMedicion = 30;
    configuracion.umbralAlarma = 1000.0;
    configuracion.modoAWS = true;
    configuracion.brokerMQTT = "";
    configuracion.puertoMQTT = 8883;
    configuracion.usarWebSocket = false;
    configuracion.extractorAlambrico = true;
    configuracion.pinExtractor = 2;
    configuracion.pinSensorGas = 36;
    configuracion.pinLED = 4;
    configuracion.pinBuzzer = 5;
    
    guardarConfiguracion();
    Serial.println("Configuración reseteada a valores por defecto");
}

// Getters
String ConfigManager::obtenerIdDispositivo() const {
    return configuracion.idDispositivo;
}

int ConfigManager::obtenerIntervaloMedicion() const {
    return configuracion.intervaloMedicion;
}

float ConfigManager::obtenerUmbralAlarma() const {
    return configuracion.umbralAlarma;
}

bool ConfigManager::esModoAWS() const {
    return configuracion.modoAWS;
}

String ConfigManager::obtenerBrokerMQTT() const {
    return configuracion.brokerMQTT;
}

int ConfigManager::obtenerPuertoMQTT() const {
    return configuracion.puertoMQTT;
}

bool ConfigManager::usarWebSocket() const {
    return configuracion.usarWebSocket;
}

bool ConfigManager::esExtractorAlambrico() const {
    return configuracion.extractorAlambrico;
}

int ConfigManager::obtenerPinExtractor() const {
    return configuracion.pinExtractor;
}

int ConfigManager::obtenerPinSensorGas() const {
    return configuracion.pinSensorGas;
}

int ConfigManager::obtenerPinLED() const {
    return configuracion.pinLED;
}

int ConfigManager::obtenerPinBuzzer() const {
    return configuracion.pinBuzzer;
}

// Setters
void ConfigManager::establecerIntervaloMedicion(int intervalo) {
    if (intervalo >= 10 && intervalo <= 60) {
        configuracion.intervaloMedicion = intervalo;
        guardarConfiguracion();
    }
}

void ConfigManager::establecerUmbralAlarma(float umbral) {
    if (umbral > 0) {
        configuracion.umbralAlarma = umbral;
        guardarConfiguracion();
    }
}

void ConfigManager::establecerModoAWS(bool modo) {
    configuracion.modoAWS = modo;
    guardarConfiguracion();
}

void ConfigManager::establecerBrokerMQTT(const String& broker) {
    configuracion.brokerMQTT = broker;
    guardarConfiguracion();
}

void ConfigManager::establecerPuertoMQTT(int puerto) {
    if (puerto > 0 && puerto <= 65535) {
        configuracion.puertoMQTT = puerto;
        guardarConfiguracion();
    }
}

void ConfigManager::establecerUsarWebSocket(bool usar) {
    configuracion.usarWebSocket = usar;
    guardarConfiguracion();
}

void ConfigManager::establecerExtractorAlambrico(bool alambrico) {
    configuracion.extractorAlambrico = alambrico;
    guardarConfiguracion();
}

void ConfigManager::establecerPinExtractor(int pin) {
    if (pin >= 0 && pin <= 39) {
        configuracion.pinExtractor = pin;
        guardarConfiguracion();
    }
}

// Utilidades
String ConfigManager::generarIdDispositivo() {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    String id = "ESP32-GASLYT-" + mac.substring(mac.length() - 6);
    return id;
}

bool ConfigManager::esConfiguracionValida() const {
    return configuracionCargada && 
           !configuracion.idDispositivo.isEmpty() &&
           configuracion.intervaloMedicion >= 10 && 
           configuracion.intervaloMedicion <= 60 &&
           configuracion.umbralAlarma > 0;
}

void ConfigManager::imprimirConfiguracion() const {
    Serial.println("=== CONFIGURACIÓN DEL SISTEMA ===");
    Serial.println("ID Dispositivo: " + configuracion.idDispositivo);
    Serial.println("Intervalo Medición: " + String(configuracion.intervaloMedicion) + " segundos");
    Serial.println("Umbral Alarma: " + String(configuracion.umbralAlarma) + " ppm");
    Serial.println("Modo AWS: " + String(configuracion.modoAWS ? "Sí" : "No"));
    Serial.println("Broker MQTT: " + configuracion.brokerMQTT);
    Serial.println("Puerto MQTT: " + String(configuracion.puertoMQTT));
    Serial.println("Usar WebSocket: " + String(configuracion.usarWebSocket ? "Sí" : "No"));
    Serial.println("Extractor Alámbrico: " + String(configuracion.extractorAlambrico ? "Sí" : "No"));
    Serial.println("Pin Extractor: " + String(configuracion.pinExtractor));
    Serial.println("Pin Sensor Gas: " + String(configuracion.pinSensorGas));
    Serial.println("Pin LED: " + String(configuracion.pinLED));
    Serial.println("Pin Buzzer: " + String(configuracion.pinBuzzer));
    Serial.println("================================");
}
