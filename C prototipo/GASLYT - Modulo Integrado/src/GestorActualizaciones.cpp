#include "GestorActualizaciones.h"

GestorActualizaciones::GestorActualizaciones() : 
    certificadosManager(nullptr), sistemaOTA(nullptr), mqttManager(nullptr), logger(nullptr),
    actualizacionesAutomaticas(false), ultimaVerificacion(0), intervaloVerificacion(3600000), // 1 hora
    inicializado(false), actualizacionEnProgreso(false), idDispositivo(""),
    callbackActualizacionCompletada(nullptr), callbackErrorActualizacion(nullptr) {
    
    servidorActualizaciones = "";
    tokenAutenticacion = "";
    topicCertificados = "";
    topicFirmware = "";
    topicEstado = "";
    topicProgreso = "";
    topicConfirmacion = "";
}

GestorActualizaciones::~GestorActualizaciones() {
    // Limpiar recursos
}

bool GestorActualizaciones::inicializar(CertificadosManager* cert, SistemaOTA* ota, 
                                       MQTTManager* mqtt, SistemaLogging* log) {
    if (!cert || !ota || !mqtt || !log) {
        Serial.println("Error: Componentes del gestor de actualizaciones no válidos");
        return false;
    }
    
    certificadosManager = cert;
    sistemaOTA = ota;
    mqttManager = mqtt;
    logger = log;
    
    // Configurar topics
    configurarTopics(mqttManager->obtenerIdDispositivo());
    
    // Configurar callbacks del sistema OTA
    sistemaOTA->establecerCallbackProgreso([](int progreso) {
        // Este callback se configurará desde el contexto del gestor
    });
    
    sistemaOTA->establecerCallbackEstado([](SistemaOTA::EstadoOTA estado) {
        // Este callback se configurará desde el contexto del gestor
    });
    
    sistemaOTA->establecerCallbackError([](const String& error) {
        // Este callback se configurará desde el contexto del gestor
    });
    
    inicializado = true;
    logger->info("ACTUALIZACIONES", "Gestor de actualizaciones inicializado correctamente");
    
    return true;
}

void GestorActualizaciones::configurarTopics(const String& idDispositivo) {
    this->idDispositivo = idDispositivo;
    
    topicCertificados = "/" + idDispositivo + "/actualizaciones/certificados";
    topicFirmware = "/" + idDispositivo + "/actualizaciones/firmware";
    topicEstado = "/" + idDispositivo + "/actualizaciones/estado";
    topicProgreso = "/" + idDispositivo + "/actualizaciones/progreso";
    topicConfirmacion = "/" + idDispositivo + "/actualizaciones/confirmacion";
    
    logger->info("ACTUALIZACIONES", "Topics configurados para dispositivo: " + idDispositivo);
}

void GestorActualizaciones::verificarActualizacionesPeriodicas() {
    if (!inicializado || !actualizacionesAutomaticas) {
        return;
    }
    
    unsigned long tiempoActual = millis();
    if (tiempoActual - ultimaVerificacion >= intervaloVerificacion) {
        ultimaVerificacion = tiempoActual;
        
        logger->info("ACTUALIZACIONES", "Verificando actualizaciones periódicas...");
        
        // Verificar actualizaciones de certificados
        if (verificarCertificadosRemoto()) {
            logger->info("ACTUALIZACIONES", "Actualización de certificados disponible");
        }
        
        // Verificar actualizaciones de firmware
        if (sistemaOTA->verificarActualizacionesDisponibles()) {
            logger->info("ACTUALIZACIONES", "Actualización de firmware disponible");
        }
    }
}

bool GestorActualizaciones::procesarComandoActualizacion(const JsonObject& comando) {
    if (!comando.containsKey("comando")) {
        logger->error("ACTUALIZACIONES", "Comando sin tipo especificado");
        return false;
    }
    
    String tipoComando = comando["comando"];
    logger->info("ACTUALIZACIONES", "Procesando comando: " + tipoComando);
    
    if (tipoComando == "actualizar_certificados") {
        return procesarComandoCertificados(comando);
    } else if (tipoComando == "actualizar_firmware") {
        return procesarComandoFirmware(comando);
    } else if (tipoComando == "verificar_certificados") {
        return verificarCertificadosRemoto();
    } else if (tipoComando == "verificar_firmware") {
        return sistemaOTA->verificarActualizacionesDisponibles();
    } else if (tipoComando == "rollback") {
        return sistemaOTA->hacerRollback();
    } else {
        logger->error("ACTUALIZACIONES", "Comando desconocido: " + tipoComando);
        return false;
    }
}

