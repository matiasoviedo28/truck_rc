# RC Truck NeutronLab — ESP32 Control System

![RC Truck 3d](images/diseno_cabina.png)
![RC Truck 3d](images/1.jpeg)

Sistema de control inalámbrico para camión RC con tolva volcadora basado en ESP32, con interfaz web embebida accesible desde cualquier celular sin necesidad de aplicación.

El proyecto reemplaza los radiocontroles tradicionales por un control vía WiFi (modo Access Point) servido directamente desde el ESP32, replicando el comportamiento de un camión real: cardán/toma de fuerza, bloqueos cruzados de seguridad, luces de freno con timer, balizas, indicadores de giro automáticos y failsafe por pérdida de conexión.

![RC Truck 3d](images/2.jpeg)

---

## Tabla de contenidos

- [Características principales](#características-principales)
- [Hardware utilizado](#hardware-utilizado)
- [Documentación](#documentación)
- [Estructura del proyecto](#estructura-del-proyecto)
- [Contribuciones](#contribuciones)
- [Licencia](#licencia)
- [Créditos](#créditos)

---

## Características principales

- **Control vía WiFi sin internet** — el ESP32 crea su propio Access Point, no requiere router externo.
- **Interfaz web embebida** — funciona en cualquier celular con navegador, sin instalar apps.
- **Volante táctil SVG** — control de dirección con gestos de rotación realistas.
- **Tolva con motor paso a paso** — controlada directamente desde el ESP32 con AccelStepper.
- **Cardán virtual** — simula la toma de fuerza de un camión real con bloqueos cruzados.
- **Sistema de luces realista** — bajas, freno con timer, reversa, giros automáticos y balizas.
- **Failsafe automático** — corta motores ante pérdida de conexión.
- **Indicador ONLINE/OFFLINE** en tiempo real.

---

## Hardware utilizado

| Componente | Modelo / Especificación |
|---|---|
| Microcontrolador | ESP32 DevKit V1 (30 pines) |
| Driver paso a paso | A4988 (con disipador) |
| Motor de tolva | NEMA17 |
| Motor de tracción | DC con driver PWM (puente H) |
| Servo de dirección | SG90 / MG90S (50 Hz, 500-2400 µs) |
| Alimentación motor | 12V (batería o fuente) |
| Alimentación lógica | 5V (del ESP32 o externa) |
| LEDs | Luces bajas, freno, reversa, giros izq/der |

---

## Documentación

| Archivo | Contenido |
|---------|-----------|
| [`guia_armado.md`](guia_armado.md) | Paso a paso completo: impresión 3D, ensamble mecánico y eléctrico |
| [`conexiones.md`](conexiones.md) | Diagrama de conexiones, tabla de pines ESP32 y configuración del A4988 |
| [`interfaz_web.md`](interfaz_web.md) | Control WiFi, protocolo HTTP e interfaz visual |
| [`funciones.md`](funciones.md) | Funciones del camión y sistema de seguridad / failsafe |
| [`instalacion.md`](instalacion.md) | Requisitos, librerías y carga del firmware |

---

## Estructura del proyecto

```
.
├── truck_rc_neutronlab.ino   # Sketch principal del ESP32
├── README.md                 # Este archivo
├── guia_armado.md            # Paso a paso de construcción
├── conexiones.md             # Pines y diagrama eléctrico
├── interfaz_web.md           # Control WiFi e interfaz
├── funciones.md              # Funciones y failsafe
├── instalacion.md            # Carga del firmware
├── images/                   # Imágenes del proyecto
└── stl/                      # Archivos para impresión 3D
```

---

## Contribuciones

Las contribuciones son bienvenidas. Si querés agregar funcionalidades, corregir bugs o adaptar el proyecto a otros chasis, hacé un fork y mandá un pull request.

Ideas pendientes de implementación:
- Control de velocidad gradual (actualmente PWM fijo en 255).
- Conversión a WebSockets para reducir latencia.
- Modo de "pausa" para el truck con configuración persistente.
- Telemetría de batería.

---

## Licencia

Este proyecto está bajo licencia MIT — libre uso, modificación y distribución.

---

## Créditos

**Proyecto creado por:** Matías Oviedo
**Nick:** NeutronLab
**Contacto:** matiasalbertooviedogonzalez@gmail.com

---

> Si te sirvió el proyecto o te gustó la idea, no olvides dejar una estrella en el repositorio.
