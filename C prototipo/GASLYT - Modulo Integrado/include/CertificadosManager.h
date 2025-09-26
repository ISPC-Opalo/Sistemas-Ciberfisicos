#ifndef CERTIFICADOSMANAGER_H
#define CERTIFICADOSMANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <WiFiClientSecure.h>
#include <esp_partition.h>

class CertificadosManager {
private:
    struct CertificadosAWS {
        String certificado;
        String clavePrivada;
        String certificadoCA;
        String endpoint;
        String version;
        unsigned long timestamp;
        bool valido;
        String hash;
    } certificadosActuales, certificadosBackup;
    
    Preferences preferences;
    const esp_partition_t* particionActual;
    const esp_partition_t* particionBackup;
    bool inicializado;
    
    // Configuración de seguridad
    static const int TAMAÑO_MAX_CERTIFICADO = 8192;  // 8KB
    static const int TAMAÑO_MAX_CLAVE = 4096;        // 4KB
    static const int TAMAÑO_MAX_CA = 4096;           // 4KB
    
public:
    CertificadosManager();
    ~CertificadosManager();
    
    // Inicialización
    bool inicializar();
    bool verificarParticiones();
    
    // Gestión de certificados
    bool cargarCertificadosDesdeParticion();
    bool guardarCertificadosEnParticion();
    bool actualizarCertificados(const String& certificado, const String& clave, 
                               const String& ca, const String& endpoint, const String& version);
    bool validarCertificados();
    bool hacerBackupCertificados();
    bool restaurarCertificadosDesdeBackup();
    
    // Verificación de integridad
    bool verificarIntegridadCertificados();
    String calcularHashCertificados();
    bool verificarFirmaDigital(const String& datos, const String& firma);
    bool verificarFormatoCertificado(const String& certificado);
    bool verificarFormatoClavePrivada(const String& clave);
    
    // Gestión de versiones
    String obtenerVersionCertificados() const;
    bool esNuevaVersion(const String& version) const;
    void establecerVersionCertificados(const String& version);
    unsigned long obtenerTimestampCertificados() const;
    
    // Acceso a certificados
    String obtenerCertificado() const;
    String obtenerClavePrivada() const;
    String obtenerCertificadoCA() const;
    String obtenerEndpoint() const;
    String obtenerHash() const;
    
    // Estado y validación
    bool esValido() const;
    bool esInicializado() const;
    bool tieneBackup() const;
    
    // Utilidades
    void imprimirEstado() const;
    void limpiarCertificados();
    void resetear();
    size_t obtenerTamañoCertificados() const;
    
    // Configuración de WiFiClientSecure
    bool configurarClienteSeguro(WiFiClientSecure* cliente);
    bool configurarClienteSeguroConBackup(WiFiClientSecure* cliente);
    
    // Logging y debugging
    void imprimirInfoCertificados() const;
    void imprimirInfoParticiones() const;
    bool verificarEspacioDisponible() const;
};

#endif
