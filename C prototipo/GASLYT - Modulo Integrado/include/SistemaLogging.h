#ifndef SISTEMALOGGING_H
#define SISTEMALOGGING_H

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

class SistemaLogging {
private:
    enum NivelLog {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3
    };
    
    NivelLog nivelActual;
    bool habilitado;
    bool incluirTimestamp;
    bool incluirNivel;
    bool incluirComponente;
    
    // Colores para terminal (ANSI)
    static const String COLOR_DEBUG;
    static const String COLOR_INFO;
    static const String COLOR_WARNING;
    static const String COLOR_ERROR;
    static const String COLOR_RESET;
    
    // Componentes del sistema
    static const String COMPONENTE_SISTEMA;
    static const String COMPONENTE_SENSOR;
    static const String COMPONENTE_WIFI;
    static const String COMPONENTE_MQTT;
    static const String COMPONENTE_ALARMAS;
    static const String COMPONENTE_CONFIG;
    static const String COMPONENTE_ENERGIA;
    
public:
    SistemaLogging();
    ~SistemaLogging();
    
    // Métodos principales
    bool inicializar();
    void establecerNivel(NivelLog nivel);
    void habilitarLogging(bool habilitar);
    void configurarFormato(bool timestamp, bool nivel, bool componente);
    
    // Métodos de logging
    void debug(const String& componente, const String& mensaje);
    void info(const String& componente, const String& mensaje);
    void warning(const String& componente, const String& mensaje);
    void error(const String& componente, const String& mensaje);
    
    // Métodos de logging con formato
    void debugf(const String& componente, const String& formato, ...);
    void infof(const String& componente, const String& formato, ...);
    void warningf(const String& componente, const String& formato, ...);
    void errorf(const String& componente, const String& formato, ...);
    
    // Métodos de logging de estado
    void logEstadoSistema();
    void logEstadoSensor();
    void logEstadoWiFi();
    void logEstadoMQTT();
    void logEstadoAlarmas();
    void logEstadoConfiguracion();
    void logEstadoEnergia();
    
    // Utilidades
    String obtenerTimestamp() const;
    String obtenerNivelString(NivelLog nivel) const;
    String obtenerColorNivel(NivelLog nivel) const;
    void imprimirSeparador(const String& titulo = "");
    
    // Getters
    NivelLog obtenerNivelActual() const;
    bool esHabilitado() const;
    bool incluyeTimestamp() const;
    bool incluyeNivel() const;
    bool incluyeComponente() const;
};

// Macros para facilitar el uso
#define LOG_DEBUG(componente, mensaje) SistemaLogging::getInstance().debug(componente, mensaje)
#define LOG_INFO(componente, mensaje) SistemaLogging::getInstance().info(componente, mensaje)
#define LOG_WARNING(componente, mensaje) SistemaLogging::getInstance().warning(componente, mensaje)
#define LOG_ERROR(componente, mensaje) SistemaLogging::getInstance().error(componente, mensaje)

#define LOG_DEBUGF(componente, formato, ...) SistemaLogging::getInstance().debugf(componente, formato, __VA_ARGS__)
#define LOG_INFOF(componente, formato, ...) SistemaLogging::getInstance().infof(componente, formato, __VA_ARGS__)
#define LOG_WARNINGF(componente, formato, ...) SistemaLogging::getInstance().warningf(componente, formato, __VA_ARGS__)
#define LOG_ERRORF(componente, formato, ...) SistemaLogging::getInstance().errorf(componente, formato, __VA_ARGS__)

// Singleton para acceso global
class SistemaLoggingSingleton {
private:
    static SistemaLogging* instancia;
    
public:
    static SistemaLogging& getInstance();
    static void inicializar();
    static void destruir();
};

#endif