void GestorActualizaciones::procesarMensajeMQTT(const String& topic, const String& payload) {
    if (topic.endsWith("/certificados")) {
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) {
            logger->error("ACTUALIZACIONES", "Error al parsear comando de certificados");
            return;
        }
        
        procesarComandoCertificados(doc.as<JsonObject>());
    } else if (topic.endsWith("/firmware")) {
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) {
            logger->error("ACTUALIZACIONES", "Error al parsear comando de firmware");
            return;
        }
        
        procesarComandoFirmware(doc.as<JsonObject>());
    }
}

bool GestorActualizaciones::actualizarCertificadosRemoto(const JsonObject& datos) {
    if (!validarDatosCertificados(datos)) {
        logger->error("ACTUALIZACIONES", "Datos de certificados inválidos");
        return false;
    }
    
    logger->info("ACTUALIZACIONES", "Iniciando actualización de certificados...");
    
    actualizacionEnProgreso = true;
    enviarNotificacionEstado("CERTIFICADOS_ACTUALIZANDO", "Actualizando certificados...");
    
    // Extraer datos
    String certificado = datos["certificado"];
    String clavePrivada = datos["clave_privada"];
    String certificadoCA = datos["certificado_ca"];
    String endpoint = datos["endpoint"];
    String version = datos["version"];
    String hash = datos["hash"];
    String firma = datos["firma"];
    
    // Verificar firma digital
    if (!firma.isEmpty()) {
        String datosParaVerificar = certificado + clavePrivada + certificadoCA + endpoint + version;
        if (!certificadosManager->verificarFirmaDigital(datosParaVerificar, firma)) {
            logger->error("ACTUALIZACIONES", "Firma digital de certificados inválida");
            actualizacionEnProgreso = false;
            enviarNotificacionError("Firma digital inválida", "certificados");
            return false;
        }
    }
    
    // Actualizar certificados
    bool exito = certificadosManager->actualizarCertificados(certificado, clavePrivada, certificadoCA, endpoint, version);
    
    if (exito) {
        logger->info("ACTUALIZACIONES", "Certificados actualizados exitosamente");
        enviarNotificacionEstado("CERTIFICADOS_ACTUALIZADOS", "Certificados actualizados correctamente");
        
        if (callbackActualizacionCompletada) {
            callbackActualizacionCompletada("certificados", true);
        }
    } else {
        logger->error("ACTUALIZACIONES", "Error al actualizar certificados");
        enviarNotificacionError("Error al actualizar certificados", "certificados");
        
        if (callbackActualizacionCompletada) {
            callbackActualizacionCompletada("certificados", false);
        }
    }
    
    actualizacionEnProgreso = false;
    return exito;
}

bool GestorActualizaciones::verificarCertificadosRemoto() {
    logger->info("ACTUALIZACIONES", "Verificando certificados remotos...");
    
    // Verificar integridad de certificados actuales
    if (!certificadosManager->verificarIntegridadCertificados()) {
        logger->warning("ACTUALIZACIONES", "Certificados actuales no válidos, restaurando desde backup");
        
        if (certificadosManager->restaurarCertificadosDesdeBackup()) {
            logger->info("ACTUALIZACIONES", "Certificados restaurados desde backup");
            enviarNotificacionEstado("CERTIFICADOS_RESTAURADOS", "Certificados restaurados desde backup");
        } else {
            logger->error("ACTUALIZACIONES", "Error al restaurar certificados desde backup");
            enviarNotificacionError("Error al restaurar certificados", "certificados");
            return false;
        }
    }
    
    notificarEstadoCertificados();
    return true;
}

