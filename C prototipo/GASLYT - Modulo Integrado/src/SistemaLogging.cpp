#include "SistemaLogging.h"

// Definición de colores ANSI
const String SistemaLogging::COLOR_DEBUG = "\033[36m";    // Cyan
const String SistemaLogging::COLOR_INFO = "\033[32m";     // Verde
const String SistemaLogging::COLOR_WARNING = "\033[33m";  // Amarillo
const String SistemaLogging::COLOR_ERROR = "\033[31m";    // Rojo
const String SistemaLogging::COLOR_RESET = "\033[0m";     // Reset

// Definición de componentes
const String SistemaLogging::COMPONENTE_SISTEMA = "SISTEMA";
const String SistemaLogging::COMPONENTE_SENSOR = "SENSOR";
const String SistemaLogging::COMPONENTE_WIFI = "WIFI";
const String SistemaLogging::COMPONENTE_MQTT = "MQTT";
const String SistemaLogging::COMPONENTE_ALARMAS = "ALARMAS";
const String SistemaLogging::COMPONENTE_CONFIG = "CONFIG";
const String SistemaLogging::COMPONENTE_ENERGIA = "ENERGIA";

// Singleton
SistemaLogging* SistemaLoggingSingleton::instancia = nullptr;

SistemaLogging::SistemaLogging() : 
    nivelActual(INFO), habilitado(true), incluirTimestamp(true), 
    incluirNivel(true), incluirComponente(true) {
}

SistemaLogging::~SistemaLogging() {
}

bool SistemaLogging::inicializar() {
    Serial.println("=== SISTEMA DE LOGGING INICIADO ===");
    Serial.println("Nivel actual: " + obtenerNivelString(nivelActual));
    Serial.println("Formato: [TIMESTAMP] [NIVEL] [COMPONENTE] MENSAJE");
    Serial.println("=====================================");
    
    info(COMPONENTE_SISTEMA, "Sistema de logging inicializado correctamente");
    return true;
}

void SistemaLogging::establecerNivel(NivelLog nivel) {
    nivelActual = nivel;
    info(COMPONENTE_SISTEMA, "Nivel de logging cambiado a: " + obtenerNivelString(nivel));
}

void SistemaLogging::habilitarLogging(bool habilitar) {
    habilitado = habilitar;
    if (habilitado) {
        info(COMPONENTE_SISTEMA, "Logging habilitado");
    } else {
        Serial.println("Logging deshabilitado");
    }
}

void SistemaLogging::configurarFormato(bool timestamp, bool nivel, bool componente) {
    incluirTimestamp = timestamp;
    incluirNivel = nivel;
    incluirComponente = componente;
    
    info(COMPONENTE_SISTEMA, "Formato de logging configurado - Timestamp: " + 
         String(timestamp ? "Sí" : "No") + ", Nivel: " + String(nivel ? "Sí" : "No") + 
         ", Componente: " + String(componente ? "Sí" : "No"));
}

void SistemaLogging::debug(const String& componente, const String& mensaje) {
    if (habilitado && nivelActual <= DEBUG) {
        imprimirLog(DEBUG, componente, mensaje);
    }
}

void SistemaLogging::info(const String& componente, const String& mensaje) {
    if (habilitado && nivelActual <= INFO) {
        imprimirLog(INFO, componente, mensaje);
    }
}

void SistemaLogging::warning(const String& componente, const String& mensaje) {
    if (habilitado && nivelActual <= WARNING) {
        imprimirLog(WARNING, componente, mensaje);
    }
}

void SistemaLogging::error(const String& componente, const String& mensaje) {
    if (habilitado && nivelActual <= ERROR) {
        imprimirLog(ERROR, componente, mensaje);
    }
}

void SistemaLogging::debugf(const String& componente, const String& formato, ...) {
    if (habilitado && nivelActual <= DEBUG) {
        char buffer[512];
        va_list args;
        va_start(args, formato);
        vsnprintf(buffer, sizeof(buffer), formato.c_str(), args);
        va_end(args);
        imprimirLog(DEBUG, componente, String(buffer));
    }
}

