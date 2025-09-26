#include "MQTTManager.h"

MQTTManager::MQTTManager() : 
    clienteMQTT(nullptr), clienteSeguro(nullptr), clienteNormal(nullptr),
    idDispositivo(""), broker(""), puerto(8883), usarSSL(true), 
    usarWebSocket(false), conectado(false), callbackMensaje(nullptr), 
    callbackConfiguracion(nullptr), callbackActualizaciones(nullptr) {
    
    // Inicializar clientes
    clienteSeguro = new WiFiClientSecure();
    clienteNormal = new WiFiClient();
    
    // Configurar cliente seguro
    clienteSeguro->setInsecure(); // Para desarrollo, en producción usar certificados
}

MQTTManager::~MQTTManager() {
    if (clienteMQTT) {
        delete clienteMQTT;
    }
    if (clienteSeguro) {
        delete clienteSeguro;
    }
    if (clienteNormal) {
        delete clienteNormal;
    }
}

bool MQTTManager::inicializar() {
    // Configurar cliente MQTT
    if (usarSSL) {
        clienteMQTT = new PubSubClient(*clienteSeguro);
    } else {
        clienteMQTT = new PubSubClient(*clienteNormal);
    }
    
    if (!clienteMQTT) {
        Serial.println("Error al inicializar cliente MQTT");
        return false;
    }
    
    // Configurar callback
    clienteMQTT->setCallback(callbackMensajeRecibido);
    
    // Configurar topics
    topicLecturas = "/" + idDispositivo + "/lecturas";
    topicAlarmas = "/" + idDispositivo + "/alarmas";
    topicMetadata = "/" + idDispositivo + "/metadata";
    topicConfiguracion = "/" + idDispositivo + "/configuracion";
    
    Serial.println("MQTTManager inicializado");
    Serial.println("ID Dispositivo: " + idDispositivo);
    Serial.println("Broker: " + broker);
    Serial.println("Puerto: " + String(puerto));
    Serial.println("SSL: " + String(usarSSL ? "Sí" : "No"));
    Serial.println("WebSocket: " + String(usarWebSocket ? "Sí" : "No"));
    
    return true;
}

bool MQTTManager::conectar() {
    if (!clienteMQTT) {
        Serial.println("Error: Cliente MQTT no inicializado");
        return false;
    }
    
    // Configurar servidor
    clienteMQTT->setServer(broker.c_str(), puerto);
    
    // Intentar conectar
    int intentos = 0;
    while (!clienteMQTT->connected() && intentos < 5) {
        Serial.println("Intentando conectar a MQTT... Intento " + String(intentos + 1));
        
        if (clienteMQTT->connect(idDispositivo.c_str())) {
            conectado = true;
            Serial.println("Conectado a MQTT exitosamente");
            
            // Suscribirse a topics
            if (clienteMQTT->subscribe(topicConfiguracion.c_str())) {
                Serial.println("Suscrito a topic de configuración: " + topicConfiguracion);
            } else {
                Serial.println("Error al suscribirse a topic de configuración");
            }
            
            // Suscribirse a topic de actualizaciones
            String topicActualizaciones = "/" + idDispositivo + "/actualizaciones";
            if (clienteMQTT->subscribe(topicActualizaciones.c_str())) {
                Serial.println("Suscrito a topic de actualizaciones: " + topicActualizaciones);
            } else {
                Serial.println("Error al suscribirse a topic de actualizaciones");
            }
            
            return true;
        } else {
            Serial.println("Error de conexión MQTT: " + String(clienteMQTT->state()));
            delay(2000);
            intentos++;
        }
    }
    
    Serial.println("No se pudo conectar a MQTT después de 5 intentos");
    conectado = false;
    return false;
}

void MQTTManager::desconectar() {
    if (clienteMQTT && clienteMQTT->connected()) {
        clienteMQTT->disconnect();
        conectado = false;
        Serial.println("Desconectado de MQTT");
    }
}

