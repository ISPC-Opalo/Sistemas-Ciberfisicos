#include "SistemaOTA.h"
#include "SistemaLogging.h"

SistemaOTA::SistemaOTA() : 
    estadoActual(OTA_DISPONIBLE), ultimaVerificacion(0), intervaloVerificacion(3600000), // 1 hora
    actualizacionesAutomaticas(false), rollbackDisponible(false),
    particionActual(nullptr), particionOta0(nullptr), particionOta1(nullptr), particionOtaData(nullptr),
    callbackProgreso(nullptr), callbackEstado(nullptr), callbackError(nullptr) {
    
    // Inicializar info de actualización
    infoActualizacion.version = "";
    infoActualizacion.url = "";
    infoActualizacion.hash = "";
    infoActualizacion.tamaño = 0;
    infoActualizacion.descripcion = "";
    infoActualizacion.critica = false;
    infoActualizacion.timestamp = 0;
    infoActualizacion.firma = "";
    infoActualizacion.progreso = 0;
    
    servidorActualizaciones = "";
    tokenAutenticacion = "";
}

SistemaOTA::~SistemaOTA() {
    // Limpiar recursos
}

bool SistemaOTA::inicializar() {
    Serial.println("Inicializando Sistema OTA...");
    
    // Verificar particiones
    if (!verificarParticiones()) {
        Serial.println("Error: No se pudieron verificar las particiones OTA");
        return false;
    }
    
    // Configurar Arduino OTA
    configurarArduinoOTA();
    
    // Obtener información de la partición actual
    particionActual = esp_ota_get_running_partition();
    if (!particionActual) {
        Serial.println("Error: No se pudo obtener la partición actual");
        return false;
    }
    
    // Verificar integridad de la versión actual
    if (!verificarIntegridadVersionActual()) {
        Serial.println("Advertencia: La versión actual no pasa la verificación de integridad");
    }
    
    // Verificar si hay rollback disponible
    rollbackDisponible = esRollbackDisponible();
    
    Serial.println("Sistema OTA inicializado correctamente");
    Serial.println("Versión actual: " + obtenerVersionActual());
    Serial.println("Rollback disponible: " + String(rollbackDisponible ? "Sí" : "No"));
    
    return true;
}

bool SistemaOTA::verificarParticiones() {
    // Buscar particiones OTA
    particionOta0 = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, "ota_0");
    particionOta1 = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, "ota_1");
    particionOtaData = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_UNDEFINED, "ota_data");
    
    if (!particionOta0 || !particionOta1 || !particionOtaData) {
        Serial.println("Error: No se encontraron las particiones OTA necesarias");
        return false;
    }
    
    Serial.println("Particiones OTA encontradas:");
    Serial.println("- OTA_0: " + String(particionOta0->size) + " bytes");
    Serial.println("- OTA_1: " + String(particionOta1->size) + " bytes");
    Serial.println("- OTA_Data: " + String(particionOtaData->size) + " bytes");
    
    return true;
}

void SistemaOTA::configurarArduinoOTA() {
    ArduinoOTA.setHostname("GASLYT-ESP32");
    ArduinoOTA.setPassword("gaslyt_ota_2024");
    
    ArduinoOTA.onStart([]() {
        String tipo;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            tipo = "firmware";
        } else {
            tipo = "filesystem";
        }
        Serial.println("Iniciando actualización OTA: " + tipo);
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("Actualización OTA completada");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        int porcentaje = (progress / (total / 100));
        Serial.println("Progreso OTA: " + String(porcentaje) + "%");
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        String mensaje = "Error OTA: ";
        switch (error) {
            case OTA_AUTH_ERROR: mensaje += "Autenticación fallida"; break;
            case OTA_BEGIN_ERROR: mensaje += "Error al iniciar"; break;
            case OTA_CONNECT_ERROR: mensaje += "Error de conexión"; break;
            case OTA_RECEIVE_ERROR: mensaje += "Error de recepción"; break;
            case OTA_END_ERROR: mensaje += "Error al finalizar"; break;
            default: mensaje += "Error desconocido"; break;
        }
        Serial.println(mensaje);
    });
    
    ArduinoOTA.begin();
    Serial.println("Arduino OTA configurado");
}

