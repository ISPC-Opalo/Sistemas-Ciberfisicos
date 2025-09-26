#include "WiFiManager.h"

WiFiManagerCustom::WiFiManagerCustom() : 
    conectado(false), portalActivo(false), ultimaVerificacion(0), 
    intervaloVerificacion(30000) { // 30 segundos
    
    // Inicializar parámetros personalizados
    intervaloMedicion = nullptr;
    umbralAlarma = nullptr;
    modoAWS = nullptr;
    brokerMQTT = nullptr;
    puertoMQTT = nullptr;
    usarWebSocket = nullptr;
    extractorAlambrico = nullptr;
    pinExtractor = nullptr;
}

WiFiManagerCustom::~WiFiManagerCustom() {
    // Limpiar parámetros
    if (intervaloMedicion) delete intervaloMedicion;
    if (umbralAlarma) delete umbralAlarma;
    if (modoAWS) delete modoAWS;
    if (brokerMQTT) delete brokerMQTT;
    if (puertoMQTT) delete puertoMQTT;
    if (usarWebSocket) delete usarWebSocket;
    if (extractorAlambrico) delete extractorAlambrico;
    if (pinExtractor) delete pinExtractor;
}

bool WiFiManagerCustom::inicializar() {
    // Configurar WiFiManager
    wm.setConfigPortalTimeout(300); // 5 minutos
    wm.setConnectTimeout(20); // 20 segundos
    wm.setSaveConfigCallback(configuracionGuardada);
    wm.setSaveConfigCallback(configuracionTimeout);
    wm.setAPCallback(configuracionIniciada);
    
    // Configurar parámetros personalizados
    configurarParametrosPersonalizados();
    
    // Cargar parámetros guardados
    cargarParametrosGuardados();
    
    Serial.println("WiFiManager inicializado");
    return true;
}

bool WiFiManagerCustom::conectar() {
    // Intentar conectar con configuración guardada
    if (wm.autoConnect("GASLYT-Config")) {
        conectado = true;
        ssidAnterior = WiFi.SSID();
        passwordAnterior = WiFi.psk();
        
        Serial.println("Conectado a WiFi exitosamente");
        Serial.println("SSID: " + WiFi.SSID());
        Serial.println("IP: " + WiFi.localIP().toString());
        Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
        
        // Sincronizar hora
        sincronizarHora();
        
        return true;
    } else {
        Serial.println("No se pudo conectar a WiFi");
        return false;
    }
}

void WiFiManagerCustom::desconectar() {
    WiFi.disconnect();
    conectado = false;
    Serial.println("Desconectado de WiFi");
}

bool WiFiManagerCustom::verificarConexion() {
    unsigned long tiempoActual = millis();
    
    if (tiempoActual - ultimaVerificacion >= intervaloVerificacion) {
        ultimaVerificacion = tiempoActual;
        
        if (WiFi.status() == WL_CONNECTED) {
            if (!conectado) {
                conectado = true;
                Serial.println("WiFi reconectado");
                sincronizarHora();
            }
            return true;
        } else {
            if (conectado) {
                conectado = false;
                Serial.println("WiFi desconectado");
            }
            return false;
        }
    }
    
    return conectado;
}

void WiFiManagerCustom::iniciarPortalCautivo() {
    if (portalActivo) {
        return;
    }
    
    Serial.println("Iniciando portal cautivo...");
    portalActivo = true;
    
    // Configurar parámetros personalizados
    configurarParametrosPersonalizados();
    
    // Iniciar portal
    if (!wm.startConfigPortal("GASLYT-Config")) {
        Serial.println("Error al iniciar portal cautivo");
        portalActivo = false;
        return;
    }
    
    // Guardar configuración
    guardarParametrosPersonalizados();
    
    portalActivo = false;
    Serial.println("Portal cautivo finalizado");
}

void WiFiManagerCustom::configurarParametrosPersonalizados() {
    // Limpiar parámetros existentes
    if (intervaloMedicion) delete intervaloMedicion;
    if (umbralAlarma) delete umbralAlarma;
    if (modoAWS) delete modoAWS;
    if (brokerMQTT) delete brokerMQTT;
    if (puertoMQTT) delete puertoMQTT;
    if (usarWebSocket) delete usarWebSocket;
    if (extractorAlambrico) delete extractorAlambrico;
    if (pinExtractor) delete pinExtractor;
    
    // Crear nuevos parámetros
    intervaloMedicion = new WiFiManagerParameter("intervalo", "Intervalo de medición (10-60 segundos)", "30", 3);
    umbralAlarma = new WiFiManagerParameter("umbral", "Umbral de alarma (ppm)", "1000", 10);
    modoAWS = new WiFiManagerParameter("modoAWS", "Modo AWS (1=Si, 0=No)", "1", 1);
    brokerMQTT = new WiFiManagerParameter("broker", "Broker MQTT", "", 100);
    puertoMQTT = new WiFiManagerParameter("puerto", "Puerto MQTT", "8883", 5);
    usarWebSocket = new WiFiManagerParameter("websocket", "Usar WebSocket (1=Si, 0=No)", "0", 1);
    extractorAlambrico = new WiFiManagerParameter("extractor", "Extractor alámbrico (1=Si, 0=No)", "1", 1);
    pinExtractor = new WiFiManagerParameter("pinExtractor", "Pin del extractor", "2", 2);
    
    // Agregar parámetros al WiFiManager
    wm.addParameter(intervaloMedicion);
    wm.addParameter(umbralAlarma);
    wm.addParameter(modoAWS);
    wm.addParameter(brokerMQTT);
    wm.addParameter(puertoMQTT);
    wm.addParameter(usarWebSocket);
    wm.addParameter(extractorAlambrico);
    wm.addParameter(pinExtractor);
}

