#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Preferences.h>
#include <ArduinoJson.h>

class ConfigManager {
private:
    Preferences preferences;
    bool configuracionCargada;
    
    // Configuraciones del sistema
    struct ConfiguracionSistema {
        String idDispositivo;
        int intervaloMedicion; // 10-60 segundos
        float umbralAlarma;
        bool modoAWS;
        String brokerMQTT;
        int puertoMQTT;
        bool usarWebSocket;
        bool extractorAlambrico;
        int pinExtractor;
        int pinSensorGas;
        int pinLED;
        int pinBuzzer;
    } configuracion;
    
public:
    ConfigManager();
    ~ConfigManager();
    
    // Métodos principales
    bool inicializar();
    bool cargarConfiguracion();
    bool guardarConfiguracion();
    void resetearConfiguracion();
    
    // Getters
    String obtenerIdDispositivo() const;
    int obtenerIntervaloMedicion() const;
    float obtenerUmbralAlarma() const;
    bool esModoAWS() const;
    String obtenerBrokerMQTT() const;
    int obtenerPuertoMQTT() const;
    bool usarWebSocket() const;
    bool esExtractorAlambrico() const;
    int obtenerPinExtractor() const;
    int obtenerPinSensorGas() const;
    int obtenerPinLED() const;
    int obtenerPinBuzzer() const;
    
    // Setters
    void establecerIntervaloMedicion(int intervalo);
    void establecerUmbralAlarma(float umbral);
    void establecerModoAWS(bool modo);
    void establecerBrokerMQTT(const String& broker);
    void establecerPuertoMQTT(int puerto);
    void establecerUsarWebSocket(bool usar);
    void establecerExtractorAlambrico(bool alambrico);
    void establecerPinExtractor(int pin);
    
    // Utilidades
    String generarIdDispositivo();
    bool esConfiguracionValida() const;
    void imprimirConfiguracion() const;
};

#endif