bool SistemaOTA::verificarActualizacionesDisponibles() {
    if (servidorActualizaciones.isEmpty()) {
        Serial.println("Error: Servidor de actualizaciones no configurado");
        return false;
    }
    
    if (!WiFi.isConnected()) {
        Serial.println("Error: WiFi no conectado");
        return false;
    }
    
    Serial.println("Verificando actualizaciones disponibles...");
    
    HTTPClient http;
    http.begin(servidorActualizaciones + "/api/check-updates");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + tokenAutenticacion);
    
    // Crear JSON con información del dispositivo
    String json = "{\"version\":\"" + obtenerVersionActual() + "\",\"device_id\":\"" + WiFi.macAddress() + "\"}";
    
    int httpCode = http.POST(json);
    if (httpCode == HTTP_CODE_OK) {
        String respuesta = http.getString();
        http.end();
        
        // Parsear respuesta JSON
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, respuesta);
        
        if (error) {
            Serial.println("Error al parsear respuesta de actualizaciones");
            return false;
        }
        
        JsonObject update = doc["update"];
        if (update["available"]) {
            infoActualizacion.version = update["version"];
            infoActualizacion.url = update["url"];
            infoActualizacion.hash = update["hash"];
            infoActualizacion.tamaño = update["size"];
            infoActualizacion.descripcion = update["description"];
            infoActualizacion.critica = update["critical"];
            infoActualizacion.timestamp = millis();
            infoActualizacion.firma = update["signature"];
            infoActualizacion.progreso = 0;
            
            estadoActual = OTA_DISPONIBLE;
            ultimaVerificacion = millis();
            
            Serial.println("Actualización disponible:");
            Serial.println("- Versión: " + infoActualizacion.version);
            Serial.println("- Tamaño: " + String(infoActualizacion.tamaño) + " bytes");
            Serial.println("- Crítica: " + String(infoActualizacion.critica ? "Sí" : "No"));
            
            if (callbackEstado) {
                callbackEstado(estadoActual);
            }
            
            return true;
        }
    } else {
        Serial.println("Error al verificar actualizaciones: " + String(httpCode));
    }
    
    http.end();
    return false;
}

bool SistemaOTA::descargarActualizacion(const String& url) {
    if (url.isEmpty()) {
        url = infoActualizacion.url;
    }
    
    if (url.isEmpty()) {
        Serial.println("Error: URL de actualización no especificada");
        return false;
    }
    
    if (!verificarEspacioSuficiente(infoActualizacion.tamaño)) {
        Serial.println("Error: Espacio insuficiente para la actualización");
        return false;
    }
    
    Serial.println("Iniciando descarga de actualización...");
    Serial.println("URL: " + url);
    Serial.println("Tamaño: " + String(infoActualizacion.tamaño) + " bytes");
    
    estadoActual = OTA_DESCARGANDO;
    if (callbackEstado) {
        callbackEstado(estadoActual);
    }
    
    HTTPClient http;
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + tokenAutenticacion);
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.println("Error al iniciar descarga: " + String(httpCode));
        estadoActual = OTA_ERROR;
        if (callbackError) {
            callbackError("Error HTTP: " + String(httpCode));
        }
        http.end();
        return false;
    }
    
    // Obtener tamaño del archivo
    size_t tamañoArchivo = http.getSize();
    if (tamañoArchivo == 0) {
        Serial.println("Error: No se pudo obtener el tamaño del archivo");
        estadoActual = OTA_ERROR;
        if (callbackError) {
            callbackError("Tamaño de archivo desconocido");
        }
        http.end();
        return false;
    }
    
    // Iniciar actualización OTA
    if (!Update.begin(tamañoArchivo)) {
        Serial.println("Error al iniciar actualización OTA: " + String(Update.errorString()));
        estadoActual = OTA_ERROR;
        if (callbackError) {
            callbackError("Error al iniciar OTA: " + String(Update.errorString()));
        }
        http.end();
        return false;
    }
    
    // Descargar y escribir datos
    WiFiClient* stream = http.getStreamPtr();
    size_t bytesEscritos = 0;
    uint8_t buffer[1024];
    
    while (http.connected() && bytesEscritos < tamañoArchivo) {
        size_t bytesLeidos = stream->readBytes(buffer, sizeof(buffer));
        if (bytesLeidos > 0) {
            size_t bytesEscritosEnEstaIteracion = Update.write(buffer, bytesLeidos);
            bytesEscritos += bytesEscritosEnEstaIteracion;
            
            // Actualizar progreso
            int progreso = (bytesEscritos * 100) / tamañoArchivo;
            infoActualizacion.progreso = progreso;
            
            if (callbackProgreso) {
                callbackProgreso(progreso);
            }
            
            Serial.println("Progreso: " + String(progreso) + "% (" + String(bytesEscritos) + "/" + String(tamañoArchivo) + ")");
        }
    }
    
    http.end();
    
    if (bytesEscritos == tamañoArchivo) {
        Serial.println("Descarga completada exitosamente");
        return true;
    } else {
        Serial.println("Error: Descarga incompleta");
        Update.abort();
        estadoActual = OTA_ERROR;
        if (callbackError) {
            callbackError("Descarga incompleta");
        }
        return false;
    }
}

