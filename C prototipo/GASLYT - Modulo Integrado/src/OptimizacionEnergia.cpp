#include "OptimizacionEnergia.h"

OptimizacionEnergia::OptimizacionEnergia() : 
    modoAhorroEnergia(false), ultimaActividad(0), tiempoInactividad(0),
    nivelBateria(100), wifiDeshabilitado(false), bluetoothDeshabilitado(false) {
    
    // Configuración por defecto
    configuracion.habilitarModoSueño = true;
    configuracion.tiempoInactividadMax = 300000; // 5 minutos
    configuracion.umbralBateriaBajo = 20; // 20%
    configuracion.deshabilitarWiFiInactivo = true;
    configuracion.deshabilitarBluetooth = true;
    configuracion.frecuenciaCPU = 80; // 80 MHz
}

OptimizacionEnergia::~OptimizacionEnergia() {
    // Restaurar configuraciones por defecto
    habilitarWiFi();
    habilitarBluetooth();
    ajustarFrecuenciaCPU(240); // Frecuencia normal
}

bool OptimizacionEnergia::inicializar() {
    ultimaActividad = millis();
    
    // Configurar frecuencia de CPU inicial
    ajustarFrecuenciaCPU(configuracion.frecuenciaCPU);
    
    // Deshabilitar Bluetooth si está configurado
    if (configuracion.deshabilitarBluetooth) {
        deshabilitarBluetooth();
    }
    
    Serial.println("Optimización de energía inicializada");
    Serial.println("Frecuencia CPU: " + String(configuracion.frecuenciaCPU) + " MHz");
    Serial.println("Modo ahorro energía: " + String(modoAhorroEnergia ? "Habilitado" : "Deshabilitado"));
    
    return true;
}

void OptimizacionEnergia::actualizarActividad() {
    ultimaActividad = millis();
    tiempoInactividad = 0;
    
    // Si estaba en modo ahorro, salir
    if (modoAhorroEnergia) {
        salirModoSueño();
    }
}

void OptimizacionEnergia::verificarEstadoBateria() {
    // Simular lectura de batería (en implementación real usar ADC)
    // Por ahora usar un valor simulado
    nivelBateria = 85; // 85% simulado
    
    if (nivelBateria <= configuracion.umbralBateriaBajo) {
        Serial.println("¡Batería baja! Nivel: " + String(nivelBateria) + "%");
        optimizarParaBateria();
    }
}

void OptimizacionEnergia::aplicarOptimizaciones() {
    unsigned long tiempoActual = millis();
    tiempoInactividad = tiempoActual - ultimaActividad;
    
    // Verificar si debe entrar en modo ahorro
    if (tiempoInactividad >= configuracion.tiempoInactividadMax && 
        configuracion.habilitarModoSueño && !modoAhorroEnergia) {
        
        Serial.println("Entrando en modo ahorro de energía por inactividad");
        habilitarModoAhorroEnergia(true);
    }
    
    // Verificar estado de batería
    verificarEstadoBateria();
}

void OptimizacionEnergia::habilitarModoAhorroEnergia(bool habilitar) {
    modoAhorroEnergia = habilitar;
    
    if (habilitar) {
        Serial.println("Activando modo ahorro de energía");
        
        // Reducir frecuencia de CPU
        ajustarFrecuenciaCPU(40); // 40 MHz para ahorro
        
        // Deshabilitar WiFi si está configurado
        if (configuracion.deshabilitarWiFiInactivo) {
            deshabilitarWiFi();
        }
        
        // Deshabilitar Bluetooth
        if (configuracion.deshabilitarBluetooth) {
            deshabilitarBluetooth();
        }
        
    } else {
        Serial.println("Desactivando modo ahorro de energía");
        
        // Restaurar frecuencia de CPU
        ajustarFrecuenciaCPU(80); // 80 MHz normal
        
        // Habilitar WiFi
        habilitarWiFi();
        
        // Bluetooth se mantiene deshabilitado por defecto
    }
}

void OptimizacionEnergia::establecerTiempoInactividad(unsigned long tiempo) {
    configuracion.tiempoInactividadMax = tiempo;
    Serial.println("Tiempo de inactividad establecido en: " + String(tiempo) + " ms");
}

void OptimizacionEnergia::establecerUmbralBateriaBajo(int umbral) {
    configuracion.umbralBateriaBajo = umbral;
    Serial.println("Umbral de batería baja establecido en: " + String(umbral) + "%");
}

void OptimizacionEnergia::deshabilitarWiFi() {
    if (!wifiDeshabilitado) {
        esp_wifi_stop();
        wifiDeshabilitado = true;
        Serial.println("WiFi deshabilitado para ahorrar energía");
    }
}

