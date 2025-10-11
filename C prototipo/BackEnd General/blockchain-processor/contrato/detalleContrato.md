# Diagrama técnico — Estructura interna del contrato

┌────────────────────────────────────────────────────┐
│                 Gaslyt-Contrato                    │
│────────────────────────────────────────────────────│
│                  VARIABLES DE ESTADO               │
│  owner: address                                    │
│  totalRegistros: uint256                           │
│  totalDispositivos: uint256                        │
│  registrosExistentes: mapping(bytes32 => bool)     │
│  dispositivos: mapping(string => DispositivoInfo)  │
│  registros: RegistroAlarma[]                       │
│────────────────────────────────────────────────────│
│                      STRUCTS                       │
│  DispositivoInfo {                                 │
│     idDispositivo: string                          │
│     idCliente: string                              │
│     fechaPrimerRegistro: uint256                   │
│     totalAlarmas: uint256                          │
│     concentracionMaxima: uint256                   │
│     activo: bool                                   │
│  }                                                 │
│                                                    │
│  RegistroAlarma {                                  │
│     fecha: uint256                                 │
│     idDispositivo: string                          │
│     idCliente: string                              │
│     cantidadAlarmas: uint256                       │
│     concentracionMax: uint256                      │
│     hashDatos: bytes32                             │
│     timestamp: uint256                             │
│     registradoPor: address                         │
│  }                                                 │
│────────────────────────────────────────────────────│
│                      EVENTOS                       │
│  event RegistroCreado(RegistroAlarma)              │
│────────────────────────────────────────────────────│
│                      FUNCIONES                     │
│  constructor()                                     │
│  registrarAlarma(...)                              │
│  getRegistro(index) view                           │
│  getDispositivo(id) view                           │
│  transferOwnership(nuevoOwner)                     │
│  activarDesactivarDispositivo(id, bool)            │
└────────────────────────────────────────────────────┘


# Flujo técnico del proceso registrarAlarma()

┌─────────────────────────────────────────────────────────────┐
│                     registrarAlarma()                       │
└─────────────────────────────────────────────────────────────┘
                │
                ▼
┌──────────────────────────────────────────────┐
│ 1. Recepción de parámetros:                  │
│    - idDispositivo, idCliente                │
│    - fecha, cantidadAlarmas, concentracionMax│
│    - hashDatos (calculado off-chain)         │
└──────────────────────────────────────────────┘
                │
                ▼
┌──────────────────────────────────────────────┐
│ 2. Generar clave única del registro:         │
│    key = keccak256(idDispositivo + fecha)    │
│    Verificar que registrosExistentes[key] == │
│    false  → si true, revertir transacción    │
└──────────────────────────────────────────────┘
                │
                ▼
┌──────────────────────────────────────────────┐
│ 3. Actualizar datos del dispositivo          │
│    - Si no existe, crearlo en `dispositivos` │
│    - Si existe, actualizar:                  │
│        totalAlarmas += cantidadAlarmas       │
│        concentracionMaxima = max(actual, nvo)│
│    - totalDispositivos++ si es nuevo         │
└──────────────────────────────────────────────┘
                │
                ▼
┌──────────────────────────────────────────────┐
│ 4. Crear objeto RegistroAlarma               │
│    registro = RegistroAlarma({ ... })        │
│    registros.push(registro)                  │
│    totalRegistros++                          │
│    registrosExistentes[key] = true           │
└──────────────────────────────────────────────┘
                │
                ▼
┌──────────────────────────────────────────────┐
│ 5. Emitir evento RegistroCreado()            │
│    Permite que sistemas externos (dApps,     │
│    oráculos, APIs) detecten el registro en   │
│    tiempo real.                              │
└──────────────────────────────────────────────┘
                │
                ▼
┌──────────────────────────────────────────────┐
│ 6. Fin — transacción confirmada              │
│    Hash, timestamp y address quedan grabados │
│    de forma inmutable en la blockchain.      │
└──────────────────────────────────────────────┘
