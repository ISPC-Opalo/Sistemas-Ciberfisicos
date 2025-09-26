#include "CertificadosManager.h"
#include "SistemaLogging.h"

CertificadosManager::CertificadosManager() : 
    inicializado(false), particionActual(nullptr), particionBackup(nullptr) {
    
    // Inicializar estructuras
    certificadosActuales.certificado = "";
    certificadosActuales.clavePrivada = "";
    certificadosActuales.certificadoCA = "";
    certificadosActuales.endpoint = "";
    certificadosActuales.version = "1.0.0";
    certificadosActuales.timestamp = 0;
    certificadosActuales.valido = false;
    certificadosActuales.hash = "";
    
    certificadosBackup = certificadosActuales;
}

CertificadosManager::~CertificadosManager() {
    if (preferences.isOpen()) {
        preferences.end();
    }
}

bool CertificadosManager::inicializar() {
    if (inicializado) {
        return true;
    }
    
    // Inicializar Preferences
    if (!preferences.begin("certificados", false)) {
        Serial.println("Error al inicializar Preferences para certificados");
        return false;
    }
    
    // Verificar particiones
    if (!verificarParticiones()) {
        Serial.println("Error al verificar particiones de certificados");
        return false;
    }
    
    // Cargar certificados desde partición
    if (!cargarCertificadosDesdeParticion()) {
        Serial.println("No se pudieron cargar certificados desde partición");
        // No es error crítico, puede ser primera vez
    }
    
    inicializado = true;
    Serial.println("CertificadosManager inicializado correctamente");
    
    return true;
}

bool CertificadosManager::verificarParticiones() {
    // Buscar partición de certificados actuales
    particionActual = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, 
                                              ESP_PARTITION_SUBTYPE_DATA_UNDEFINED, 
                                              "certs_current");
    
    // Buscar partición de backup
    particionBackup = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, 
                                              ESP_PARTITION_SUBTYPE_DATA_UNDEFINED, 
                                              "certs_backup");
    
    if (!particionActual || !particionBackup) {
        Serial.println("Error: No se encontraron las particiones de certificados");
        return false;
    }
    
    Serial.println("Particiones de certificados encontradas:");
    Serial.println("- Actual: " + String(particionActual->size) + " bytes");
    Serial.println("- Backup: " + String(particionBackup->size) + " bytes");
    
    return true;
}

bool CertificadosManager::cargarCertificadosDesdeParticion() {
    if (!particionActual) {
        return false;
    }
    
    // Leer datos de la partición
    uint8_t* buffer = (uint8_t*)malloc(particionActual->size);
    if (!buffer) {
        Serial.println("Error al asignar memoria para certificados");
        return false;
    }
    
    esp_err_t result = esp_partition_read(particionActual, 0, buffer, particionActual->size);
    if (result != ESP_OK) {
        Serial.println("Error al leer partición de certificados: " + String(esp_err_to_name(result)));
        free(buffer);
        return false;
    }
    
    // Parsear datos (formato: version|timestamp|hash|certificado|clave|ca|endpoint)
    String datos = String((char*)buffer);
    free(buffer);
    
    if (datos.length() == 0) {
        Serial.println("Partición de certificados vacía");
        return false;
    }
    
    // Parsear campos
    int pos = 0;
    int nextPos = datos.indexOf('|', pos);
    if (nextPos == -1) return false;
    certificadosActuales.version = datos.substring(pos, nextPos);
    
    pos = nextPos + 1;
    nextPos = datos.indexOf('|', pos);
    if (nextPos == -1) return false;
    certificadosActuales.timestamp = datos.substring(pos, nextPos).toInt();
    
    pos = nextPos + 1;
    nextPos = datos.indexOf('|', pos);
    if (nextPos == -1) return false;
    certificadosActuales.hash = datos.substring(pos, nextPos);
    
    pos = nextPos + 1;
    nextPos = datos.indexOf('|', pos);
    if (nextPos == -1) return false;
    certificadosActuales.certificado = datos.substring(pos, nextPos);
    
    pos = nextPos + 1;
    nextPos = datos.indexOf('|', pos);
    if (nextPos == -1) return false;
    certificadosActuales.clavePrivada = datos.substring(pos, nextPos);
    
    pos = nextPos + 1;
    nextPos = datos.indexOf('|', pos);
    if (nextPos == -1) return false;
    certificadosActuales.certificadoCA = datos.substring(pos, nextPos);
    
    pos = nextPos + 1;
    certificadosActuales.endpoint = datos.substring(pos);
    
    // Validar certificados cargados
    certificadosActuales.valido = validarCertificados();
    
    Serial.println("Certificados cargados desde partición:");
    Serial.println("- Versión: " + certificadosActuales.version);
    Serial.println("- Timestamp: " + String(certificadosActuales.timestamp));
    Serial.println("- Válido: " + String(certificadosActuales.valido ? "Sí" : "No"));
    
    return certificadosActuales.valido;
}

