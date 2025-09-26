#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <time.h>

class MQTTManager {
private:
    PubSubClient* clienteMQTT;
    WiFiClientSecure* clienteSeguro;
    WiFiClient* clienteNormal;
    
    String idDispositivo;
    String broker;
    int puerto;
    bool usarSSL;
    bool usarWebSocket;
    bool conectado;
    
    // Configuración AWS IoT Core
    String certificadoAWS;
    String clavePrivadaAWS;
    String certificadoCA;
    String endpointAWS;
    
    // Configuración modo DEV
    String brokerDEV;
    int puertoDEV;
    String usuarioDEV;
    String passwordDEV;
    
    // Topics
    String topicLecturas;
    String topicAlarmas;
    String topicMetadata;
    String topicConfiguracion;
    
    // Callbacks
    void (*callbackMensaje)(String, String);
    void (*callbackConfiguracion)(String);
    void (*callbackActualizaciones)(String);
    
public:
    MQTTManager();
    ~MQTTManager();
    
    // Métodos principales
    bool inicializar();
    bool conectar();
    void desconectar();
    bool verificarConexion();
    void procesarMensajes();
    
    // Configuración
    void configurarAWS(const String& endpoint, const String& certificado, 
                      const String& clavePrivada, const String& certificadoCA);
    void configurarDEV(const String& broker, int puerto, const String& usuario = "", 
                      const String& password = "");
    void establecerModoSSL(bool usar);
    void establecerModoWebSocket(bool usar);
    
    // Publicación
    bool publicarLectura(const JsonObject& datos);
    bool publicarAlarma(const JsonObject& datos);
    bool publicarMetadata(const JsonObject& datos);
    
    // Configuración de topics
    void establecerIdDispositivo(const String& id);
    void establecerCallback(void (*callback)(String, String));
    void establecerCallbackConfiguracion(void (*callback)(String));
    void establecerCallbackActualizaciones(void (*callback)(String));
    
    // Estado
    bool estaConectado() const;
    String obtenerIdDispositivo() const;
    String obtenerBroker() const;
    int obtenerPuerto() const;
    
    // Utilidades
    void imprimirEstado() const;
    void reconectar();
    bool esConfiguracionValida() const;
    
    // Callbacks MQTT
    static void callbackMensajeRecibido(char* topic, byte* payload, unsigned int length);
    void procesarMensajeConfiguracion(const String& payload);
    void procesarMensajeActualizaciones(const String& payload);
};

#endif
