![Banner ISPC](./E%20assets/Material%20de%20Referencia/ISPClogo.png)

# Tecnicatura Superior en Telecomunicaciones – Segundo Año Cohorte 2024

**Docente:** Ana María Farias  
**Institución:** ISPC – Instituto Superior Politécnico Córdoba  
**Materia:** Sistemas Ciberfísicos

## 👥 Integrantes del equipo

- **Lujan, Luciano** — [GitHub](https://github.com/lucianoilujan)  
- **Durigutti, Vittorio** — [GitHub](https://github.com/vittoriodurigutti)  
- **Juncos, Lisandro** — [GitHub](https://github.com/Lisandro-05)  
- **Marquez, Jose** — [GitHub](https://github.com/marquezjose)

---

## 🛰️ Descripción General de la Materia

**Sistemas Ciberfísicos** integra tecnologías IoT, sensorización, automatización industrial y comunicación en red para el desarrollo de soluciones inteligentes.  
La materia propone la construcción de sistemas distribuidos que vinculen hardware embebido, infraestructura de red y plataformas de visualización de datos en tiempo real.

El enfoque está orientado a:
- Diseño de arquitecturas IoT.
- Interacción entre sistemas físicos y lógicos.
- Automatización de procesos industriales.
- Monitoreo remoto y toma de decisiones basada en datos.

---

## 📁 Descripción del Repositorio

Este repositorio centraliza:
- Documentación técnica de requerimientos y diseño.  
- Código fuente de firmware embebido (ESP32).  
- Backend para integración de datos, blockchain y monitoreo.  
- Dashboards para visualización de KPIs y alarmas.  
- Evidencias de implementación y entregables académicos.

### 🧭 Organización principal

- 📦 Sistemas-Ciberfisicos
- ┣ 📂 A_requisitos → Documentación base, alcance y requerimientos
- ┣ 📂 B_investigacion → Material técnico, datasheets y análisis de sensores
- ┣ 📂 C_prototipo → Código fuente, backend, nodos IoT y dashboards
- ┣ 📂 D_presentacion → Evidencias, informes y presentaciones finales
- ┣ 📂 E_assets → Imágenes, diagramas de arquitectura y multimedia
- ┗ 📜 README.md

---

## 🧰 Stack Tecnológico

| Componente                     | Tecnología / Herramienta                    |
|----------------------------------|---------------------------------------------|
| **Hardware IoT**                | ESP32, sensores MQ (gases), ventiladores, LoRa |
| **Comunicación**                | MQTT, WiFi                            |
| **Backend**                     | Python (Flask/FastAPI), MariaDB, Redis, Telegraf |
| **Infraestructura**            | Docker, AWS IoT Core, CloudWatch            |
| **Blockchain**                  | Solidity, Web3.py                           |
| **Visualización de datos**     | Grafana, Node-RED                           |                   |

---

## 🎯 Finalidad del Producto

**GASLYT** — Sistema Ciberfísico IoT para monitoreo en tiempo real de gases en espacios hogareños confinados.  

El sistema:
- Detecta concentraciones de gases críticos.  
- Transmite datos en tiempo real vía MQTT.  
- Ejecuta acciones automáticas de ventilación.  
- Almacena información en blockchain para trazabilidad.  
- Visualiza alarmas y métricas en paneles dinámicos.

👉 Esta solución permite **aumentar la seguridad hogareña** y **optimizar la respuesta operativa** mediante integración IoT y automatización.

---

## Componentes Clave del Proyecto

- 🧠 **Nodo Central:** Captura de gases, Coordinación y activación de sistemas de ventilación. Y transmisión inalámbrica hacia el backend
- 🖥️ **Backend:** Procesamiento, almacenamiento y publicación de datos.  
- 🔗 **Blockchain:** Registro inmutable de eventos críticos.  
- 📊 **Dashboard Grafana:** Monitoreo en tiempo real.  

---

## 📜 Licencia

Este proyecto se distribuye bajo la [MIT License](./LICENSE).

---

## 🧭 Créditos

Proyecto desarrollado en el marco de la materia **Sistemas Ciberfísicos** – ISPC 2025.  
Parte del trayecto académico de la **Tecnicatura Superior en Telecomunicaciones**.

---