bool CertificadosManager::guardarCertificadosEnParticion() {
    if (!particionActual) {
        return false;
    }
    
    // Preparar datos para guardar
    String datos = certificadosActuales.version + "|" +
                   String(certificadosActuales.timestamp) + "|" +
                   certificadosActuales.hash + "|" +
                   certificadosActuales.certificado + "|" +
                   certificadosActuales.clavePrivada + "|" +
                   certificadosActuales.certificadoCA + "|" +
                   certificadosActuales.endpoint;
    
    // Verificar tamaño
    if (datos.length() > particionActual->size) {
        Serial.println("Error: Certificados exceden el tamaño de la partición");
        return false;
    }
    
    // Escribir en partición
    esp_err_t result = esp_partition_erase_range(particionActual, 0, particionActual->size);
    if (result != ESP_OK) {
        Serial.println("Error al borrar partición: " + String(esp_err_to_name(result)));
        return false;
    }
    
    result = esp_partition_write(particionActual, 0, datos.c_str(), datos.length());
    if (result != ESP_OK) {
        Serial.println("Error al escribir en partición: " + String(esp_err_to_name(result)));
        return false;
    }
    
    Serial.println("Certificados guardados en partición exitosamente");
    return true;
}

bool CertificadosManager::actualizarCertificados(const String& certificado, const String& clave, 
                                                const String& ca, const String& endpoint, const String& version) {
    // Validar entrada
    if (certificado.isEmpty() || clave.isEmpty() || ca.isEmpty() || endpoint.isEmpty()) {
        Serial.println("Error: Certificados incompletos");
        return false;
    }
    
    // Verificar formato
    if (!verificarFormatoCertificado(certificado) || 
        !verificarFormatoClavePrivada(clave) ||
        !verificarFormatoCertificado(ca)) {
        Serial.println("Error: Formato de certificados inválido");
        return false;
    }
    
    // Hacer backup antes de actualizar
    if (!hacerBackupCertificados()) {
        Serial.println("Advertencia: No se pudo hacer backup de certificados");
    }
    
    // Actualizar certificados
    certificadosActuales.certificado = certificado;
    certificadosActuales.clavePrivada = clave;
    certificadosActuales.certificadoCA = ca;
    certificadosActuales.endpoint = endpoint;
    certificadosActuales.version = version;
    certificadosActuales.timestamp = millis();
    certificadosActuales.hash = calcularHashCertificados();
    
    // Validar nuevos certificados
    if (!validarCertificados()) {
        Serial.println("Error: Los nuevos certificados no son válidos");
        // Restaurar desde backup
        restaurarCertificadosDesdeBackup();
        return false;
    }
    
    // Guardar en partición
    if (!guardarCertificadosEnParticion()) {
        Serial.println("Error: No se pudieron guardar los certificados");
        // Restaurar desde backup
        restaurarCertificadosDesdeBackup();
        return false;
    }
    
    Serial.println("Certificados actualizados exitosamente");
    Serial.println("- Nueva versión: " + version);
    Serial.println("- Timestamp: " + String(certificadosActuales.timestamp));
    
    return true;
}

bool CertificadosManager::validarCertificados() {
    if (certificadosActuales.certificado.isEmpty() || 
        certificadosActuales.clavePrivada.isEmpty() ||
        certificadosActuales.certificadoCA.isEmpty()) {
        return false;
    }
    
    // Verificar formato básico
    if (!verificarFormatoCertificado(certificadosActuales.certificado) ||
        !verificarFormatoClavePrivada(certificadosActuales.clavePrivada) ||
        !verificarFormatoCertificado(certificadosActuales.certificadoCA)) {
        return false;
    }
    
    // Verificar integridad
    String hashCalculado = calcularHashCertificados();
    if (hashCalculado != certificadosActuales.hash) {
        Serial.println("Error: Hash de certificados no coincide");
        return false;
    }
    
    return true;
}