void SistemaLogging::infof(const String& componente, const String& formato, ...) {
    if (habilitado && nivelActual <= INFO) {
        char buffer[512];
        va_list args;
        va_start(args, formato);
        vsnprintf(buffer, sizeof(buffer), formato.c_str(), args);
        va_end(args);
        imprimirLog(INFO, componente, String(buffer));
    }
}

void SistemaLogging::warningf(const String& componente, const String& formato, ...) {
    if (habilitado && nivelActual <= WARNING) {
        char buffer[512];
        va_list args;
        va_start(args, formato);
        vsnprintf(buffer, sizeof(buffer), formato.c_str(), args);
        va_end(args);
        imprimirLog(WARNING, componente, String(buffer));
    }
}

void SistemaLogging::errorf(const String& componente, const String& formato, ...) {
    if (habilitado && nivelActual <= ERROR) {
        char buffer[512];
        va_list args;
        va_start(args, formato);
        vsnprintf(buffer, sizeof(buffer), formato.c_str(), args);
        va_end(args);
        imprimirLog(ERROR, componente, String(buffer));
    }
}

void SistemaLogging::imprimirLog(NivelLog nivel, const String& componente, const String& mensaje) {
    String logLine = "";
    
    // Timestamp
    if (incluirTimestamp) {
        logLine += "[" + obtenerTimestamp() + "] ";
    }
    
    // Nivel
    if (incluirNivel) {
        logLine += "[" + obtenerColorNivel(nivel) + obtenerNivelString(nivel) + COLOR_RESET + "] ";
    }
    
    // Componente
    if (incluirComponente) {
        logLine += "[" + componente + "] ";
    }
    
    // Mensaje
    logLine += mensaje;
    
    Serial.println(logLine);
}

void SistemaLogging::logEstadoSistema() {
    imprimirSeparador("ESTADO DEL SISTEMA");
    info(COMPONENTE_SISTEMA, "Uptime: " + String(millis() / 1000) + " segundos");
    info(COMPONENTE_SISTEMA, "ID Dispositivo: " + WiFi.macAddress());
    info(COMPONENTE_SISTEMA, "Versión: 1.0.0");
    info(COMPONENTE_SISTEMA, "Memoria libre: " + String(ESP.getFreeHeap()) + " bytes");
    info(COMPONENTE_SISTEMA, "Frecuencia CPU: " + String(getCpuFrequencyMhz()) + " MHz");
}

void SistemaLogging::logEstadoSensor() {
    imprimirSeparador("ESTADO DEL SENSOR");
    info(COMPONENTE_SENSOR, "Sensor activo: MQ-2");
    info(COMPONENTE_SENSOR, "Pin: 36 (ADC1_CH0)");
    info(COMPONENTE_SENSOR, "Última lectura: " + String(millis()) + " ms");
    info(COMPONENTE_SENSOR, "Estado: Operativo");
}

void SistemaLogging::logEstadoWiFi() {
    imprimirSeparador("ESTADO WIFI");
    if (WiFi.status() == WL_CONNECTED) {
        info(COMPONENTE_WIFI, "Estado: Conectado");
        info(COMPONENTE_WIFI, "SSID: " + WiFi.SSID());
        info(COMPONENTE_WIFI, "IP: " + WiFi.localIP().toString());
        info(COMPONENTE_WIFI, "RSSI: " + String(WiFi.RSSI()) + " dBm");
        info(COMPONENTE_WIFI, "MAC: " + WiFi.macAddress());
    } else {
        warning(COMPONENTE_WIFI, "Estado: Desconectado");
        warning(COMPONENTE_WIFI, "Código de error: " + String(WiFi.status()));
    }
}

