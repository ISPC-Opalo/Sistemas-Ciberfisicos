#ifndef CONFIGURACIONREMOTA_H
#define CONFIGURACIONREMOTA_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ConfigManager.h"
#include "GasSensor.h"
#include "SistemaAlarmas.h"
#include "SistemaLogging.h"

class ConfiguracionRemota {
private:
    ConfigManager* configManager;
    GasSensor* sensorGas;
    SistemaAlarmas* sistemaAlarmas;
    SistemaLogging* logger;
    
    String topicConfiguracion;
    bool configuracionRecibida;
    unsigned long ultimaConfiguracion;
    
    // Callback para notificar cambios
    void (*callbackConfiguracionCambiada)(const String& parametro, const String& valor);
    
public:
    ConfiguracionRemota();
    ~ConfiguracionRemota();
    
    // Métodos principales
    bool inicializar(ConfigManager* config, GasSensor* sensor, SistemaAlarmas* alarmas, SistemaLogging* log);
    void procesarMensajeConfiguracion(const String& payload);
    void establecerTopicConfiguracion(const String& topic);
    void establecerCallback(void (*callback)(const String&, const String&));
    
    // Procesamiento de configuraciones específicas
    bool procesarIntervaloMedicion(int intervalo);
    bool procesarUmbralAlarma(float umbral);
    bool procesarModoAWS(bool modo);
    bool procesarBrokerMQTT(const String& broker);
    bool procesarPuertoMQTT(int puerto);
    bool procesarExtractorAlambrico(bool alambrico);
    bool procesarPinExtractor(int pin);
    bool procesarNivelLogging(const String& nivel);
    bool procesarConfiguracionCompleta(const JsonObject& config);
    
    // Validación de configuraciones
    bool validarIntervaloMedicion(int intervalo);
    bool validarUmbralAlarma(float umbral);
    bool validarPuertoMQTT(int puerto);
    bool validarPinExtractor(int pin);
    bool validarNivelLogging(const String& nivel);
    
    // Respuesta a configuraciones
    void enviarConfirmacionConfiguracion(const String& parametro, bool exito, const String& mensaje = "");
    void enviarEstadoConfiguracion();
    
    // Utilidades
    String obtenerTopicConfiguracion() const;
    bool esConfiguracionRecibida() const;
    unsigned long obtenerTiempoUltimaConfiguracion() const;
    void imprimirConfiguracionesDisponibles() const;
    
    // Getters
    ConfigManager* obtenerConfigManager() const;
    GasSensor* obtenerSensorGas() const;
    SistemaAlarmas* obtenerSistemaAlarmas() const;
    SistemaLogging* obtenerLogger() const;
};

#endif