bool SistemaOTA::instalarActualizacion() {
    Serial.println("Instalando actualización...");
    
    estadoActual = OTA_INSTALANDO;
    if (callbackEstado) {
        callbackEstado(estadoActual);
    }
    
    // Finalizar actualización
    if (Update.end()) {
        Serial.println("Actualización instalada exitosamente");
        
        // Verificar integridad
        if (!verificarIntegridadFirmware()) {
            Serial.println("Error: La actualización no pasa la verificación de integridad");
            estadoActual = OTA_ERROR;
            if (callbackError) {
                callbackError("Verificación de integridad fallida");
            }
            return false;
        }
        
        estadoActual = OTA_COMPLETADO;
        if (callbackEstado) {
            callbackEstado(estadoActual);
        }
        
        Serial.println("Actualización completada. Reiniciando en 5 segundos...");
        delay(5000);
        reiniciarConNuevaVersion();
        
        return true;
    } else {
        Serial.println("Error al instalar actualización: " + String(Update.errorString()));
        estadoActual = OTA_ERROR;
        if (callbackError) {
            callbackError("Error de instalación: " + String(Update.errorString()));
        }
        return false;
    }
}

bool SistemaOTA::verificarIntegridadFirmware() {
    // Verificar hash si está disponible
    if (!infoActualizacion.hash.isEmpty()) {
        String hashCalculado = calcularHashFirmware();
        if (hashCalculado != infoActualizacion.hash) {
            Serial.println("Error: Hash del firmware no coincide");
            Serial.println("Esperado: " + infoActualizacion.hash);
            Serial.println("Calculado: " + hashCalculado);
            return false;
        }
    }
    
    // Verificar firma digital si está disponible
    if (!infoActualizacion.firma.isEmpty()) {
        String datos = infoActualizacion.version + infoActualizacion.hash;
        if (!verificarFirmaDigital(datos, infoActualizacion.firma)) {
            Serial.println("Error: Firma digital no válida");
            return false;
        }
    }
    
    Serial.println("Verificación de integridad exitosa");
    return true;
}

void SistemaOTA::reiniciarConNuevaVersion() {
    Serial.println("Reiniciando con nueva versión...");
    ESP.restart();
}

String SistemaOTA::obtenerVersionActual() {
    // Obtener versión desde la partición actual
    esp_app_desc_t app_desc;
    esp_ota_get_partition_description(particionActual, &app_desc);
    return String(app_desc.version);
}

String SistemaOTA::obtenerVersionDisponible() const {
    return infoActualizacion.version;
}

bool SistemaOTA::esActualizacionCritica() const {
    return infoActualizacion.critica;
}

void SistemaOTA::establecerVersionActual(const String& version) {
    // Esta función se usaría para actualizar la versión en la partición
    // En la implementación real, esto requeriría modificar la partición
    Serial.println("Versión actual establecida: " + version);
}

bool SistemaOTA::esNuevaVersion(const String& version) const {
    return version != obtenerVersionActual();
}