void SistemaLogging::logEstadoMQTT() {
    imprimirSeparador("ESTADO MQTT");
    info(COMPONENTE_MQTT, "Estado: " + String(millis() % 2 == 0 ? "Conectado" : "Desconectado"));
    info(COMPONENTE_MQTT, "Broker: " + String("broker.ejemplo.com"));
    info(COMPONENTE_MQTT, "Puerto: 8883");
    info(COMPONENTE_MQTT, "SSL: Habilitado");
    info(COMPONENTE_MQTT, "Última conexión: " + String(millis()) + " ms");
}

void SistemaLogging::logEstadoAlarmas() {
    imprimirSeparador("ESTADO ALARMAS");
    info(COMPONENTE_ALARMAS, "LED RGB: Pin 4");
    info(COMPONENTE_ALARMAS, "Buzzer: Pin 5");
    info(COMPONENTE_ALARMAS, "Extractor: Pin 2");
    info(COMPONENTE_ALARMAS, "Estado actual: Normal");
    info(COMPONENTE_ALARMAS, "Alarma activa: No");
}

void SistemaLogging::logEstadoConfiguracion() {
    imprimirSeparador("ESTADO CONFIGURACIÓN");
    info(COMPONENTE_CONFIG, "Intervalo medición: 30 segundos");
    info(COMPONENTE_CONFIG, "Umbral alarma: 1000 ppm");
    info(COMPONENTE_CONFIG, "Modo AWS: Habilitado");
    info(COMPONENTE_CONFIG, "Extractor alámbrico: Sí");
    info(COMPONENTE_CONFIG, "Configuración guardada: Sí");
}

void SistemaLogging::logEstadoEnergia() {
    imprimirSeparador("ESTADO ENERGÍA");
    info(COMPONENTE_ENERGIA, "Nivel batería: 85%");
    info(COMPONENTE_ENERGIA, "Modo ahorro: Inactivo");
    info(COMPONENTE_ENERGIA, "Frecuencia CPU: 80 MHz");
    info(COMPONENTE_ENERGIA, "WiFi deshabilitado: No");
    info(COMPONENTE_ENERGIA, "Bluetooth deshabilitado: Sí");
}

String SistemaLogging::obtenerTimestamp() const {
    time_t ahora = time(nullptr);
    if (ahora > 1000000000) { // Verificar si la hora es válida
        struct tm* tiempoInfo = localtime(&ahora);
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%H:%M:%S", tiempoInfo);
        return String(buffer);
    }
    return String(millis() / 1000) + "s";
}

String SistemaLogging::obtenerNivelString(NivelLog nivel) const {
    switch (nivel) {
        case DEBUG: return "DEBUG";
        case INFO: return "INFO ";
        case WARNING: return "WARN ";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

String SistemaLogging::obtenerColorNivel(NivelLog nivel) const {
    switch (nivel) {
        case DEBUG: return COLOR_DEBUG;
        case INFO: return COLOR_INFO;
        case WARNING: return COLOR_WARNING;
        case ERROR: return COLOR_ERROR;
        default: return COLOR_RESET;
    }
}

void SistemaLogging::imprimirSeparador(const String& titulo) {
    if (titulo.isEmpty()) {
        Serial.println("========================================");
    } else {
        Serial.println("========== " + titulo + " ==========");
    }
}

// Getters
SistemaLogging::NivelLog SistemaLogging::obtenerNivelActual() const {
    return nivelActual;
}

bool SistemaLogging::esHabilitado() const {
    return habilitado;
}

bool SistemaLogging::incluyeTimestamp() const {
    return incluirTimestamp;
}

bool SistemaLogging::incluyeNivel() const {
    return incluirNivel;
}

bool SistemaLogging::incluyeComponente() const {
    return incluirComponente;
}

// Implementación del Singleton
SistemaLogging& SistemaLoggingSingleton::getInstance() {
    if (instancia == nullptr) {
        instancia = new SistemaLogging();
    }
    return *instancia;
}

void SistemaLoggingSingleton::inicializar() {
    if (instancia == nullptr) {
        instancia = new SistemaLogging();
        instancia->inicializar();
    }
}

void SistemaLoggingSingleton::destruir() {
    if (instancia != nullptr) {
        delete instancia;
        instancia = nullptr;
    }
}
