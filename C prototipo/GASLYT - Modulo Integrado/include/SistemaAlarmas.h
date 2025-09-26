#ifndef SISTEMAALARMAS_H
#define SISTEMAALARMAS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class SistemaAlarmas {
private:
    Adafruit_NeoPixel* ledRGB;
    int pinBuzzer;
    int pinExtractor;
    bool extractorAlambrico;
    
    // Estados del sistema
    enum EstadoSistema {
        NORMAL,
        ADVERTENCIA,
        ALARMA,
        SIN_WIFI,
        ERROR_SENSOR
    } estadoActual;
    
    // Configuración de colores
    struct ColorRGB {
        uint8_t rojo;
        uint8_t verde;
        uint8_t azul;
    };
    
    ColorRGB colorNormal = {0, 255, 0};      // Verde
    ColorRGB colorAdvertencia = {255, 255, 0}; // Amarillo
    ColorRGB colorAlarma = {255, 0, 0};      // Rojo
    ColorRGB colorSinWifi = {0, 0, 255};     // Azul
    ColorRGB colorError = {255, 0, 255};     // Magenta
    
    // Configuración de sonidos
    struct SonidoAlarma {
        int frecuencia;
        int duracion;
        int repeticiones;
        int pausaEntreRepeticiones;
    };
    
    SonidoAlarma sonidoNormal = {1000, 100, 1, 0};
    SonidoAlarma sonidoAdvertencia = {1500, 200, 2, 100};
    SonidoAlarma sonidoAlarma = {2000, 500, 3, 200};
    SonidoAlarma sonidoSinWifi = {800, 300, 1, 0};
    SonidoAlarma sonidoError = {500, 1000, 1, 0};
    
    unsigned long ultimaAlarma;
    unsigned long intervaloAlarma;
    bool alarmaActiva;
    
public:
    SistemaAlarmas(int pinLED, int pinBuzzer, int pinExtractor = -1, bool extractorAlambrico = false);
    ~SistemaAlarmas();
    
    // Métodos principales
    bool inicializar();
    void actualizarEstado(EstadoSistema estado);
    void procesarAlarmas();
    
    // Control de LED
    void establecerColor(uint8_t rojo, uint8_t verde, uint8_t azul);
    void establecerColor(ColorRGB color);
    void parpadear(int veces, int intervalo);
    void apagarLED();
    
    // Control de buzzer
    void reproducirSonido(SonidoAlarma sonido);
    void tocarBuzzer(int frecuencia, int duracion);
    void apagarBuzzer();
    
    // Control de extractor
    void activarExtractor();
    void desactivarExtractor();
    void toggleExtractor();
    
    // Estados específicos
    void indicarNormal();
    void indicarAdvertencia();
    void indicarAlarma();
    void indicarSinWifi();
    void indicarError();
    
    // Configuración
    void establecerIntervaloAlarma(unsigned long intervalo);
    void establecerExtractorAlambrico(bool alambrico);
    void establecerPinExtractor(int pin);
    
    // Estado
    EstadoSistema obtenerEstadoActual() const;
    bool esAlarmaActiva() const;
    bool esExtractorActivo() const;
    
    // Utilidades
    void imprimirEstado() const;
    void resetear();
    void apagarTodo();
};

#endif
