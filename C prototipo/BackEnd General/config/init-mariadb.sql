-- Script de inicialización para MariaDB - GASLYT
-- Creación de tablas para gestión de dispositivos, clientes, productos, técnicos, etc.

USE opalodb;

-- Tabla de clientes
CREATE TABLE IF NOT EXISTS clientes (
    id INT AUTO_INCREMENT PRIMARY KEY,
    nombre VARCHAR(255) NOT NULL,
    email VARCHAR(255) UNIQUE,
    telefono VARCHAR(50),
    direccion TEXT,
    ciudad VARCHAR(100),
    codigo_postal VARCHAR(20),
    pais VARCHAR(100) DEFAULT 'Argentina',
    fecha_registro TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    activo BOOLEAN DEFAULT TRUE,
    notas TEXT,
    INDEX idx_email (email),
    INDEX idx_activo (activo)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de técnicos
CREATE TABLE IF NOT EXISTS tecnicos (
    id INT AUTO_INCREMENT PRIMARY KEY,
    nombre VARCHAR(255) NOT NULL,
    apellido VARCHAR(255) NOT NULL,
    email VARCHAR(255) UNIQUE,
    telefono VARCHAR(50),
    especialidad VARCHAR(255),
    certificaciones TEXT,
    fecha_ingreso DATE,
    activo BOOLEAN DEFAULT TRUE,
    costo_hora DECIMAL(10,2),
    zona_cobertura TEXT,
    INDEX idx_email (email),
    INDEX idx_activo (activo)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de productos
CREATE TABLE IF NOT EXISTS productos (
    id INT AUTO_INCREMENT PRIMARY KEY,
    codigo VARCHAR(100) UNIQUE NOT NULL,
    nombre VARCHAR(255) NOT NULL,
    descripcion TEXT,
    categoria VARCHAR(100),
    marca VARCHAR(100),
    modelo VARCHAR(100),
    precio_compra DECIMAL(10,2),
    precio_venta DECIMAL(10,2),
    stock_minimo INT DEFAULT 0,
    stock_actual INT DEFAULT 0,
    unidad_medida VARCHAR(50) DEFAULT 'unidad',
    activo BOOLEAN DEFAULT TRUE,
    fecha_creacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_codigo (codigo),
    INDEX idx_categoria (categoria),
    INDEX idx_activo (activo)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de repuestos
CREATE TABLE IF NOT EXISTS repuestos (
    id INT AUTO_INCREMENT PRIMARY KEY,
    codigo VARCHAR(100) UNIQUE NOT NULL,
    nombre VARCHAR(255) NOT NULL,
    descripcion TEXT,
    categoria VARCHAR(100),
    marca VARCHAR(100),
    modelo VARCHAR(100),
    precio_compra DECIMAL(10,2),
    precio_venta DECIMAL(10,2),
    stock_minimo INT DEFAULT 0,
    stock_actual INT DEFAULT 0,
    unidad_medida VARCHAR(50) DEFAULT 'unidad',
    compatibilidad TEXT,
    activo BOOLEAN DEFAULT TRUE,
    fecha_creacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_codigo (codigo),
    INDEX idx_categoria (categoria),
    INDEX idx_activo (activo)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de dispositivos GASLYT
CREATE TABLE IF NOT EXISTS dispositivos (
    id INT AUTO_INCREMENT PRIMARY KEY,
    id_dispositivo VARCHAR(100) UNIQUE NOT NULL,
    mac_address VARCHAR(17) UNIQUE,
    cliente_id INT,
    ubicacion VARCHAR(255),
    direccion_instalacion TEXT,
    fecha_instalacion DATE,
    fecha_ultima_calibracion DATE,
    fecha_proxima_calibracion DATE,
    tecnico_instalador_id INT,
    tecnico_mantenimiento_id INT,
    estado VARCHAR(50) DEFAULT 'activo',
    version_firmware VARCHAR(50),
    version_certificados VARCHAR(50),
    configuracion JSON,
    notas TEXT,
    fecha_registro TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    fecha_actualizacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (cliente_id) REFERENCES clientes(id) ON DELETE SET NULL,
    FOREIGN KEY (tecnico_instalador_id) REFERENCES tecnicos(id) ON DELETE SET NULL,
    FOREIGN KEY (tecnico_mantenimiento_id) REFERENCES tecnicos(id) ON DELETE SET NULL,
    INDEX idx_id_dispositivo (id_dispositivo),
    INDEX idx_cliente (cliente_id),
    INDEX idx_estado (estado)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de configuraciones de dispositivos
CREATE TABLE IF NOT EXISTS configuraciones_dispositivos (
    id INT AUTO_INCREMENT PRIMARY KEY,
    dispositivo_id INT NOT NULL,
    parametro VARCHAR(100) NOT NULL,
    valor VARCHAR(500),
    valor_anterior VARCHAR(500),
    fecha_cambio TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    usuario_cambio VARCHAR(100),
    motivo_cambio TEXT,
    FOREIGN KEY (dispositivo_id) REFERENCES dispositivos(id) ON DELETE CASCADE,
    INDEX idx_dispositivo (dispositivo_id),
    INDEX idx_parametro (parametro),
    INDEX idx_fecha (fecha_cambio)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de actualizaciones de firmware
CREATE TABLE IF NOT EXISTS actualizaciones_firmware (
    id INT AUTO_INCREMENT PRIMARY KEY,
    dispositivo_id INT NOT NULL,
    version_anterior VARCHAR(50),
    version_nueva VARCHAR(50) NOT NULL,
    url_descarga VARCHAR(500),
    hash_verificacion VARCHAR(255),
    tamaño_archivo BIGINT,
    descripcion TEXT,
    critica BOOLEAN DEFAULT FALSE,
    estado VARCHAR(50) DEFAULT 'pendiente',
    fecha_solicitud TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    fecha_inicio TIMESTAMP NULL,
    fecha_completado TIMESTAMP NULL,
    fecha_rollback TIMESTAMP NULL,
    error_mensaje TEXT,
    FOREIGN KEY (dispositivo_id) REFERENCES dispositivos(id) ON DELETE CASCADE,
    INDEX idx_dispositivo (dispositivo_id),
    INDEX idx_estado (estado),
    INDEX idx_fecha_solicitud (fecha_solicitud)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de actualizaciones de certificados
CREATE TABLE IF NOT EXISTS actualizaciones_certificados (
    id INT AUTO_INCREMENT PRIMARY KEY,
    dispositivo_id INT NOT NULL,
    version_anterior VARCHAR(50),
    version_nueva VARCHAR(50) NOT NULL,
    endpoint VARCHAR(255),
    hash_verificacion VARCHAR(255),
    estado VARCHAR(50) DEFAULT 'pendiente',
    fecha_solicitud TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    fecha_inicio TIMESTAMP NULL,
    fecha_completado TIMESTAMP NULL,
    fecha_rollback TIMESTAMP NULL,
    error_mensaje TEXT,
    FOREIGN KEY (dispositivo_id) REFERENCES dispositivos(id) ON DELETE CASCADE,
    INDEX idx_dispositivo (dispositivo_id),
    INDEX idx_estado (estado),
    INDEX idx_fecha_solicitud (fecha_solicitud)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de mantenimientos
CREATE TABLE IF NOT EXISTS mantenimientos (
    id INT AUTO_INCREMENT PRIMARY KEY,
    dispositivo_id INT NOT NULL,
    tecnico_id INT NOT NULL,
    tipo_mantenimiento VARCHAR(100) NOT NULL,
    fecha_programada DATE,
    fecha_realizada DATE,
    duracion_minutos INT,
    descripcion TEXT,
    repuestos_utilizados JSON,
    costo_repuestos DECIMAL(10,2) DEFAULT 0,
    costo_mano_obra DECIMAL(10,2) DEFAULT 0,
    costo_total DECIMAL(10,2) DEFAULT 0,
    estado VARCHAR(50) DEFAULT 'programado',
    observaciones TEXT,
    fecha_creacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (dispositivo_id) REFERENCES dispositivos(id) ON DELETE CASCADE,
    FOREIGN KEY (tecnico_id) REFERENCES tecnicos(id) ON DELETE CASCADE,
    INDEX idx_dispositivo (dispositivo_id),
    INDEX idx_tecnico (tecnico_id),
    INDEX idx_fecha_programada (fecha_programada),
    INDEX idx_estado (estado)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de inventario de repuestos
CREATE TABLE IF NOT EXISTS inventario_repuestos (
    id INT AUTO_INCREMENT PRIMARY KEY,
    repuesto_id INT NOT NULL,
    cantidad_entrada INT NOT NULL,
    cantidad_salida INT DEFAULT 0,
    stock_actual INT NOT NULL,
    fecha_movimiento TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    tipo_movimiento ENUM('entrada', 'salida', 'ajuste') NOT NULL,
    motivo VARCHAR(255),
    referencia VARCHAR(255),
    costo_unitario DECIMAL(10,2),
    FOREIGN KEY (repuesto_id) REFERENCES repuestos(id) ON DELETE CASCADE,
    INDEX idx_repuesto (repuesto_id),
    INDEX idx_fecha (fecha_movimiento),
    INDEX idx_tipo (tipo_movimiento)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de costos y precios
CREATE TABLE IF NOT EXISTS costos_productos (
    id INT AUTO_INCREMENT PRIMARY KEY,
    producto_id INT,
    repuesto_id INT,
    costo_compra DECIMAL(10,2) NOT NULL,
    precio_venta DECIMAL(10,2) NOT NULL,
    margen_ganancia DECIMAL(5,2),
    fecha_vigencia DATE,
    proveedor VARCHAR(255),
    activo BOOLEAN DEFAULT TRUE,
    fecha_creacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (producto_id) REFERENCES productos(id) ON DELETE CASCADE,
    FOREIGN KEY (repuesto_id) REFERENCES repuestos(id) ON DELETE CASCADE,
    INDEX idx_producto (producto_id),
    INDEX idx_repuesto (repuesto_id),
    INDEX idx_fecha_vigencia (fecha_vigencia),
    INDEX idx_activo (activo)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de logs del sistema
CREATE TABLE IF NOT EXISTS logs_sistema (
    id INT AUTO_INCREMENT PRIMARY KEY,
    dispositivo_id INT,
    nivel VARCHAR(20) NOT NULL,
    componente VARCHAR(100),
    mensaje TEXT NOT NULL,
    timestamp_log TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ip_origen VARCHAR(45),
    usuario VARCHAR(100),
    FOREIGN KEY (dispositivo_id) REFERENCES dispositivos(id) ON DELETE SET NULL,
    INDEX idx_dispositivo (dispositivo_id),
    INDEX idx_nivel (nivel),
    INDEX idx_componente (componente),
    INDEX idx_timestamp (timestamp_log)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Insertar datos iniciales
INSERT INTO clientes (nombre, email, telefono, direccion, ciudad) VALUES
('Cliente Demo', 'demo@ejemplo.com', '+54 11 1234-5678', 'Av. Corrientes 1234', 'Buenos Aires'),
('Empresa Test', 'test@empresa.com', '+54 11 9876-5432', 'Av. Santa Fe 5678', 'Buenos Aires');

INSERT INTO tecnicos (nombre, apellido, email, telefono, especialidad, costo_hora) VALUES
('Juan', 'Pérez', 'juan.perez@tecnico.com', '+54 11 1111-1111', 'Sensores de Gas', 2500.00),
('María', 'González', 'maria.gonzalez@tecnico.com', '+54 11 2222-2222', 'Sistemas IoT', 2800.00);

INSERT INTO productos (codigo, nombre, descripcion, categoria, precio_compra, precio_venta, stock_actual) VALUES
('GASLYT-001', 'Sensor GASLYT Básico', 'Sensor de gas inflamable con comunicación MQTT', 'Sensores', 15000.00, 25000.00, 10),
('GASLYT-002', 'Sensor GASLYT Premium', 'Sensor de gas con extractor integrado', 'Sensores', 25000.00, 40000.00, 5);

INSERT INTO repuestos (codigo, nombre, descripcion, categoria, precio_compra, precio_venta, stock_actual) VALUES
('REP-001', 'Sensor MQ-2', 'Sensor de gas MQ-2 de repuesto', 'Sensores', 5000.00, 8000.00, 20),
('REP-002', 'Extractor 12V', 'Extractor de aire 12V para GASLYT', 'Electrónica', 3000.00, 5000.00, 15),
('REP-003', 'Cable de Alimentación', 'Cable de alimentación 12V 2m', 'Cables', 500.00, 1000.00, 50);

-- Crear vistas útiles
CREATE VIEW vista_dispositivos_completa AS
SELECT 
    d.id,
    d.id_dispositivo,
    d.mac_address,
    c.nombre as cliente_nombre,
    c.email as cliente_email,
    d.ubicacion,
    d.estado,
    d.version_firmware,
    d.fecha_instalacion,
    ti.nombre as tecnico_instalador,
    tm.nombre as tecnico_mantenimiento
FROM dispositivos d
LEFT JOIN clientes c ON d.cliente_id = c.id
LEFT JOIN tecnicos ti ON d.tecnico_instalador_id = ti.id
LEFT JOIN tecnicos tm ON d.tecnico_mantenimiento_id = tm.id;

CREATE VIEW vista_stock_bajo AS
SELECT 
    codigo,
    nombre,
    stock_actual,
    stock_minimo,
    (stock_actual - stock_minimo) as diferencia
FROM productos 
WHERE stock_actual <= stock_minimo
UNION
SELECT 
    codigo,
    nombre,
    stock_actual,
    stock_minimo,
    (stock_actual - stock_minimo) as diferencia
FROM repuestos 
WHERE stock_actual <= stock_minimo;

-- ========================================
-- TABLAS PARA BLOCKCHAIN PROCESSOR GASLYT
-- ========================================

-- Tabla de transacciones blockchain
CREATE TABLE IF NOT EXISTS blockchain_transactions (
    id INT AUTO_INCREMENT PRIMARY KEY,
    tx_hash VARCHAR(66) UNIQUE NOT NULL,
    block_number BIGINT NULL,
    gas_used BIGINT NOT NULL,
    gas_price BIGINT NOT NULL,
    gas_cost_eth FLOAT NOT NULL,
    confirmation_time INT NULL COMMENT 'segundos para confirmar',
    status VARCHAR(20) NOT NULL DEFAULT 'pending' COMMENT 'pending, confirmed, failed',
    retry_count INT DEFAULT 0,
    error_message TEXT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    confirmed_at TIMESTAMP NULL,
    INDEX idx_tx_hash (tx_hash),
    INDEX idx_status (status),
    INDEX idx_created_at (created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de registros de alarmas en blockchain
CREATE TABLE IF NOT EXISTS blockchain_registry (
    id INT AUTO_INCREMENT PRIMARY KEY,
    tx_hash VARCHAR(66) NOT NULL,
    fecha_registro TIMESTAMP NOT NULL,
    id_dispositivo VARCHAR(100) NOT NULL,
    id_cliente VARCHAR(100) NOT NULL,
    cantidad_alarmas INT NOT NULL,
    concentracion_maxima FLOAT NOT NULL,
    datos_hash VARCHAR(66) NOT NULL COMMENT 'Hash de los datos',
    datos_json JSON NULL COMMENT 'Datos completos para verificación',
    verificado BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_tx_hash (tx_hash),
    INDEX idx_fecha_registro (fecha_registro),
    INDEX idx_dispositivo_fecha (id_dispositivo, fecha_registro),
    INDEX idx_cliente_fecha (id_cliente, fecha_registro)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de métricas de la blockchain
CREATE TABLE IF NOT EXISTS blockchain_metrics (
    id INT AUTO_INCREMENT PRIMARY KEY,
    fecha_metrica TIMESTAMP NOT NULL,
    network_name VARCHAR(50) NOT NULL,
    
    -- Métricas de transacciones
    tx_count_total INT DEFAULT 0,
    tx_count_pending INT DEFAULT 0,
    tx_count_confirmed INT DEFAULT 0,
    tx_count_failed INT DEFAULT 0,
    
    -- Métricas de gas
    gas_price_avg FLOAT NULL,
    gas_price_min FLOAT NULL,
    gas_price_max FLOAT NULL,
    gas_used_total BIGINT DEFAULT 0,
    gas_cost_total_eth FLOAT DEFAULT 0.0,
    
    -- Métricas de rendimiento
    avg_confirmation_time FLOAT NULL COMMENT 'segundos',
    blocks_mined INT DEFAULT 0,
    network_difficulty BIGINT NULL,
    
    -- Métricas de negocio
    dispositivos_registrados INT DEFAULT 0,
    alarmas_registradas INT DEFAULT 0,
    
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_fecha_network (fecha_metrica, network_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de salud de la red blockchain
CREATE TABLE IF NOT EXISTS network_health (
    id INT AUTO_INCREMENT PRIMARY KEY,
    fecha_check TIMESTAMP NOT NULL,
    network_name VARCHAR(50) NOT NULL,
    
    -- Conectividad
    rpc_endpoint_status VARCHAR(20) NOT NULL COMMENT 'up, down, slow',
    rpc_response_time FLOAT NULL COMMENT 'ms',
    websocket_status VARCHAR(20) NULL,
    
    -- Estado de la red
    latest_block BIGINT NULL,
    block_time_avg FLOAT NULL COMMENT 'segundos',
    network_hashrate VARCHAR(50) NULL,
    
    -- Sincronización
    is_synced BOOLEAN NULL,
    sync_percentage FLOAT NULL,
    
    -- Configuración de transacciones
    recommended_gas_price BIGINT NULL,
    max_gas_limit BIGINT NULL,
    
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_fecha_check (fecha_check)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de errores del procesador blockchain
CREATE TABLE IF NOT EXISTS blockchain_errors (
    id INT AUTO_INCREMENT PRIMARY KEY,
    error_uuid VARCHAR(36) UNIQUE DEFAULT (UUID()),
    fecha_ocurrencia TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    
    -- Información del error
    error_type VARCHAR(100) NOT NULL COMMENT 'connection, gas, contract, etc',
    error_message TEXT NOT NULL,
    error_code VARCHAR(20) NULL,
    stack_trace TEXT NULL,
    
    -- Contexto del error
    tx_hash VARCHAR(66) NULL,
    block_number BIGINT NULL,
    gas_price BIGINT NULL,
    retry_attempt INT DEFAULT 0,
    
    -- Resolución
    resolved BOOLEAN DEFAULT FALSE,
    resolution_method VARCHAR(100) NULL,
    resolved_at TIMESTAMP NULL,
    
    -- Información adicional
    client_ip VARCHAR(45) NULL,
    user_agent VARCHAR(255) NULL,
    extra_data JSON NULL,
    
    INDEX idx_error_type (error_type),
    INDEX idx_fecha_ocurrencia (fecha_ocurrencia),
    INDEX idx_resolved (resolved)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de lotes de procesamiento
CREATE TABLE IF NOT EXISTS processing_batches (
    id INT AUTO_INCREMENT PRIMARY KEY,
    batch_uuid VARCHAR(36) UNIQUE DEFAULT (UUID()),
    fecha_procesamiento TIMESTAMP NOT NULL,
    
    -- Información del lote
    registros_count INT NOT NULL,
    batch_size INT NOT NULL,
    fecha_datos TIMESTAMP NOT NULL,
    
    -- Resultados
    tx_hashes JSON NULL COMMENT 'Lista de hashes de transacciones',
    registros_exitosos INT DEFAULT 0,
    registros_fallidos INT DEFAULT 0,
    
    -- Métricas
    gas_total_usado BIGINT DEFAULT 0,
    gas_costo_total_eth FLOAT DEFAULT 0.0,
    tiempo_procesamiento_segundos FLOAT NULL,
    
    -- Estado
    estado VARCHAR(20) DEFAULT 'processing' COMMENT 'processing, completed, failed',
    
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    completed_at TIMESTAMP NULL,
    INDEX idx_fecha_procesamiento (fecha_procesamiento),
    INDEX idx_estado (estado)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla de configuración de la blockchain
CREATE TABLE IF NOT EXISTS blockchain_config (
    id INT AUTO_INCREMENT PRIMARY KEY,
    config_key VARCHAR(100) UNIQUE NOT NULL,
    config_value LONGTEXT NULL,
    config_type VARCHAR(20) DEFAULT 'string' COMMENT 'string, number, boolean, json',
    description TEXT NULL,
    
    -- Metadatos
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    updated_by VARCHAR(100) NULL,
    
    INDEX idx_config_key (config_key)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ========================================
-- DATOS INICIALES PARA BLOCKCHAIN
-- ========================================

-- Insertar configuración inicial de blockchain
INSERT INTO blockchain_config (config_key, config_value, config_type, description) VALUES
('eth_network', 'sepolia', 'string', 'Red Ethereum configurada'),
('gas_limit_default', '150000', 'number', 'Límite de gas por defecto'),
('batch_size_default', '10', 'number', 'Tamaño de lote por defecto'),
('execution_hour', '23', 'number', 'Hora de ejecución diaria'),
('execution_minute', '59', 'number', 'Minuto de ejecución diaria'),
('max_retries', '3', 'number', 'Máximo número de reintentos'),
('retry_delay', '5', 'number', 'Delay entre reintentos en segundos'),
('log_level', 'INFO', 'string', 'Nivel de logging'),
('log_format', 'json', 'string', 'Formato de logs');

-- ========================================
-- VISTAS ÚTILES PARA BLOCKCHAIN
-- ========================================

-- Vista de transacciones blockchain con estado
CREATE VIEW vista_transacciones_blockchain AS
SELECT 
    bt.id,
    bt.tx_hash,
    bt.status,
    bt.gas_used,
    bt.gas_price,
    bt.gas_cost_eth,
    bt.confirmation_time,
    bt.created_at,
    bt.confirmed_at,
    br.id_dispositivo,
    br.cantidad_alarmas,
    br.concentracion_maxima
FROM blockchain_transactions bt
LEFT JOIN blockchain_registry br ON bt.tx_hash = br.tx_hash
ORDER BY bt.created_at DESC;

-- Vista de métricas de blockchain por día
CREATE VIEW vista_metricas_diarias AS
SELECT 
    DATE(fecha_metrica) as fecha,
    network_name,
    SUM(tx_count_total) as total_transacciones,
    SUM(tx_count_confirmed) as transacciones_confirmadas,
    SUM(tx_count_failed) as transacciones_fallidas,
    AVG(gas_price_avg) as precio_gas_promedio,
    SUM(gas_cost_total_eth) as costo_total_eth,
    AVG(avg_confirmation_time) as tiempo_confirmacion_promedio
FROM blockchain_metrics
GROUP BY DATE(fecha_metrica), network_name
ORDER BY fecha DESC;

-- Vista de dispositivos con más alarmas registradas
CREATE VIEW vista_dispositivos_alarmas AS
SELECT 
    id_dispositivo,
    COUNT(*) as total_registros,
    SUM(cantidad_alarmas) as total_alarmas,
    MAX(concentracion_maxima) as concentracion_maxima_historica,
    MAX(fecha_registro) as ultimo_registro,
    MIN(fecha_registro) as primer_registro
FROM blockchain_registry
GROUP BY id_dispositivo
ORDER BY total_alarmas DESC;

-- Vista de salud de la red reciente
CREATE VIEW vista_salud_red_reciente AS
SELECT 
    network_name,
    rpc_endpoint_status,
    latest_block,
    block_time_avg,
    is_synced,
    recommended_gas_price,
    fecha_check
FROM network_health
WHERE fecha_check >= DATE_SUB(NOW(), INTERVAL 24 HOUR)
ORDER BY fecha_check DESC;

-- Vista de errores no resueltos
CREATE VIEW vista_errores_pendientes AS
SELECT 
    error_type,
    COUNT(*) as cantidad_errores,
    MAX(fecha_ocurrencia) as ultimo_error,
    MIN(fecha_ocurrencia) as primer_error
FROM blockchain_errors
WHERE resolved = FALSE
GROUP BY error_type
ORDER BY cantidad_errores DESC;

-- ========================================
-- TRIGGERS PARA BLOCKCHAIN
-- ========================================

-- Trigger para actualizar métricas cuando se confirma una transacción
DELIMITER //

CREATE TRIGGER actualizar_metricas_transaccion_confirmada
AFTER UPDATE ON blockchain_transactions
FOR EACH ROW
BEGIN
    IF NEW.status = 'confirmed' AND OLD.status != 'confirmed' THEN
        -- Actualizar métricas del día
        INSERT INTO blockchain_metrics (
            fecha_metrica, network_name, tx_count_confirmed, 
            gas_used_total, gas_cost_total_eth, avg_confirmation_time
        ) VALUES (
            CURDATE(), 'sepolia', 1, NEW.gas_used, NEW.gas_cost_eth, NEW.confirmation_time
        ) ON DUPLICATE KEY UPDATE
            tx_count_confirmed = tx_count_confirmed + 1,
            gas_used_total = gas_used_total + NEW.gas_used,
            gas_cost_total_eth = gas_cost_total_eth + NEW.gas_cost_eth,
            avg_confirmation_time = (avg_confirmation_time + NEW.confirmation_time) / 2;
    END IF;
END//

-- Trigger para actualizar métricas cuando falla una transacción
CREATE TRIGGER actualizar_metricas_transaccion_fallida
AFTER UPDATE ON blockchain_transactions
FOR EACH ROW
BEGIN
    IF NEW.status = 'failed' AND OLD.status != 'failed' THEN
        -- Actualizar métricas del día
        INSERT INTO blockchain_metrics (
            fecha_metrica, network_name, tx_count_failed
        ) VALUES (
            CURDATE(), 'sepolia', 1
        ) ON DUPLICATE KEY UPDATE
            tx_count_failed = tx_count_failed + 1;
    END IF;
END//

-- Trigger para actualizar contador de alarmas registradas
CREATE TRIGGER actualizar_contador_alarmas_registradas
AFTER INSERT ON blockchain_registry
FOR EACH ROW
BEGIN
    -- Actualizar métricas del día
    INSERT INTO blockchain_metrics (
        fecha_metrica, network_name, alarmas_registradas
    ) VALUES (
        CURDATE(), 'sepolia', NEW.cantidad_alarmas
    ) ON DUPLICATE KEY UPDATE
        alarmas_registradas = alarmas_registradas + NEW.cantidad_alarmas;
END//

DELIMITER ;

-- ========================================
-- TRIGGERS ORIGINALES PARA STOCK
-- ========================================

-- Crear triggers para actualizar stock automáticamente
DELIMITER //

CREATE TRIGGER actualizar_stock_productos_after_insert
AFTER INSERT ON inventario_repuestos
FOR EACH ROW
BEGIN
    UPDATE repuestos 
    SET stock_actual = stock_actual + NEW.cantidad_entrada - NEW.cantidad_salida
    WHERE id = NEW.repuesto_id;
END//

CREATE TRIGGER actualizar_stock_productos_after_update
AFTER UPDATE ON inventario_repuestos
FOR EACH ROW
BEGIN
    UPDATE repuestos 
    SET stock_actual = stock_actual - OLD.cantidad_entrada + OLD.cantidad_salida + NEW.cantidad_entrada - NEW.cantidad_salida
    WHERE id = NEW.repuesto_id;
END//

DELIMITER ;
