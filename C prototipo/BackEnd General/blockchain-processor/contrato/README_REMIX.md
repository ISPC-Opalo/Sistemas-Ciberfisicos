# 🚀 Despliegue del Contrato GASLYT en Remix

## 📋 Instrucciones para Remix IDE

### 1. Preparación del Entorno

1. **Abrir Remix IDE**: Ve a [remix.ethereum.org](https://remix.ethereum.org)

2. **Crear nuevo archivo**: 
   - Clic en "File Explorer" (ícono de carpeta)
   - Clic derecho → "New File"
   - Nombre: `GaslytContract.sol`

3. **Copiar código**: Pega el contenido del archivo `GaslytContract.sol`

### 2. Configuración del Compilador

1. **Abrir pestaña Solidity Compiler**:
   - Clic en el ícono de compilador (⚙️)
   - Versión del compilador: `0.8.19` o superior
   - Language: `Solidity`
   - EVM Version: `default`

2. **Compilar**:
   - Clic en "Compile GaslytContract.sol"
   - Verificar que no hay errores

### 3. Despliegue del Contrato

1. **Abrir pestaña Deploy & Run**:
   - Clic en el ícono de despliegue (🚀)
   - Environment: `Injected Web3` (para MetaMask)
   - Account: Selecciona tu cuenta de MetaMask
   - Gas Limit: `5000000` (ajustar según necesidad)

2. **Conectar MetaMask**:
   - Asegúrate de tener MetaMask instalado
   - Conecta a la red deseada (Sepolia para testing)
   - Verifica que tienes ETH suficiente para gas

3. **Desplegar**:
   - Clic en "Deploy"
   - Confirma la transacción en MetaMask
   - Espera la confirmación

### 4. Configuración Post-Despliegue

1. **Obtener dirección del contrato**:
   - Copia la dirección del contrato desplegado
   - Formato: `0x...`

2. **Actualizar configuración**:
   ```bash
   # En tu archivo .env
   CONTRACT_ADDRESS=0x[DIRECCION_DEL_CONTRATO]
   ```

3. **Verificar despliegue**:
   - En Remix, expande el contrato desplegado
   - Verifica que `owner` sea tu dirección
   - `totalRegistros` debe ser 0 inicialmente

## 🔧 Funciones del Contrato

### Funciones Principales

- **`registrarAlarmaDiaria`**: Registra alarmas diarias de dispositivos
- **`obtenerDispositivo`**: Consulta información de un dispositivo
- **`obtenerRegistro`**: Obtiene un registro específico
- **`obtenerEstadisticas`**: Estadísticas generales del contrato

### Funciones de Administración

- **`transferirPropiedad`**: Transfiere la propiedad del contrato
- **`renunciarPropiedad`**: Renuncia a la propiedad
- **`pausarContrato`**: Pausa el contrato (futuro)
- **`reanudarContrato`**: Reanuda el contrato (futuro)

## 📊 Parámetros de la Función Principal

```solidity
function registrarAlarmaDiaria(
    uint256 _fecha,           // Timestamp Unix de la fecha
    string memory _idDispositivo,  // ID único del dispositivo
    string memory _idCliente,      // ID del cliente propietario
    uint256 _cantidadAlarmas,      // Cantidad de alarmas del día
    uint256 _concentracionMax,     // Concentración máxima del día
    bytes32 _hashDatos            // Hash SHA-256 de verificación
) external returns (bytes32 registroId)
```

## 🧪 Testing en Remix

### 1. Probar Registro de Alarma

1. **Expandir contrato desplegado**
2. **Clic en `registrarAlarmaDiaria`**
3. **Completar parámetros**:
   ```
   _fecha: 1704067200 (1/1/2024)
   _idDispositivo: "GASLYT_001"
   _idCliente: "CLIENTE_001"
   _cantidadAlarmas: 5
   _concentracionMax: 150
   _hashDatos: 0x1234567890abcdef... (32 bytes)
   ```
4. **Ejecutar transacción**

### 2. Verificar Resultados

1. **Consultar `totalRegistros`**: Debe incrementar a 1
2. **Consultar `totalDispositivos`**: Debe incrementar a 1
3. **Usar `obtenerDispositivo`**: Con "GASLYT_001"
4. **Usar `obtenerRegistro`**: Con índice 0

## 🔒 Consideraciones de Seguridad

### Validaciones Implementadas

- ✅ Verificación de parámetros vacíos
- ✅ Prevención de registros duplicados
- ✅ Control de acceso por propietario
- ✅ Validación de cantidades positivas

### Recomendaciones

- 🔐 Usar claves privadas seguras
- 🔐 Desplegar en redes de prueba primero
- 🔐 Verificar código antes del despliegue
- 🔐 Mantener backup de la clave privada

## 📈 Optimización de Gas

### Estimaciones de Gas

- **Despliegue**: ~2,500,000 gas
- **Registro de alarma**: ~150,000 gas
- **Consultas**: ~30,000 gas (view functions)

### Optimizaciones Implementadas

- ✅ Uso de `memory` para strings
- ✅ Estructuras eficientes
- ✅ Eventos para logging
- ✅ Mapeos optimizados

## 🚨 Solución de Problemas

### Errores Comunes

1. **"Out of gas"**:
   - Aumentar gas limit
   - Verificar red congestionada

2. **"Transaction failed"**:
   - Verificar saldo de ETH
   - Revisar parámetros de entrada

3. **"Contract not deployed"**:
   - Verificar compilación exitosa
   - Revisar configuración de red

### Logs y Debugging

- Usar eventos emitidos para debugging
- Verificar transacciones en explorador de bloques
- Revisar logs de Remix para errores

## 📚 Recursos Adicionales

- [Documentación de Solidity](https://docs.soliditylang.org/)
- [Remix IDE Documentation](https://remix-ide.readthedocs.io/)
- [Ethereum Gas Tracker](https://ethgasstation.info/)
- [Sepolia Faucet](https://sepoliafaucet.com/)

---

**Versión del Contrato**: 1.0  
**Compilador**: Solidity ^0.8.19  
**Red Recomendada**: Sepolia (testing)  
**Gas Estimado**: 150,000 por registro