bool CertificadosManager::hacerBackupCertificados() {
    if (!particionBackup) {
        return false;
    }
    
    // Copiar certificados actuales a backup
    certificadosBackup = certificadosActuales;
    
    // Preparar datos para backup
    String datos = certificadosBackup.version + "|" +
                   String(certificadosBackup.timestamp) + "|" +
                   certificadosBackup.hash + "|" +
                   certificadosBackup.certificado + "|" +
                   certificadosBackup.clavePrivada + "|" +
                   certificadosBackup.certificadoCA + "|" +
                   certificadosBackup.endpoint;
    
    // Escribir en partición de backup
    esp_err_t result = esp_partition_erase_range(particionBackup, 0, particionBackup->size);
    if (result != ESP_OK) {
        Serial.println("Error al borrar partición de backup: " + String(esp_err_to_name(result)));
        return false;
    }
    
    result = esp_partition_write(particionBackup, 0, datos.c_str(), datos.length());
    if (result != ESP_OK) {
        Serial.println("Error al escribir backup: " + String(esp_err_to_name(result)));
        return false;
    }
    
    Serial.println("Backup de certificados creado exitosamente");
    return true;
}

bool CertificadosManager::restaurarCertificadosDesdeBackup() {
    if (!particionBackup) {
        return false;
    }
    
    // Leer backup
    uint8_t* buffer = (uint8_t*)malloc(particionBackup->size);
    if (!buffer) {
        Serial.println("Error al asignar memoria para backup");
        return false;
    }
    
    esp_err_t result = esp_partition_read(particionBackup, 0, buffer, particionBackup->size);
    if (result != ESP_OK) {
        Serial.println("Error al leer backup: " + String(esp_err_to_name(result)));
        free(buffer);
        return false;
    }
    
    // Parsear backup
    String datos = String((char*)buffer);
    free(buffer);
    
    if (datos.length() == 0) {
        Serial.println("Backup de certificados vacío");
        return false;
    }
    
    // Restaurar certificados
    certificadosActuales = certificadosBackup;
    
    // Guardar certificados restaurados
    if (!guardarCertificadosEnParticion()) {
        Serial.println("Error al guardar certificados restaurados");
        return false;
    }
    
    Serial.println("Certificados restaurados desde backup exitosamente");
    return true;
}

bool CertificadosManager::verificarIntegridadCertificados() {
    return validarCertificados();
}

String CertificadosManager::calcularHashCertificados() {
    String datos = certificadosActuales.certificado + 
                   certificadosActuales.clavePrivada + 
                   certificadosActuales.certificadoCA + 
                   certificadosActuales.endpoint;
    
    // Calcular hash SHA-256 simple (implementación básica)
    // En producción usar librería de hash real
    uint32_t hash = 0;
    for (int i = 0; i < datos.length(); i++) {
        hash = hash * 31 + datos.charAt(i);
    }
    
    return String(hash, HEX);
}

bool CertificadosManager::verificarFirmaDigital(const String& datos, const String& firma) {
    // Implementación básica de verificación de firma
    // En producción usar librería de criptografía real
    if (firma.isEmpty()) {
        return false;
    }
    
    // Verificar que la firma comience con "rsa:"
    if (!firma.startsWith("rsa:")) {
        return false;
    }
    
    // Verificar longitud mínima de firma
    if (firma.length() < 10) {
        return false;
    }
    
    return true;
}

bool CertificadosManager::verificarFormatoCertificado(const String& certificado) {
    return certificado.startsWith("-----BEGIN CERTIFICATE-----") && 
           certificado.endsWith("-----END CERTIFICATE-----");
}

bool CertificadosManager::verificarFormatoClavePrivada(const String& clave) {
    return clave.startsWith("-----BEGIN PRIVATE KEY-----") && 
           clave.endsWith("-----END PRIVATE KEY-----");
}

// Getters
String CertificadosManager::obtenerVersionCertificados() const {
    return certificadosActuales.version;
}

bool CertificadosManager::esNuevaVersion(const String& version) const {
    return version != certificadosActuales.version;
}

void CertificadosManager::establecerVersionCertificados(const String& version) {
    certificadosActuales.version = version;
}

unsigned long CertificadosManager::obtenerTimestampCertificados() const {
    return certificadosActuales.timestamp;
}

String CertificadosManager::obtenerCertificado() const {
    return certificadosActuales.certificado;
}

String CertificadosManager::obtenerClavePrivada() const {
    return certificadosActuales.clavePrivada;
}