bool MQTTManager::verificarConexion() {
    if (!clienteMQTT) {
        return false;
    }
    
    bool estado = clienteMQTT->connected();
    if (estado != conectado) {
        conectado = estado;
        if (conectado) {
            Serial.println("MQTT reconectado");
        } else {
            Serial.println("MQTT desconectado");
        }
    }
    
    return conectado;
}

void MQTTManager::procesarMensajes() {
    if (clienteMQTT && clienteMQTT->connected()) {
        clienteMQTT->loop();
    }
}

void MQTTManager::configurarAWS(const String& endpoint, const String& certificado, 
                               const String& clavePrivada, const String& certificadoCA) {
    endpointAWS = endpoint;
    certificadoAWS = certificado;
    clavePrivadaAWS = clavePrivada;
    certificadoCA = certificadoCA;
    
    // Configurar certificados
    if (clienteSeguro) {
        clienteSeguro->setCACert(certificadoCA.c_str());
        clienteSeguro->setCertificate(certificado.c_str());
        clienteSeguro->setPrivateKey(clavePrivada.c_str());
    }
    
    // Configurar broker
    broker = endpoint;
    puerto = 8883;
    usarSSL = true;
    usarWebSocket = false;
    
    Serial.println("Configuración AWS IoT Core establecida");
    Serial.println("Endpoint: " + endpoint);
}

void MQTTManager::configurarDEV(const String& brokerDev, int puertoDev, 
                               const String& usuario, const String& password) {
    brokerDEV = brokerDev;
    puertoDEV = puertoDev;
    usuarioDEV = usuario;
    passwordDEV = password;
    
    // Configurar broker
    broker = brokerDev;
    puerto = puertoDev;
    usarSSL = (puertoDev == 8883);
    usarWebSocket = false;
    
    Serial.println("Configuración modo DEV establecida");
    Serial.println("Broker: " + brokerDev);
    Serial.println("Puerto: " + String(puertoDev));
}

void MQTTManager::establecerModoSSL(bool usar) {
    usarSSL = usar;
    if (usar) {
        puerto = 8883;
    } else {
        puerto = 1883;
    }
}

void MQTTManager::establecerModoWebSocket(bool usar) {
    usarWebSocket = usar;
    // Implementar lógica de WebSocket si es necesario
}

bool MQTTManager::publicarLectura(const JsonObject& datos) {
    if (!clienteMQTT || !clienteMQTT->connected()) {
        return false;
    }
    
    String payload;
    serializeJson(datos, payload);
    
    bool resultado = clienteMQTT->publish(topicLecturas.c_str(), payload.c_str(), false);
    
    if (resultado) {
        Serial.println("Lectura publicada exitosamente");
        Serial.println("Topic: " + topicLecturas);
        Serial.println("Payload: " + payload);
    } else {
        Serial.println("Error al publicar lectura");
    }
    
    return resultado;
}

bool MQTTManager::publicarAlarma(const JsonObject& datos) {
    if (!clienteMQTT || !clienteMQTT->connected()) {
        return false;
    }
    
    String payload;
    serializeJson(datos, payload);
    
    bool resultado = clienteMQTT->publish(topicAlarmas.c_str(), payload.c_str(), true); // QoS 2
    
    if (resultado) {
        Serial.println("Alarma publicada exitosamente");
        Serial.println("Topic: " + topicAlarmas);
        Serial.println("Payload: " + payload);
    } else {
        Serial.println("Error al publicar alarma");
    }
    
    return resultado;
}

bool MQTTManager::publicarMetadata(const JsonObject& datos) {
    if (!clienteMQTT || !clienteMQTT->connected()) {
        return false;
    }
    
    String payload;
    serializeJson(datos, payload);
    
    bool resultado = clienteMQTT->publish(topicMetadata.c_str(), payload.c_str(), true); // QoS 2
    
    if (resultado) {
        Serial.println("Metadata publicada exitosamente");
        Serial.println("Topic: " + topicMetadata);
        Serial.println("Payload: " + payload);
    } else {
        Serial.println("Error al publicar metadata");
    }
    
    return resultado;
}