void GestorActualizaciones::notificarEstadoCertificados() {
    DynamicJsonDocument doc(512);
    doc["tipo"] = "certificados";
    doc["version"] = certificadosManager->obtenerVersionCertificados();
    doc["valido"] = certificadosManager->esValido();
    doc["timestamp"] = certificadosManager->obtenerTimestampCertificados();
    doc["hash"] = certificadosManager->obtenerHash();
    doc["endpoint"] = certificadosManager->obtenerEndpoint();
    
    String payload;
    serializeJson(doc, payload);
    
    if (mqttManager->publicarMetadata(doc.as<JsonObject>())) {
        logger->info("ACTUALIZACIONES", "Estado de certificados enviado");
    } else {
        logger->error("ACTUALIZACIONES", "Error al enviar estado de certificados");
    }
}

bool GestorActualizaciones::procesarComandoCertificados(const JsonObject& comando) {
    if (!validarComandoCertificados(comando)) {
        logger->error("ACTUALIZACIONES", "Comando de certificados inválido");
        return false;
    }
    
    String tipoComando = comando["comando"];
    
    if (tipoComando == "actualizar_certificados") {
        return actualizarCertificadosRemoto(comando);
    } else if (tipoComando == "verificar_certificados") {
        return verificarCertificadosRemoto();
    } else {
        logger->error("ACTUALIZACIONES", "Comando de certificados desconocido: " + tipoComando);
        return false;
    }
}

bool GestorActualizaciones::actualizarFirmwareRemoto(const JsonObject& datos) {
    if (!validarDatosFirmware(datos)) {
        logger->error("ACTUALIZACIONES", "Datos de firmware inválidos");
        return false;
    }
    
    logger->info("ACTUALIZACIONES", "Iniciando actualización de firmware...");
    
    actualizacionEnProgreso = true;
    enviarNotificacionEstado("FIRMWARE_ACTUALIZANDO", "Actualizando firmware...");
    
    // Extraer datos
    String version = datos["version"];
    String url = datos["url"];
    String hash = datos["hash"];
    String firma = datos["firma"];
    bool critica = datos["critica"];
    
    // Verificar firma digital
    if (!firma.isEmpty()) {
        String datosParaVerificar = version + url + hash;
        if (!sistemaOTA->verificarFirmaDigital(datosParaVerificar, firma)) {
            logger->error("ACTUALIZACIONES", "Firma digital de firmware inválida");
            actualizacionEnProgreso = false;
            enviarNotificacionError("Firma digital inválida", "firmware");
            return false;
        }
    }
    
    // Configurar callbacks para progreso
    sistemaOTA->establecerCallbackProgreso([this](int progreso) {
        enviarNotificacionProgreso(progreso, "Descargando firmware...");
    });
    
    sistemaOTA->establecerCallbackEstado([this](SistemaOTA::EstadoOTA estado) {
        String estadoStr = "";
        switch (estado) {
            case SistemaOTA::OTA_DESCARGANDO: estadoStr = "Descargando"; break;
            case SistemaOTA::OTA_INSTALANDO: estadoStr = "Instalando"; break;
            case SistemaOTA::OTA_COMPLETADO: estadoStr = "Completado"; break;
            case SistemaOTA::OTA_ERROR: estadoStr = "Error"; break;
            default: estadoStr = "Desconocido"; break;
        }
        enviarNotificacionEstado("FIRMWARE_" + estadoStr.toUpperCase(), estadoStr);
    });
    
    sistemaOTA->establecerCallbackError([this](const String& error) {
        enviarNotificacionError(error, "firmware");
    });
    
    // Descargar e instalar firmware
    bool exito = false;
    if (sistemaOTA->descargarActualizacion(url)) {
        exito = sistemaOTA->instalarActualizacion();
    }
    
    if (exito) {
        logger->info("ACTUALIZACIONES", "Firmware actualizado exitosamente");
        enviarNotificacionEstado("FIRMWARE_ACTUALIZADO", "Firmware actualizado correctamente");
        
        if (callbackActualizacionCompletada) {
            callbackActualizacionCompletada("firmware", true);
        }
    } else {
        logger->error("ACTUALIZACIONES", "Error al actualizar firmware");
        enviarNotificacionError("Error al actualizar firmware", "firmware");
        
        if (callbackActualizacionCompletada) {
            callbackActualizacionCompletada("firmware", false);
        }
    }
    
    actualizacionEnProgreso = false;
    return exito;
}