void WiFiManagerCustom::guardarParametrosPersonalizados() {
    // Guardar en Preferences
    Preferences preferences;
    preferences.begin("gaslyt", false);
    
    if (intervaloMedicion) {
        preferences.putInt("intervaloMed", atoi(intervaloMedicion->getValue()));
    }
    if (umbralAlarma) {
        preferences.putFloat("umbralAlarma", atof(umbralAlarma->getValue()));
    }
    if (modoAWS) {
        preferences.putBool("modoAWS", atoi(modoAWS->getValue()) == 1);
    }
    if (brokerMQTT) {
        preferences.putString("brokerMQTT", brokerMQTT->getValue());
    }
    if (puertoMQTT) {
        preferences.putInt("puertoMQTT", atoi(puertoMQTT->getValue()));
    }
    if (usarWebSocket) {
        preferences.putBool("usarWebSocket", atoi(usarWebSocket->getValue()) == 1);
    }
    if (extractorAlambrico) {
        preferences.putBool("extractorAlambrico", atoi(extractorAlambrico->getValue()) == 1);
    }
    if (pinExtractor) {
        preferences.putInt("pinExtractor", atoi(pinExtractor->getValue()));
    }
    
    preferences.end();
    Serial.println("Parámetros personalizados guardados");
}

void WiFiManagerCustom::cargarParametrosGuardados() {
    Preferences preferences;
    preferences.begin("gaslyt", false);
    
    // Cargar valores guardados
    int intervalo = preferences.getInt("intervaloMed", 30);
    float umbral = preferences.getFloat("umbralAlarma", 1000.0);
    bool aws = preferences.getBool("modoAWS", true);
    String broker = preferences.getString("brokerMQTT", "");
    int puerto = preferences.getInt("puertoMQTT", 8883);
    bool ws = preferences.getBool("usarWebSocket", false);
    bool extractor = preferences.getBool("extractorAlambrico", true);
    int pin = preferences.getInt("pinExtractor", 2);
    
    preferences.end();
    
    // Establecer valores en parámetros
    if (intervaloMedicion) {
        intervaloMedicion->setValue(String(intervalo).c_str(), 3);
    }
    if (umbralAlarma) {
        umbralAlarma->setValue(String(umbral).c_str(), 10);
    }
    if (modoAWS) {
        modoAWS->setValue(aws ? "1" : "0", 1);
    }
    if (brokerMQTT) {
        brokerMQTT->setValue(broker.c_str(), 100);
    }
    if (puertoMQTT) {
        puertoMQTT->setValue(String(puerto).c_str(), 5);
    }
    if (usarWebSocket) {
        usarWebSocket->setValue(ws ? "1" : "0", 1);
    }
    if (extractorAlambrico) {
        extractorAlambrico->setValue(extractor ? "1" : "0", 1);
    }
    if (pinExtractor) {
        pinExtractor->setValue(String(pin).c_str(), 2);
    }
}

// Getters
bool WiFiManagerCustom::estaConectado() const {
    return conectado;
}

bool WiFiManagerCustom::esPortalActivo() const {
    return portalActivo;
}

String WiFiManagerCustom::obtenerSSID() const {
    return WiFi.SSID();
}

String WiFiManagerCustom::obtenerIP() const {
    return WiFi.localIP().toString();
}

int WiFiManagerCustom::obtenerRSSI() const {
    return WiFi.RSSI();
}

// NTP
bool WiFiManagerCustom::sincronizarHora() {
    configTime(zonaHoraria, diasHorarioVerano, servidorNTP);
    
    Serial.println("Sincronizando hora con servidor NTP...");
    
    // Esperar hasta 10 segundos para sincronización
    for (int i = 0; i < 10; i++) {
        if (time(nullptr) > 1000000000) { // Verificar si la hora es válida
            Serial.println("Hora sincronizada exitosamente");
            Serial.println("Hora actual: " + obtenerHoraActual());
            return true;
        }
        delay(1000);
    }
    
    Serial.println("Error al sincronizar hora");
    return false;
}

String WiFiManagerCustom::obtenerHoraActual() const {
    time_t ahora = time(nullptr);
    struct tm* tiempoInfo = localtime(&ahora);
    
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tiempoInfo);
    
    return String(buffer);
}

unsigned long WiFiManagerCustom::obtenerTimestamp() const {
    return time(nullptr);
}

// Utilidades
void WiFiManagerCustom::imprimirEstado() const {
    Serial.println("=== ESTADO WIFI ===");
    Serial.println("Conectado: " + String(conectado ? "Sí" : "No"));
    Serial.println("SSID: " + obtenerSSID());
    Serial.println("IP: " + obtenerIP());
    Serial.println("RSSI: " + String(obtenerRSSI()) + " dBm");
    Serial.println("Portal activo: " + String(portalActivo ? "Sí" : "No"));
    Serial.println("Hora: " + obtenerHoraActual());
    Serial.println("==================");
}

void WiFiManagerCustom::reiniciar() {
    Serial.println("Reiniciando WiFi...");
    WiFi.disconnect();
    delay(1000);
    conectar();
}

void WiFiManagerCustom::resetearConfiguracion() {
    wm.resetSettings();
    Serial.println("Configuración WiFi reseteada");
}

// Callbacks estáticos
void WiFiManagerCustom::configuracionGuardada() {
    Serial.println("Configuración guardada");
}

void WiFiManagerCustom::configuracionTimeout() {
    Serial.println("Timeout de configuración");
}

void WiFiManagerCustom::configuracionIniciada() {
    Serial.println("Portal de configuración iniciado");
}
