#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>
#include <WiFiManager.h>
#include <time.h>
#include <ArduinoJson.h>

class WiFiManagerCustom {
private:
    WiFiManager wm;
    bool conectado;
    bool portalActivo;
    unsigned long ultimaVerificacion;
    unsigned long intervaloVerificacion;
    String ssidAnterior;
    String passwordAnterior;
    
    // Configuración de parámetros personalizados
    WiFiManagerParameter* intervaloMedicion;
    WiFiManagerParameter* umbralAlarma;
    WiFiManagerParameter* modoAWS;
    WiFiManagerParameter* brokerMQTT;
    WiFiManagerParameter* puertoMQTT;
    WiFiManagerParameter* usarWebSocket;
    WiFiManagerParameter* extractorAlambrico;
    WiFiManagerParameter* pinExtractor;
    
    // Configuración NTP
    const char* servidorNTP = "pool.ntp.org";
    const long zonaHoraria = -3 * 3600; // GMT-3 (Argentina)
    const int diasHorarioVerano = 0;
    
public:
    WiFiManagerCustom();
    ~WiFiManagerCustom();
    
    // Métodos principales
    bool inicializar();
    bool conectar();
    void desconectar();
    bool verificarConexion();
    void iniciarPortalCautivo();
    
    // Configuración
    void configurarParametrosPersonalizados();
    void guardarParametrosPersonalizados();
    void cargarParametrosGuardados();
    
    // Estado
    bool estaConectado() const;
    bool esPortalActivo() const;
    String obtenerSSID() const;
    String obtenerIP() const;
    int obtenerRSSI() const;
    
    // NTP
    bool sincronizarHora();
    String obtenerHoraActual() const;
    unsigned long obtenerTimestamp() const;
    
    // Utilidades
    void imprimirEstado() const;
    void reiniciar();
    void resetearConfiguracion();
    
    // Callbacks
    static void configuracionGuardada();
    static void configuracionTimeout();
    static void configuracionIniciada();
};

#endif