void OptimizacionEnergia::habilitarWiFi() {
    if (wifiDeshabilitado) {
        esp_wifi_start();
        wifiDeshabilitado = false;
        Serial.println("WiFi habilitado");
    }
}

void OptimizacionEnergia::deshabilitarBluetooth() {
    if (!bluetoothDeshabilitado) {
        esp_bt_controller_disable();
        bluetoothDeshabilitado = true;
        Serial.println("Bluetooth deshabilitado para ahorrar energía");
    }
}

void OptimizacionEnergia::habilitarBluetooth() {
    if (bluetoothDeshabilitado) {
        esp_bt_controller_enable(ESP_BT_MODE_BLE);
        bluetoothDeshabilitado = false;
        Serial.println("Bluetooth habilitado");
    }
}

void OptimizacionEnergia::ajustarFrecuenciaCPU(int frecuencia) {
    // Configurar frecuencia de CPU
    if (frecuencia == 40) {
        setCpuFrequencyMhz(40);
    } else if (frecuencia == 80) {
        setCpuFrequencyMhz(80);
    } else if (frecuencia == 160) {
        setCpuFrequencyMhz(160);
    } else if (frecuencia == 240) {
        setCpuFrequencyMhz(240);
    }
    
    configuracion.frecuenciaCPU = frecuencia;
    Serial.println("Frecuencia de CPU ajustada a: " + String(frecuencia) + " MHz");
}

void OptimizacionEnergia::entrarModoSueño() {
    Serial.println("Entrando en modo sueño profundo");
    
    // Configurar despertador
    esp_sleep_enable_timer_wakeup(1000000); // 1 segundo
    
    // Entrar en modo sueño
    esp_deep_sleep_start();
}

void OptimizacionEnergia::salirModoSueño() {
    Serial.println("Saliendo del modo sueño");
    modoAhorroEnergia = false;
    ultimaActividad = millis();
}

void OptimizacionEnergia::configurarDespertador(unsigned long tiempo) {
    esp_sleep_enable_timer_wakeup(tiempo * 1000); // Convertir a microsegundos
    Serial.println("Despertador configurado para: " + String(tiempo) + " segundos");
}

// Getters
bool OptimizacionEnergia::esModoAhorroEnergia() const {
    return modoAhorroEnergia;
}

int OptimizacionEnergia::obtenerNivelBateria() const {
    return nivelBateria;
}

bool OptimizacionEnergia::esBateriaBaja() const {
    return nivelBateria <= configuracion.umbralBateriaBajo;
}

unsigned long OptimizacionEnergia::obtenerTiempoInactividad() const {
    return tiempoInactividad;
}

// Utilidades
void OptimizacionEnergia::imprimirEstado() const {
    Serial.println("=== ESTADO OPTIMIZACIÓN ENERGÍA ===");
    Serial.println("Modo ahorro energía: " + String(modoAhorroEnergia ? "Activo" : "Inactivo"));
    Serial.println("Nivel batería: " + String(nivelBateria) + "%");
    Serial.println("Batería baja: " + String(esBateriaBaja() ? "Sí" : "No"));
    Serial.println("Tiempo inactividad: " + String(tiempoInactividad) + " ms");
    Serial.println("Frecuencia CPU: " + String(configuracion.frecuenciaCPU) + " MHz");
    Serial.println("WiFi deshabilitado: " + String(wifiDeshabilitado ? "Sí" : "No"));
    Serial.println("Bluetooth deshabilitado: " + String(bluetoothDeshabilitado ? "Sí" : "No"));
    Serial.println("Tiempo inactividad max: " + String(configuracion.tiempoInactividadMax) + " ms");
    Serial.println("Umbral batería baja: " + String(configuracion.umbralBateriaBajo) + "%");
    Serial.println("===================================");
}

void OptimizacionEnergia::resetear() {
    modoAhorroEnergia = false;
    ultimaActividad = millis();
    tiempoInactividad = 0;
    
    // Restaurar configuraciones
    habilitarWiFi();
    habilitarBluetooth();
    ajustarFrecuenciaCPU(80);
    
    Serial.println("Optimización de energía reseteada");
}

void OptimizacionEnergia::optimizarParaBateria() {
    Serial.println("Aplicando optimizaciones para batería baja");
    
    // Reducir frecuencia de CPU
    ajustarFrecuenciaCPU(40);
    
    // Deshabilitar WiFi
    deshabilitarWiFi();
    
    // Deshabilitar Bluetooth
    deshabilitarBluetooth();
    
    // Reducir tiempo de inactividad
    configuracion.tiempoInactividadMax = 60000; // 1 minuto
    
    Serial.println("Optimizaciones para batería baja aplicadas");
}
