#ifndef OPTIMIZACIONENERGIA_H
#define OPTIMIZACIONENERGIA_H

#include <Arduino.h>
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <esp_bt.h>

class OptimizacionEnergia {
private:
    bool modoAhorroEnergia;
    unsigned long ultimaActividad;
    unsigned long tiempoInactividad;
    int nivelBateria;
    bool wifiDeshabilitado;
    bool bluetoothDeshabilitado;
    
    // Configuración de ahorro de energía
    struct ConfiguracionAhorro {
        bool habilitarModoSueño;
        unsigned long tiempoInactividadMax;
        int umbralBateriaBajo;
        bool deshabilitarWiFiInactivo;
        bool deshabilitarBluetooth;
        int frecuenciaCPU;
    } configuracion;
    
public:
    OptimizacionEnergia();
    ~OptimizacionEnergia();
    
    // Métodos principales
    bool inicializar();
    void actualizarActividad();
    void verificarEstadoBateria();
    void aplicarOptimizaciones();
    
    // Control de energía
    void habilitarModoAhorroEnergia(bool habilitar);
    void establecerTiempoInactividad(unsigned long tiempo);
    void establecerUmbralBateriaBajo(int umbral);
    
    // Control de componentes
    void deshabilitarWiFi();
    void habilitarWiFi();
    void deshabilitarBluetooth();
    void habilitarBluetooth();
    void ajustarFrecuenciaCPU(int frecuencia);
    
    // Modo sueño
    void entrarModoSueño();
    void salirModoSueño();
    void configurarDespertador(unsigned long tiempo);
    
    // Estado
    bool esModoAhorroEnergia() const;
    int obtenerNivelBateria() const;
    bool esBateriaBaja() const;
    unsigned long obtenerTiempoInactividad() const;
    
    // Utilidades
    void imprimirEstado() const;
    void resetear();
    void optimizarParaBateria();
};

#endif
