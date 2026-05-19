# Diagrama de conexiones y pines

## Diagrama general

```
                        ┌─────────────────┐
                        │   CELULAR       │
                        │   (Navegador)   │
                        └────────┬────────┘
                                 │ WiFi 2.4 GHz
                                 │ (AP del ESP32)
                                 │
                        ┌────────▼────────┐
                        │     ESP32       │
                        │   (Servidor     │
                        │     web AP)     │
                        └────────┬────────┘
                                 │
        ┌────────────┬───────────┼───────────┬──────────────┐
        │            │           │           │              │
   ┌────▼────┐  ┌────▼────┐ ┌────▼────┐ ┌────▼────┐  ┌──────▼──────┐
   │  Servo  │  │ Driver  │ │ Driver  │ │  LEDs   │  │   Driver    │
   │ Dirección│ │ Motor DC│ │  A4988  │ │ (luces) │  │   A4988     │
   │         │  │ (PWM)   │ │ NEMA17  │ │         │  │             │
   └─────────┘  └─────────┘ └─────────┘ └─────────┘  └─────────────┘
```

---

## Tabla de pines del ESP32

![ESP32](images/esp32.png)

| GPIO | Función | Descripción |
|------|---------|-------------|
| **2** | Servo dirección | PWM 50 Hz al servo (rango 500-2400 µs) |
| **4** | DIR (A4988) | Dirección del motor paso a paso de tolva |
| **12** | Motor A | PWM hacia adelante (motor de tracción) |
| **13** | Motor B | PWM hacia atrás / reversa |
| **14** | Luces bajas | Salida digital LED bajas |
| **25** | Giro izquierdo | Salida digital LED giro izq |
| **26** | Luz reversa | Salida digital LED reversa |
| **27** | Luz freno | Salida digital LED freno (timer 1500 ms) |
| **32** | STEP (A4988) | Pulsos al motor paso a paso de tolva |
| **33** | Giro derecho | Salida digital LED giro der |

---

## Conexión del driver A4988

| Pin A4988 | Conectar a | Observación |
|-----------|-----------|-------------|
| VMOT | +12V | Alimentación del motor |
| GND (motor) | GND batería 12V | |
| VDD | +5V | Alimentación de lógica del driver |
| GND (lógica) | GND del ESP32 | **Crítico:** debe compartir GND con ESP32 |
| STEP | GPIO 32 | Pulsos de paso |
| DIR | GPIO 4 | Sentido de giro |
| ENABLE | GND | Motor siempre energizado |
| RESET | SLEEP | Puenteados entre sí |
| 1A, 1B, 2A, 2B | Bobinas NEMA17 | Identificar con multímetro en continuidad |

> ⚠️ **Importante:** Calibrar el **Vref** del A4988 antes de usarlo. Para NEMA17 típico (1.5A) debe estar entre 0.6V y 0.8V. Medir entre el potenciómetro y GND con el driver alimentado pero **sin el motor conectado**.

> ⚠️ **Nunca conectar o desconectar el motor con el driver alimentado** — destruye instantáneamente el A4988.
