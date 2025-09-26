#ifndef GESTORACTUALIZACIONES_H
#define GESTORACTUALIZACIONES_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "CertificadosManager.h"
#include "SistemaOTA.h"
#include "MQTTManager.h"
#include "SistemaLogging.h"

class GestorActualizaciones {
private:
    CertificadosManager* certificadosManager;
    SistemaOTA* sistemaOTA;
    MQTTManager* mqttManager;
    SistemaLogging* logger;
    
    String servidorActualizaciones;
    String tokenAutenticacion;
    bool actualizacionesAutomaticas;
    unsigned long ultimaVerificacion;
    unsigned long intervaloVerificacion;
    
    // Topics MQTT
    String topicCertificados;
    String topicFirmware;
    String topicEstado;
    String topicProgreso;
    String topicConfirmacion;
    
    // Estado del gestor
    bool inicializado;
    bool actualizacionEnProgreso;
    String idDispositivo;
    
    // Callbacks
    void (*callbackActualizacionCompletada)(const String& tipo, bool exito);
    void (*callbackErrorActualizacion)(const String& error);
    
public:
    GestorActualizaciones();
    ~GestorActualizaciones();
    
    // Inicialización
    bool inicializar(CertificadosManager* cert, SistemaOTA* ota, MQTTManager* mqtt, SistemaLogging* log);
    void configurarTopics(const String& idDispositivo);
    
    // Gestión principal
    void verificarActualizacionesPeriodicas();
    bool procesarComandoActualizacion(const JsonObject& comando);
    void procesarMensajeMQTT(const String& topic, const String& payload);
    
    // Actualizaciones de certificados
    bool actualizarCertificadosRemoto(const JsonObject& datos);
    bool verificarCertificadosRemoto();
    void notificarEstadoCertificados();
    bool procesarComandoCertificados(const JsonObject& comando);
    
    // Actualizaciones de firmware
    bool actualizarFirmwareRemoto(const JsonObject& datos);
    bool verificarFirmwareRemoto();
    void notificarEstadoFirmware();
    bool procesarComandoFirmware(const JsonObject& comando);
    
    // Configuración
    void establecerServidorActualizaciones(const String& servidor);
    void establecerTokenAutenticacion(const String& token);
    void habilitarActualizacionesAutomaticas(bool habilitar);
    void establecerIntervaloVerificacion(unsigned long intervalo);
    
    // Callbacks
    void establecerCallbackActualizacionCompletada(void (*callback)(const String&, bool));
    void establecerCallbackErrorActualizacion(void (*callback)(const String&));
    
    // Estado e información
    bool esInicializado() const;
    bool esActualizacionEnProgreso() const;
    String obtenerEstadoActual() const;
    String obtenerProgresoActualizacion() const;
    unsigned long obtenerUltimaVerificacion() const;
    
    // Utilidades
    void imprimirEstado() const;
    void imprimirConfiguracion() const;
    void resetear();
    
    // Notificaciones MQTT
    void enviarNotificacionEstado(const String& estado, const String& mensaje = "");
    void enviarNotificacionProgreso(int progreso, const String& descripcion = "");
    void enviarNotificacionConfirmacion(const String& comando, bool exito, const String& mensaje = "");
    void enviarNotificacionError(const String& error, const String& contexto = "");
    
    // Validación de comandos
    bool validarComandoCertificados(const JsonObject& comando);
    bool validarComandoFirmware(const JsonObject& comando);
    bool validarDatosCertificados(const JsonObject& datos);
    bool validarDatosFirmware(const JsonObject& datos);
    
    // Gestión de errores
    void manejarError(const String& error, const String& contexto);
    void registrarEvento(const String& evento, const String& detalles);
    void limpiarErrores();
    
    // Logging y debugging
    void imprimirHistorialActualizaciones() const;
    void imprimirEstadisticas() const;
    void verificarConfiguracion() const;
};

#endif