bool GestorActualizaciones::verificarFirmwareRemoto() {
    logger->info("ACTUALIZACIONES", "Verificando firmware remoto...");
    
    bool actualizacionDisponible = sistemaOTA->verificarActualizacionesDisponibles();
    
    if (actualizacionDisponible) {
        logger->info("ACTUALIZACIONES", "Actualización de firmware disponible: " + sistemaOTA->obtenerVersionDisponible());
        enviarNotificacionEstado("FIRMWARE_DISPONIBLE", "Actualización disponible: " + sistemaOTA->obtenerVersionDisponible());
    } else {
        logger->info("ACTUALIZACIONES", "Firmware actualizado");
        enviarNotificacionEstado("FIRMWARE_ACTUALIZADO", "Firmware actualizado");
    }
    
    notificarEstadoFirmware();
    return actualizacionDisponible;
}

void GestorActualizaciones::notificarEstadoFirmware() {
    DynamicJsonDocument doc(512);
    doc["tipo"] = "firmware";
    doc["version_actual"] = sistemaOTA->obtenerVersionActual();
    doc["version_disponible"] = sistemaOTA->obtenerVersionDisponible();
    doc["rollback_disponible"] = sistemaOTA->esRollbackDisponible();
    doc["espacio_disponible"] = sistemaOTA->obtenerEspacioDisponible();
    doc["inicializado"] = sistemaOTA->esInicializado();
    
    String payload;
    serializeJson(doc, payload);
    
    if (mqttManager->publicarMetadata(doc.as<JsonObject>())) {
        logger->info("ACTUALIZACIONES", "Estado de firmware enviado");
    } else {
        logger->error("ACTUALIZACIONES", "Error al enviar estado de firmware");
    }
}

bool GestorActualizaciones::procesarComandoFirmware(const JsonObject& comando) {
    if (!validarComandoFirmware(comando)) {
        logger->error("ACTUALIZACIONES", "Comando de firmware inválido");
        return false;
    }
    
    String tipoComando = comando["comando"];
    
    if (tipoComando == "actualizar_firmware") {
        return actualizarFirmwareRemoto(comando);
    } else if (tipoComando == "verificar_firmware") {
        return verificarFirmwareRemoto();
    } else if (tipoComando == "rollback") {
        return sistemaOTA->hacerRollback();
    } else {
        logger->error("ACTUALIZACIONES", "Comando de firmware desconocido: " + tipoComando);
        return false;
    }
}

// Configuración
void GestorActualizaciones::establecerServidorActualizaciones(const String& servidor) {
    servidorActualizaciones = servidor;
    sistemaOTA->establecerServidorActualizaciones(servidor);
    logger->info("ACTUALIZACIONES", "Servidor de actualizaciones establecido: " + servidor);
}

void GestorActualizaciones::establecerTokenAutenticacion(const String& token) {
    tokenAutenticacion = token;
    sistemaOTA->establecerTokenAutenticacion(token);
    logger->info("ACTUALIZACIONES", "Token de autenticación establecido");
}

void GestorActualizaciones::habilitarActualizacionesAutomaticas(bool habilitar) {
    actualizacionesAutomaticas = habilitar;
    sistemaOTA->habilitarActualizacionesAutomaticas(habilitar);
    logger->info("ACTUALIZACIONES", "Actualizaciones automáticas: " + String(habilitar ? "Habilitadas" : "Deshabilitadas"));
}

void GestorActualizaciones::establecerIntervaloVerificacion(unsigned long intervalo) {
    intervaloVerificacion = intervalo;
    sistemaOTA->establecerIntervaloVerificacion(intervalo);
    logger->info("ACTUALIZACIONES", "Intervalo de verificación establecido: " + String(intervalo) + " ms");
}

// Callbacks
void GestorActualizaciones::establecerCallbackActualizacionCompletada(void (*callback)(const String&, bool)) {
    callbackActualizacionCompletada = callback;
}

void GestorActualizaciones::establecerCallbackErrorActualizacion(void (*callback)(const String&)) {
    callbackErrorActualizacion = callback;
}

// Getters
bool GestorActualizaciones::esInicializado() const {
    return inicializado;
}

bool GestorActualizaciones::esActualizacionEnProgreso() const {
    return actualizacionEnProgreso;
}

String GestorActualizaciones::obtenerEstadoActual() const {
    if (actualizacionEnProgreso) {
        return "ACTUALIZANDO";
    } else if (sistemaOTA->obtenerEstadoActual() == SistemaOTA::OTA_DISPONIBLE) {
        return "DISPONIBLE";
    } else {
        return "IDLE";
    }
}

