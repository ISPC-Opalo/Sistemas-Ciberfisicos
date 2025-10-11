// SPDX-License-Identifier: MIT
pragma solidity ^0.8.19;

/*
 * @title   GaslytContrato
 * @dev     Contrato inteligente para registrar alarmas diarias de dispositivos GASLYT
 * @author  Durigutti Vittorio, Lisandro Juncos, Luciano Lujan, Jose Marquez
 * @notice  Este contrato permite registrar datos críticos de dispositivos de monitoreo de gas
 *          con verificación de integridad mediante hash de datos
 */

contract GaslytContrato {
    
    // ============ VARIABLES DE ESTADO ============
    
    /// @dev Dirección del propietario del contrato
    address public owner;
    
    /// @dev Contador de registros totales
    uint256 public totalRegistros;
    
    /// @dev Contador de dispositivos únicos registrados
    uint256 public totalDispositivos;
    
    /// @dev Mapeo para verificar si un dispositivo ya fue registrado en una fecha específica
    mapping(bytes32 => bool) public registrosExistentes;
    
    /// @dev Mapeo para almacenar información de dispositivos
    mapping(string => DispositivoInfo) public dispositivos;
    
    /// @dev Array de todos los registros para consultas
    RegistroAlarma[] public registros;
    
    // ============ ESTRUCTURAS ============
    
    /*
     * @dev Estructura para almacenar información de un dispositivo
     */
    struct DispositivoInfo {
        string idDispositivo;
        string idCliente;
        uint256 fechaPrimerRegistro;
        uint256 totalAlarmas;
        uint256 concentracionMaxima;
        bool activo;
    }
    
    /*
     * @dev Estructura para almacenar un registro de alarma diaria
     */
    struct RegistroAlarma {
        uint256 fecha;
        string idDispositivo;
        string idCliente;
        uint256 cantidadAlarmas;
        uint256 concentracionMax;
        bytes32 hashDatos;
        uint256 timestamp;
        address registradoPor;
    }
    
    // ============ EVENTOS ============
    
    /*
     * @dev Emitido cuando se registra una nueva alarma diaria
     * @param registroId ID único del registro
     * @param idDispositivo ID del dispositivo
     * @param fecha Fecha del registro (timestamp)
     * @param cantidadAlarmas Cantidad de alarmas registradas
     * @param concentracionMax Concentración máxima registrada
     * @param hashDatos Hash de verificación de los datos
     */
    event AlarmaRegistrada(
        bytes32 indexed registroId,
        string indexed idDispositivo,
        uint256 fecha,
        uint256 cantidadAlarmas,
        uint256 concentracionMax,
        bytes32 hashDatos
    );
    
    /**
     * @dev Emitido cuando se registra un nuevo dispositivo
     * @param idDispositivo ID del dispositivo
     * @param idCliente ID del cliente
     * @param fechaRegistro Fecha del primer registro
     */
    event DispositivoRegistrado(
        string indexed idDispositivo,
        string indexed idCliente,
        uint256 fechaRegistro
    );
    
    /**
     * @dev Emitido cuando cambia el propietario del contrato
     * @param nuevoOwner Nueva dirección del propietario
     * @param anteriorOwner Dirección anterior del propietario
     */
    event OwnershipTransferred(
        address indexed nuevoOwner,
        address indexed anteriorOwner
    );
    
    // ============ MODIFICADORES ============
    
    /*
     * @dev Modificador para restringir funciones solo al propietario
     */
    modifier soloPropietario() {
        require(msg.sender == owner, "GaslytContrato: Solo el propietario puede ejecutar esta funcion");
        _;
    }
    
    /*
     * @dev Modificador para validar que los parámetros no estén vacíos
     */
    modifier parametrosValidos(
        string memory _idDispositivo,
        string memory _idCliente,
        uint256 _cantidadAlarmas,
        uint256 _concentracionMax
    ) {
        require(bytes(_idDispositivo).length > 0, "GaslytContrato: ID del dispositivo no puede estar vacio");
        require(bytes(_idCliente).length > 0, "GaslytContrato: ID del cliente no puede estar vacio");
        require(_cantidadAlarmas > 0, "GaslytContrato: La cantidad de alarmas debe ser mayor a 0");
        require(_concentracionMax > 0, "GaslytContrato: La concentracion maxima debe ser mayor a 0");
        _;
    }
    
    // ============ CONSTRUCTOR ============
    
    /*
     * @dev Constructor del contrato
     * @notice Establece el deployer como propietario inicial
     */
    constructor() {
        owner = msg.sender;
        emit OwnershipTransferred(address(0), msg.sender);
    }
    
    // ============ FUNCIONES PRINCIPALES ============
    
    /*
     * @dev Registra una alarma diaria para un dispositivo
     * @param _fecha Fecha del registro (timestamp Unix, ejemplo 20251010 para el 10 oct. 2025)
     * @param _idDispositivo ID único del dispositivo
     * @param _idCliente ID del cliente propietario del dispositivo
     * @param _cantidadAlarmas Cantidad de alarmas registradas en el día
     * @param _concentracionMax Concentración máxima registrada en el día
     * @param _hashDatos Hash SHA-256 de los datos para verificación de integridad
     * @return registroId ID único del registro generado
     */
    function registrarAlarmaDiaria(
        uint256 _fecha,
        string memory _idDispositivo,
        string memory _idCliente,
        uint256 _cantidadAlarmas,
        uint256 _concentracionMax,
        bytes32 _hashDatos
    ) 
        external 
        parametrosValidos(_idDispositivo, _idCliente, _cantidadAlarmas, _concentracionMax)
        returns (bytes32 registroId)
    {
        // Generar ID único del registro
        registroId = keccak256(
            abi.encodePacked(
                _fecha,
                _idDispositivo,
                _idCliente,
                _cantidadAlarmas,
                _concentracionMax,
                _hashDatos,
                block.timestamp,
                msg.sender
            )
        );
        
        // Verificar que no exista un registro duplicado
        require(
            !registrosExistentes[registroId],
            "GaslytContrato: Ya existe un registro con estos datos"
        );
        
        // Marcar el registro como existente
        registrosExistentes[registroId] = true;
        
        // Crear el registro
        RegistroAlarma memory nuevoRegistro = RegistroAlarma({
            fecha: _fecha,
            idDispositivo: _idDispositivo,
            idCliente: _idCliente,
            cantidadAlarmas: _cantidadAlarmas,
            concentracionMax: _concentracionMax,
            hashDatos: _hashDatos,
            timestamp: block.timestamp,
            registradoPor: msg.sender
        });
        
        // Almacenar el registro
        registros.push(nuevoRegistro);
        totalRegistros++;
        
        // Actualizar información del dispositivo
        _actualizarDispositivo(_idDispositivo, _idCliente, _cantidadAlarmas, _concentracionMax);
        
        // Emitir evento
        emit AlarmaRegistrada(
            registroId,
            _idDispositivo,
            _fecha,
            _cantidadAlarmas,
            _concentracionMax,
            _hashDatos
        );
        
        return registroId;
    }
    
    // ============ FUNCIONES DE CONSULTA ============
    
    /*
     * @dev Obtiene información de un dispositivo
     * @param _idDispositivo ID del dispositivo a consultar
     * @return info Información del dispositivo
     */
    function obtenerDispositivo(string memory _idDispositivo)
        external
        view
        returns (DispositivoInfo memory info)
    {
        require(bytes(_idDispositivo).length > 0, "GaslytContrato: ID del dispositivo no puede estar vacio");
        return dispositivos[_idDispositivo];
    }
    
    /*
     * @dev Obtiene un registro específico por índice
     * @param _indice Índice del registro en el array
     * @return registro Datos del registro
     */
    function obtenerRegistro(uint256 _indice)
        external
        view
        returns (RegistroAlarma memory registro)
    {
        require(_indice < registros.length, "GaslytContrato: Indice fuera de rango");
        return registros[_indice];
    }
    
    /*
     * @dev Obtiene el total de registros almacenados
     * @return total Cantidad total de registros
     */
    function obtenerTotalRegistros() external view returns (uint256 total) {
        return registros.length;
    }
    
    /*
     * @dev Obtiene estadísticas generales del contrato
     * @return totalRegistros_ Total de registros
     * @return totalDispositivos_ Total de dispositivos únicos
     * @return fechaUltimoRegistro Fecha del último registro
     */
    function obtenerEstadisticas()
        external
        view
        returns (
            uint256 totalRegistros_,
            uint256 totalDispositivos_,
            uint256 fechaUltimoRegistro
        )
    {
        totalRegistros_ = registros.length;
        totalDispositivos_ = totalDispositivos;
        fechaUltimoRegistro = registros.length > 0 ? registros[registros.length - 1].timestamp : 0;
    }
    
    /*
     * @dev Verifica si un registro existe
     * @param _registroId ID del registro a verificar
     * @return existe True si el registro existe
     */
    function verificarRegistro(bytes32 _registroId) external view returns (bool existe) {
        return registrosExistentes[_registroId];
    }
    
    // ============ FUNCIONES INTERNAS ============
    
    /*
     * @dev Actualiza la información de un dispositivo
     * @param _idDispositivo ID del dispositivo
     * @param _idCliente ID del cliente
     * @param _cantidadAlarmas Cantidad de alarmas del día
     * @param _concentracionMax Concentración máxima del día
     */
    function _actualizarDispositivo(
        string memory _idDispositivo,
        string memory _idCliente,
        uint256 _cantidadAlarmas,
        uint256 _concentracionMax
    ) internal {
        DispositivoInfo storage dispositivo = dispositivos[_idDispositivo];
        
        // Si es un dispositivo nuevo
        if (dispositivo.fechaPrimerRegistro == 0) {
            dispositivo.idDispositivo = _idDispositivo;
            dispositivo.idCliente = _idCliente;
            dispositivo.fechaPrimerRegistro = block.timestamp;
            dispositivo.totalAlarmas = _cantidadAlarmas;
            dispositivo.concentracionMaxima = _concentracionMax;
            dispositivo.activo = true;
            
            totalDispositivos++;
            
            emit DispositivoRegistrado(_idDispositivo, _idCliente, block.timestamp);
        } else {
            // Actualizar estadísticas del dispositivo existente
            dispositivo.totalAlarmas += _cantidadAlarmas;
            
            if (_concentracionMax > dispositivo.concentracionMaxima) {
                dispositivo.concentracionMaxima = _concentracionMax;
            }
        }
    }
    
    // ============ FUNCIONES DE ADMINISTRACIÓN ============
    
    /*
     * @dev Transfiere la propiedad del contrato
     * @param _nuevoOwner Nueva dirección del propietario
     */
    function transferirPropiedad(address _nuevoOwner) external soloPropietario {
        require(_nuevoOwner != address(0), "GaslytContrato: Nueva direccion no puede ser cero");
        require(_nuevoOwner != owner, "GaslytContrato: Nueva direccion debe ser diferente a la actual");
        
        address anteriorOwner = owner;
        owner = _nuevoOwner;
        
        emit OwnershipTransferred(_nuevoOwner, anteriorOwner);
    }
    
    /**
     * @dev Permite al propietario renunciar a la propiedad
     * @notice Esta función hace que el contrato quede sin propietario
     */
    function renunciarPropiedad() external soloPropietario {
        address anteriorOwner = owner;
        owner = address(0);
        
        emit OwnershipTransferred(address(0), anteriorOwner);
    }
    
    // ============ FUNCIONES DE EMERGENCIA ============
    
    /*
     * @dev Permite al propietario pausar el contrato en caso de emergencia
     * @notice Esta función está preparada para futuras implementaciones de pausa
     */
    function pausarContrato() external soloPropietario {
        // Implementación futura para pausar el contrato
        // Por ahora solo emite un evento para logging
        emit OwnershipTransferred(owner, owner); // Evento de logging
    }
    
    /*
     * @dev Permite al propietario reanudar el contrato
     * @notice Esta función está preparada para futuras implementaciones de reanudación
     */
    function reanudarContrato() external soloPropietario {
        // Implementación futura para reanudar el contrato
        // Por ahora solo emite un evento para logging
        emit OwnershipTransferred(owner, owner); // Evento de logging
    }
}