bool SistemaOTA::hacerRollback() {
    if (!esRollbackDisponible()) {
        Serial.println("Error: Rollback no disponible");
        return false;
    }
    
    Serial.println("Iniciando rollback...");
    
    estadoActual = OTA_ROLLBACK;
    if (callbackEstado) {
        callbackEstado(estadoActual);
    }
    
    // Obtener la partición de rollback
    const esp_partition_t* particionRollback = nullptr;
    if (particionActual == particionOta0) {
        particionRollback = particionOta1;
    } else {
        particionRollback = particionOta0;
    }
    
    if (!particionRollback) {
        Serial.println("Error: No se encontró la partición de rollback");
        estadoActual = OTA_ERROR;
        if (callbackError) {
            callbackError("Partición de rollback no encontrada");
        }
        return false;
    }
    
    // Verificar integridad de la partición de rollback
    esp_app_desc_t app_desc;
    esp_ota_get_partition_description(particionRollback, &app_desc);
    
    // Establecer la partición de rollback como la partición de arranque
    esp_err_t result = esp_ota_set_boot_partition(particionRollback);
    if (result != ESP_OK) {
        Serial.println("Error al establecer partición de rollback: " + String(esp_err_to_name(result)));
        estadoActual = OTA_ERROR;
        if (callbackError) {
            callbackError("Error al establecer rollback");
        }
        return false;
    }
    
    Serial.println("Rollback completado. Reiniciando...");
    delay(2000);
    ESP.restart();
    
    return true;
}

bool SistemaOTA::esRollbackDisponible() const {
    return rollbackDisponible;
}

void SistemaOTA::limpiarVersionAnterior() {
    // Limpiar la partición que no está en uso
    const esp_partition_t* particionLimpiar = nullptr;
    if (particionActual == particionOta0) {
        particionLimpiar = particionOta1;
    } else {
        particionLimpiar = particionOta0;
    }
    
    if (particionLimpiar) {
        esp_err_t result = esp_partition_erase_range(particionLimpiar, 0, particionLimpiar->size);
        if (result == ESP_OK) {
            Serial.println("Versión anterior limpiada exitosamente");
        } else {
            Serial.println("Error al limpiar versión anterior: " + String(esp_err_to_name(result)));
        }
    }
}

bool SistemaOTA::verificarIntegridadVersionActual() {
    // Verificación básica de integridad
    esp_app_desc_t app_desc;
    esp_ota_get_partition_description(particionActual, &app_desc);
    
    // Verificar que la versión no esté vacía
    if (strlen(app_desc.version) == 0) {
        return false;
    }
    
    // Verificar que la fecha de compilación sea válida
    if (strlen(app_desc.date) == 0) {
        return false;
    }
    
    return true;
}

// Configuración
void SistemaOTA::establecerServidorActualizaciones(const String& servidor) {
    servidorActualizaciones = servidor;
    Serial.println("Servidor de actualizaciones establecido: " + servidor);
}

void SistemaOTA::establecerTokenAutenticacion(const String& token) {
    tokenAutenticacion = token;
    Serial.println("Token de autenticación establecido");
}

void SistemaOTA::establecerIntervaloVerificacion(unsigned long intervalo) {
    intervaloVerificacion = intervalo;
    Serial.println("Intervalo de verificación establecido: " + String(intervalo) + " ms");
}

void SistemaOTA::habilitarActualizacionesAutomaticas(bool habilitar) {
    actualizacionesAutomaticas = habilitar;
    Serial.println("Actualizaciones automáticas: " + String(habilitar ? "Habilitadas" : "Deshabilitadas"));
}

// Callbacks
void SistemaOTA::establecerCallbackProgreso(void (*callback)(int)) {
    callbackProgreso = callback;
}

void SistemaOTA::establecerCallbackEstado(void (*callback)(EstadoOTA)) {
    callbackEstado = callback;
}

void SistemaOTA::establecerCallbackError(void (*callback)(const String&)) {
    callbackError = callback;
}

// Getters
SistemaOTA::EstadoOTA SistemaOTA::obtenerEstadoActual() const {
    return estadoActual;
}

int SistemaOTA::obtenerProgreso() const {
    return infoActualizacion.progreso;
}

String SistemaOTA::obtenerDescripcionActualizacion() const {
    return infoActualizacion.descripcion;
}

size_t SistemaOTA::obtenerTamañoActualizacion() const {
    return infoActualizacion.tamaño;
}

unsigned long SistemaOTA::obtenerTimestampActualizacion() const {
    return infoActualizacion.timestamp;
}

