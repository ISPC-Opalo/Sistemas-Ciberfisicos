#ifndef GASSENSOR_H
#define GASSENSOR_H

#include <Arduino.h>
#include <MQUnifiedsensor.h>

class GasSensor {
private:
    MQUnifiedsensor* sensor;
    int pinSensor;
    String tipoSensor;
    float umbralAlarma;
    float ultimaLectura;
    unsigned long ultimaMedicion;
    bool alarmaActiva;
    
    // Configuración del sensor
    struct ConfiguracionSensor {
        String tipo;
        float voltaje;
        int pin;
        String unidad;
        float ratioAireLimpio;
    } config;
    
public:
    GasSensor(int pin, const String& tipo = "MQ-2");
    ~GasSensor();
    
    // Métodos principales
    bool inicializar();
    float leerConcentracion();
    bool verificarUmbral();
    void calibrar();
    
    // Configuración
    void establecerUmbral(float umbral);
    void establecerTipoSensor(const String& tipo);
    void establecerRatioAireLimpio(float ratio);
    
    // Estado
    float obtenerUltimaLectura() const;
    bool esAlarmaActiva() const;
    unsigned long obtenerTiempoUltimaMedicion() const;
    String obtenerTipoSensor() const;
    
    // Utilidades
    void imprimirLectura() const;
    bool esLecturaValida(float lectura) const;
    float convertirUnidades(float lectura, const String& unidadDestino) const;
};

#endif
