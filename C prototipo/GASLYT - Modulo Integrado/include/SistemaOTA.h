#ifndef SISTEMAOTA_H
#define SISTEMAOTA_H

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>

class SistemaOTA {
private:
    enum EstadoOTA {
        OTA_DISPONIBLE,
        OTA_DESCARGANDO,
        OTA_INSTALANDO,
        OTA_COMPLETADO,
        OTA_ERROR,
        OTA_ROLLBACK
    } estadoActual;
    
    struct InfoActualizacion {
        String version;
        String url;
        String hash;
        size_t tamaño;
        String descripcion;
        bool critica;
        unsigned long timestamp;
        String firma;
        int progreso;
    } infoActualizacion;
    
    // Configuración OTA
    String servidorActualizaciones;
    String tokenAutenticacion;
    unsigned long ultimaVerificacion;
    unsigned long intervaloVerificacion;
    bool actualizacionesAutomaticas;
    bool rollbackDisponible;
    
    // Particiones OTA
    const esp_partition_t* particionActual;
    const esp_partition_t* particionOta0;
    const esp_partition_t* particionOta1;
    const esp_partition_t* particionOtaData;
    
    // Callbacks
    void (*callbackProgreso)(int porcentaje);
    void (*callbackEstado)(EstadoOTA estado);
    void (*callbackError)(const String& error);
    
public:
    SistemaOTA();
    ~SistemaOTA();
    
    // Inicialización
    bool inicializar();
    bool verificarParticiones();
    void configurarArduinoOTA();
    
    // Gestión de actualizaciones
    bool verificarActualizacionesDisponibles();
    bool descargarActualizacion(const String& url);
    bool instalarActualizacion();
    bool verificarIntegridadFirmware();
    void reiniciarConNuevaVersion();
    
    // Control de versiones
    String obtenerVersionActual();
    String obtenerVersionDisponible() const;
    bool esActualizacionCritica() const;
    void establecerVersionActual(const String& version);
    bool esNuevaVersion(const String& version) const;
    
    // Gestión de rollback
    bool hacerRollback();
    bool esRollbackDisponible() const;
    void limpiarVersionAnterior();
    bool verificarIntegridadVersionActual();
    
    // Configuración
    void establecerServidorActualizaciones(const String& servidor);
    void establecerTokenAutenticacion(const String& token);
    void establecerIntervaloVerificacion(unsigned long intervalo);
    void habilitarActualizacionesAutomaticas(bool habilitar);
    
    // Callbacks
    void establecerCallbackProgreso(void (*callback)(int));
    void establecerCallbackEstado(void (*callback)(EstadoOTA));
    void establecerCallbackError(void (*callback)(const String&));
    
    // Estado y información
    EstadoOTA obtenerEstadoActual() const;
    int obtenerProgreso() const;
    String obtenerDescripcionActualizacion() const;
    size_t obtenerTamañoActualizacion() const;
    unsigned long obtenerTimestampActualizacion() const;
    
    // Utilidades
    void imprimirEstado() const;
    void imprimirInfoParticiones() const;
    void resetear();
    bool esInicializado() const;
    
    // Verificación de integridad
    bool verificarHashFirmware(const String& hash);
    bool verificarFirmaDigital(const String& datos, const String& firma);
    String calcularHashFirmware();
    
    // Gestión de memoria
    size_t obtenerEspacioDisponible() const;
    bool verificarEspacioSuficiente(size_t tamaño) const;
    void limpiarCache();
    
    // Logging y debugging
    void imprimirInfoActualizacion() const;
    void imprimirHistorialVersiones() const;
    bool verificarConfiguracion() const;
};

#endif