String CertificadosManager::obtenerCertificadoCA() const {
    return certificadosActuales.certificadoCA;
}

String CertificadosManager::obtenerEndpoint() const {
    return certificadosActuales.endpoint;
}

String CertificadosManager::obtenerHash() const {
    return certificadosActuales.hash;
}

bool CertificadosManager::esValido() const {
    return certificadosActuales.valido;
}

bool CertificadosManager::esInicializado() const {
    return inicializado;
}

bool CertificadosManager::tieneBackup() const {
    return !certificadosBackup.certificado.isEmpty();
}

bool CertificadosManager::configurarClienteSeguro(WiFiClientSecure* cliente) {
    if (!cliente || !esValido()) {
        return false;
    }
    
    cliente->setCACert(certificadosActuales.certificadoCA.c_str());
    cliente->setCertificate(certificadosActuales.certificado.c_str());
    cliente->setPrivateKey(certificadosActuales.clavePrivada.c_str());
    
    return true;
}

bool CertificadosManager::configurarClienteSeguroConBackup(WiFiClientSecure* cliente) {
    // Intentar con certificados actuales
    if (configurarClienteSeguro(cliente)) {
        return true;
    }
    
    // Si falla, intentar con backup
    if (tieneBackup()) {
        cliente->setCACert(certificadosBackup.certificadoCA.c_str());
        cliente->setCertificate(certificadosBackup.certificado.c_str());
        cliente->setPrivateKey(certificadosBackup.clavePrivada.c_str());
        return true;
    }
    
    return false;
}

void CertificadosManager::imprimirEstado() const {
    Serial.println("=== ESTADO CERTIFICADOS ===");
    Serial.println("Inicializado: " + String(inicializado ? "Sí" : "No"));
    Serial.println("Válido: " + String(esValido() ? "Sí" : "No"));
    Serial.println("Versión: " + certificadosActuales.version);
    Serial.println("Timestamp: " + String(certificadosActuales.timestamp));
    Serial.println("Endpoint: " + certificadosActuales.endpoint);
    Serial.println("Hash: " + certificadosActuales.hash);
    Serial.println("Tiene backup: " + String(tieneBackup() ? "Sí" : "No"));
    Serial.println("Tamaño: " + String(obtenerTamañoCertificados()) + " bytes");
    Serial.println("==========================");
}

void CertificadosManager::imprimirInfoCertificados() const {
    Serial.println("=== INFO CERTIFICADOS ===");
    Serial.println("Certificado: " + String(certificadosActuales.certificado.length()) + " caracteres");
    Serial.println("Clave privada: " + String(certificadosActuales.clavePrivada.length()) + " caracteres");
    Serial.println("Certificado CA: " + String(certificadosActuales.certificadoCA.length()) + " caracteres");
    Serial.println("=========================");
}

void CertificadosManager::imprimirInfoParticiones() const {
    Serial.println("=== INFO PARTICIONES ===");
    if (particionActual) {
        Serial.println("Partición actual: " + String(particionActual->size) + " bytes");
    }
    if (particionBackup) {
        Serial.println("Partición backup: " + String(particionBackup->size) + " bytes");
    }
    Serial.println("=======================");
}

bool CertificadosManager::verificarEspacioDisponible() const {
    if (!particionActual) {
        return false;
    }
    
    size_t tamañoNecesario = certificadosActuales.certificado.length() +
                            certificadosActuales.clavePrivada.length() +
                            certificadosActuales.certificadoCA.length() +
                            certificadosActuales.endpoint.length() + 100; // Overhead
    
    return tamañoNecesario < particionActual->size;
}

void CertificadosManager::limpiarCertificados() {
    certificadosActuales.certificado = "";
    certificadosActuales.clavePrivada = "";
    certificadosActuales.certificadoCA = "";
    certificadosActuales.endpoint = "";
    certificadosActuales.version = "1.0.0";
    certificadosActuales.timestamp = 0;
    certificadosActuales.valido = false;
    certificadosActuales.hash = "";
}

void CertificadosManager::resetear() {
    limpiarCertificados();
    certificadosBackup = certificadosActuales;
    inicializado = false;
}

size_t CertificadosManager::obtenerTamañoCertificados() const {
    return certificadosActuales.certificado.length() +
           certificadosActuales.clavePrivada.length() +
           certificadosActuales.certificadoCA.length() +
           certificadosActuales.endpoint.length();
}