String GestorActualizaciones::obtenerProgresoActualizacion() const {
    return String(sistemaOTA->obtenerProgreso()) + "%";
}

unsigned long GestorActualizaciones::obtenerUltimaVerificacion() const {
    return ultimaVerificacion;
}

// Utilidades
void GestorActualizaciones::imprimirEstado() const {
    Serial.println("=== ESTADO GESTOR ACTUALIZACIONES ===");
    Serial.println("Inicializado: " + String(inicializado ? "Sí" : "No"));
    Serial.println("Actualización en progreso: " + String(actualizacionEnProgreso ? "Sí" : "No"));
    Serial.println("Estado actual: " + obtenerEstadoActual());
    Serial.println("Progreso: " + obtenerProgresoActualizacion());
    Serial.println("Actualizaciones automáticas: " + String(actualizacionesAutomaticas ? "Sí" : "No"));
    Serial.println("Última verificación: " + String(ultimaVerificacion));
    Serial.println("Servidor: " + servidorActualizaciones);
    Serial.println("=====================================");
}

void GestorActualizaciones::imprimirConfiguracion() const {
    Serial.println("=== CONFIGURACIÓN GESTOR ACTUALIZACIONES ===");
    Serial.println("ID Dispositivo: " + idDispositivo);
    Serial.println("Servidor: " + servidorActualizaciones);
    Serial.println("Token: " + String(tokenAutenticacion.isEmpty() ? "No configurado" : "Configurado"));
    Serial.println("Intervalo verificación: " + String(intervaloVerificacion) + " ms");
    Serial.println("Topic Certificados: " + topicCertificados);
    Serial.println("Topic Firmware: " + topicFirmware);
    Serial.println("Topic Estado: " + topicEstado);
    Serial.println("Topic Progreso: " + topicProgreso);
    Serial.println("Topic Confirmación: " + topicConfirmacion);
    Serial.println("=============================================");
}

void GestorActualizaciones::resetear() {
    actualizacionEnProgreso = false;
    ultimaVerificacion = 0;
    sistemaOTA->resetear();
    logger->info("ACTUALIZACIONES", "Gestor de actualizaciones reseteado");
}

// Notificaciones MQTT
void GestorActualizaciones::enviarNotificacionEstado(const String& estado, const String& mensaje) {
    DynamicJsonDocument doc(256);
    doc["estado"] = estado;
    doc["mensaje"] = mensaje;
    doc["timestamp"] = millis();
    doc["dispositivo"] = idDispositivo;
    
    String payload;
    serializeJson(doc, payload);
    
    if (mqttManager->publicarMetadata(doc.as<JsonObject>())) {
        logger->info("ACTUALIZACIONES", "Notificación de estado enviada: " + estado);
    }
}

void GestorActualizaciones::enviarNotificacionProgreso(int progreso, const String& descripcion) {
    DynamicJsonDocument doc(256);
    doc["progreso"] = progreso;
    doc["descripcion"] = descripcion;
    doc["timestamp"] = millis();
    doc["dispositivo"] = idDispositivo;
    
    String payload;
    serializeJson(doc, payload);
    
    if (mqttManager->publicarMetadata(doc.as<JsonObject>())) {
        logger->debug("ACTUALIZACIONES", "Progreso: " + String(progreso) + "% - " + descripcion);
    }
}

void GestorActualizaciones::enviarNotificacionConfirmacion(const String& comando, bool exito, const String& mensaje) {
    DynamicJsonDocument doc(256);
    doc["comando"] = comando;
    doc["exito"] = exito;
    doc["mensaje"] = mensaje;
    doc["timestamp"] = millis();
    doc["dispositivo"] = idDispositivo;
    
    String payload;
    serializeJson(doc, payload);
    
    if (mqttManager->publicarMetadata(doc.as<JsonObject>())) {
        logger->info("ACTUALIZACIONES", "Confirmación enviada: " + comando + " - " + String(exito ? "Éxito" : "Error"));
    }
}