void MQTTManager::establecerIdDispositivo(const String& id) {
    idDispositivo = id;
    
    // Actualizar topics
    topicLecturas = "/" + idDispositivo + "/lecturas";
    topicAlarmas = "/" + idDispositivo + "/alarmas";
    topicMetadata = "/" + idDispositivo + "/metadata";
    topicConfiguracion = "/" + idDispositivo + "/configuracion";
    
    Serial.println("ID del dispositivo establecido: " + id);
}

void MQTTManager::establecerCallback(void (*callback)(String, String)) {
    callbackMensaje = callback;
}

void MQTTManager::establecerCallbackConfiguracion(void (*callback)(String)) {
    callbackConfiguracion = callback;
}

void MQTTManager::establecerCallbackActualizaciones(void (*callback)(String)) {
    callbackActualizaciones = callback;
}

// Getters
bool MQTTManager::estaConectado() const {
    return conectado;
}

String MQTTManager::obtenerIdDispositivo() const {
    return idDispositivo;
}

String MQTTManager::obtenerBroker() const {
    return broker;
}

int MQTTManager::obtenerPuerto() const {
    return puerto;
}

// Utilidades
void MQTTManager::imprimirEstado() const {
    Serial.println("=== ESTADO MQTT ===");
    Serial.println("Conectado: " + String(conectado ? "Sí" : "No"));
    Serial.println("ID Dispositivo: " + idDispositivo);
    Serial.println("Broker: " + broker);
    Serial.println("Puerto: " + String(puerto));
    Serial.println("SSL: " + String(usarSSL ? "Sí" : "No"));
    Serial.println("WebSocket: " + String(usarWebSocket ? "Sí" : "No"));
    Serial.println("Topic Lecturas: " + topicLecturas);
    Serial.println("Topic Alarmas: " + topicAlarmas);
    Serial.println("Topic Metadata: " + topicMetadata);
    Serial.println("Topic Configuración: " + topicConfiguracion);
    Serial.println("==================");
}

void MQTTManager::reconectar() {
    if (clienteMQTT && !clienteMQTT->connected()) {
        Serial.println("Intentando reconectar MQTT...");
        conectar();
    }
}

bool MQTTManager::esConfiguracionValida() const {
    return !idDispositivo.isEmpty() && !broker.isEmpty() && puerto > 0;
}

// Callback estático
void MQTTManager::callbackMensajeRecibido(char* topic, byte* payload, unsigned int length) {
    String topicStr = String(topic);
    String payloadStr = "";
    
    for (int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }
    
    Serial.println("Mensaje MQTT recibido:");
    Serial.println("Topic: " + topicStr);
    Serial.println("Payload: " + payloadStr);
    
    // Procesar mensaje según el topic
    if (topicStr.endsWith("/configuracion")) {
        // Procesar configuración
        Serial.println("Configuración recibida: " + payloadStr);
        // Aquí se llamaría al callback de configuración
    } else if (topicStr.endsWith("/actualizaciones")) {
        // Procesar actualizaciones
        Serial.println("Actualización recibida: " + payloadStr);
        // Aquí se llamaría al callback de actualizaciones
    } else if (topicStr.endsWith("/comandos")) {
        // Procesar comandos
        Serial.println("Comando recibido: " + payloadStr);
    }
}

void MQTTManager::procesarMensajeConfiguracion(const String& payload) {
    if (callbackConfiguracion) {
        callbackConfiguracion(payload);
    }
}

void MQTTManager::procesarMensajeActualizaciones(const String& payload) {
    if (callbackActualizaciones) {
        callbackActualizaciones(payload);
    }
}