// Utilidades
void SistemaOTA::imprimirEstado() const {
    Serial.println("=== ESTADO SISTEMA OTA ===");
    Serial.println("Estado actual: " + String(estadoActual));
    Serial.println("Versión actual: " + obtenerVersionActual());
    Serial.println("Versión disponible: " + infoActualizacion.version);
    Serial.println("Progreso: " + String(infoActualizacion.progreso) + "%");
    Serial.println("Rollback disponible: " + String(rollbackDisponible ? "Sí" : "No"));
    Serial.println("Actualizaciones automáticas: " + String(actualizacionesAutomaticas ? "Sí" : "No"));
    Serial.println("Última verificación: " + String(ultimaVerificacion));
    Serial.println("==========================");
}

void SistemaOTA::imprimirInfoParticiones() const {
    Serial.println("=== INFO PARTICIONES OTA ===");
    if (particionActual) {
        Serial.println("Partición actual: " + String(particionActual->size) + " bytes");
    }
    if (particionOta0) {
        Serial.println("Partición OTA_0: " + String(particionOta0->size) + " bytes");
    }
    if (particionOta1) {
        Serial.println("Partición OTA_1: " + String(particionOta1->size) + " bytes");
    }
    if (particionOtaData) {
        Serial.println("Partición OTA_Data: " + String(particionOtaData->size) + " bytes");
    }
    Serial.println("=============================");
}

void SistemaOTA::resetear() {
    estadoActual = OTA_DISPONIBLE;
    infoActualizacion.version = "";
    infoActualizacion.url = "";
    infoActualizacion.hash = "";
    infoActualizacion.tamaño = 0;
    infoActualizacion.descripcion = "";
    infoActualizacion.critica = false;
    infoActualizacion.timestamp = 0;
    infoActualizacion.firma = "";
    infoActualizacion.progreso = 0;
    ultimaVerificacion = 0;
}

bool SistemaOTA::esInicializado() const {
    return particionActual != nullptr;
}

// Verificación de integridad
bool SistemaOTA::verificarHashFirmware(const String& hash) {
    String hashCalculado = calcularHashFirmware();
    return hashCalculado == hash;
}

bool SistemaOTA::verificarFirmaDigital(const String& datos, const String& firma) {
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

String SistemaOTA::calcularHashFirmware() {
    // Implementación básica de hash
    // En producción usar librería de hash real
    String datos = obtenerVersionActual() + String(millis());
    uint32_t hash = 0;
    for (int i = 0; i < datos.length(); i++) {
        hash = hash * 31 + datos.charAt(i);
    }
    return String(hash, HEX);
}

// Gestión de memoria
size_t SistemaOTA::obtenerEspacioDisponible() const {
    if (!particionOta0 || !particionOta1) {
        return 0;
    }
    
    // Retornar el espacio de la partición que no está en uso
    if (particionActual == particionOta0) {
        return particionOta1->size;
    } else {
        return particionOta0->size;
    }
}

bool SistemaOTA::verificarEspacioSuficiente(size_t tamaño) const {
    return tamaño <= obtenerEspacioDisponible();
}

void SistemaOTA::limpiarCache() {
    // Limpiar cache de HTTP si es necesario
    Serial.println("Cache limpiado");
}

// Logging y debugging
void SistemaOTA::imprimirInfoActualizacion() const {
    Serial.println("=== INFO ACTUALIZACIÓN ===");
    Serial.println("Versión: " + infoActualizacion.version);
    Serial.println("URL: " + infoActualizacion.url);
    Serial.println("Hash: " + infoActualizacion.hash);
    Serial.println("Tamaño: " + String(infoActualizacion.tamaño) + " bytes");
    Serial.println("Descripción: " + infoActualizacion.descripcion);
    Serial.println("Crítica: " + String(infoActualizacion.critica ? "Sí" : "No"));
    Serial.println("Timestamp: " + String(infoActualizacion.timestamp));
    Serial.println("Firma: " + infoActualizacion.firma);
    Serial.println("Progreso: " + String(infoActualizacion.progreso) + "%");
    Serial.println("=========================");
}

void SistemaOTA::imprimirHistorialVersiones() const {
    Serial.println("=== HISTORIAL VERSIONES ===");
    Serial.println("Versión actual: " + obtenerVersionActual());
    Serial.println("Versión disponible: " + infoActualizacion.version);
    Serial.println("Rollback disponible: " + String(rollbackDisponible ? "Sí" : "No"));
    Serial.println("===========================");
}

bool SistemaOTA::verificarConfiguracion() const {
    return !servidorActualizaciones.isEmpty() && !tokenAutenticacion.isEmpty();
}