void GestorActualizaciones::enviarNotificacionError(const String& error, const String& contexto) {
    DynamicJsonDocument doc(256);
    doc["error"] = error;
    doc["contexto"] = contexto;
    doc["timestamp"] = millis();
    doc["dispositivo"] = idDispositivo;
    
    String payload;
    serializeJson(doc, payload);
    
    if (mqttManager->publicarMetadata(doc.as<JsonObject>())) {
        logger->error("ACTUALIZACIONES", "Error enviado: " + error + " (" + contexto + ")");
    }
}

// Validación de comandos
bool GestorActualizaciones::validarComandoCertificados(const JsonObject& comando) {
    if (!comando.containsKey("comando")) {
        return false;
    }
    
    String comandoStr = comando["comando"];
    if (comandoStr == "actualizar_certificados") {
        return validarDatosCertificados(comando);
    } else if (comandoStr == "verificar_certificados") {
        return true;
    }
    
    return false;
}

bool GestorActualizaciones::validarComandoFirmware(const JsonObject& comando) {
    if (!comando.containsKey("comando")) {
        return false;
    }
    
    String comandoStr = comando["comando"];
    if (comandoStr == "actualizar_firmware") {
        return validarDatosFirmware(comando);
    } else if (comandoStr == "verificar_firmware" || comandoStr == "rollback") {
        return true;
    }
    
    return false;
}

bool GestorActualizaciones::validarDatosCertificados(const JsonObject& datos) {
    return datos.containsKey("certificado") &&
           datos.containsKey("clave_privada") &&
           datos.containsKey("certificado_ca") &&
           datos.containsKey("endpoint") &&
           datos.containsKey("version");
}

bool GestorActualizaciones::validarDatosFirmware(const JsonObject& datos) {
    return datos.containsKey("version") &&
           datos.containsKey("url") &&
           datos.containsKey("hash");
}

// Gestión de errores
void GestorActualizaciones::manejarError(const String& error, const String& contexto) {
    logger->error("ACTUALIZACIONES", "Error en " + contexto + ": " + error);
    enviarNotificacionError(error, contexto);
    
    if (callbackErrorActualizacion) {
        callbackErrorActualizacion(error);
    }
}

void GestorActualizaciones::registrarEvento(const String& evento, const String& detalles) {
    logger->info("ACTUALIZACIONES", "Evento: " + evento + " - " + detalles);
}

void GestorActualizaciones::limpiarErrores() {
    // Limpiar errores pendientes
    logger->info("ACTUALIZACIONES", "Errores limpiados");
}

// Logging y debugging
void GestorActualizaciones::imprimirHistorialActualizaciones() const {
    Serial.println("=== HISTORIAL ACTUALIZACIONES ===");
    Serial.println("Última verificación: " + String(ultimaVerificacion));
    Serial.println("Estado actual: " + obtenerEstadoActual());
    Serial.println("Progreso: " + obtenerProgresoActualizacion());
    Serial.println("================================");
}

void GestorActualizaciones::imprimirEstadisticas() const {
    Serial.println("=== ESTADÍSTICAS ACTUALIZACIONES ===");
    Serial.println("Certificados válidos: " + String(certificadosManager->esValido() ? "Sí" : "No"));
    Serial.println("Versión certificados: " + certificadosManager->obtenerVersionCertificados());
    Serial.println("Versión firmware: " + sistemaOTA->obtenerVersionActual());
    Serial.println("Rollback disponible: " + String(sistemaOTA->esRollbackDisponible() ? "Sí" : "No"));
    Serial.println("Espacio disponible: " + String(sistemaOTA->obtenerEspacioDisponible()) + " bytes");
    Serial.println("===================================");
}

void GestorActualizaciones::verificarConfiguracion() const {
    Serial.println("=== VERIFICACIÓN CONFIGURACIÓN ===");
    Serial.println("Servidor configurado: " + String(servidorActualizaciones.isEmpty() ? "No" : "Sí"));
    Serial.println("Token configurado: " + String(tokenAutenticacion.isEmpty() ? "No" : "Sí"));
    Serial.println("Topics configurados: " + String(topicCertificados.isEmpty() ? "No" : "Sí"));
    Serial.println("Sistema OTA inicializado: " + String(sistemaOTA->esInicializado() ? "Sí" : "No"));
    Serial.println("Certificados inicializados: " + String(certificadosManager->esInicializado() ? "Sí" : "No"));
    Serial.println("===================================");
}
