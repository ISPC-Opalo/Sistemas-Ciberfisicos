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
