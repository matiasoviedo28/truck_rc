# Funciones del camión y sistema de seguridad

## Funciones del camión

### Dirección

- Volante táctil con rotación de hasta ±360° (dos vueltas completas a cada lado).
- Mapeo proporcional al servo físico (rango 55° a 135°, centro en 95°).
- Vuelve al centro automáticamente al soltar.

### Tracción

- **Modo RÁPIDO:** un toque al acelerador y el truck queda avanzando hasta tocar FRENO.
- **Modo LENTO:** mantener presionado para avanzar, soltar para detener.
- Reversa con la misma lógica según modo.

### Tolva volcadora

- Motor NEMA17 controlado con AccelStepper (no bloqueante).
- Recorrido seguro: **9000 pasos** (con margen del límite físico real de 10000).
- Velocidad: 1000 pasos/s, aceleración 400 pasos/s².
- Botón se ilumina durante el movimiento y se apaga al terminar.
- Comportamiento "un toque": va sola hasta el tope, sin STOP intermedio.
- Posición de reposo al encender: **tolva abajo = 0 pasos**. Subir avanza hacia `PASOS_MAXIMOS`, bajar regresa a `0`.

#### Gestión de energía del driver (pin ENABLE)

El driver A4988 se controla mediante el pin **GPIO 5** (ENABLE), evitando que el motor consuma corriente innecesariamente cuando está quieto.

| Momento | Estado ENABLE | Efecto |
|---------|--------------|--------|
| Arranque del sistema | HIGH (inactivo) | Motor frío, sin consumo |
| Se recibe comando UP/DOWN | LOW (activo) + delay 2 ms | Driver despierta antes del primer pulso |
| Tolva llega al destino | HIGH (inactivo) | Corriente cortada al terminar |
| Failsafe por pérdida de WiFi | HIGH (inactivo) | Motor se detiene y se desenergiza |

> **Por qué importa:** un NEMA17 parado pero energizado puede alcanzar 50–70 °C en pocos minutos y drenar la batería continuamente. Con el control de ENABLE el motor solo consume durante el movimiento efectivo.

### Sistema de luces

- **Luces bajas:** toggle manual.
- **Luz de freno:** se enciende automáticamente al transicionar de movimiento a detenido, durante **1500 ms** y se apaga sola.
- **Luz de reversa:** activa mientras el truck va en reversa.
- **Giros automáticos:** parpadean en el lado correspondiente cuando el volante supera ±45°.
- **Balizas (4 intermitentes):** toggle manual, parpadeo a 500 ms.

### Cardán (toma de fuerza simulada)

Replica el comportamiento real de un camión: la tolva solo puede operar con el cardán acoplado.

**Bloqueos cruzados:**
- El cardán solo se puede acoplar/desacoplar con el motor detenido.
- Con el cardán acoplado, el acelerador y reversa quedan bloqueados.
- La tolva solo responde si: cardán acoplado + motor detenido + conexión OK.

---

## Sistema de seguridad y failsafe

El sistema tiene múltiples capas de seguridad pensadas para un control inalámbrico.

### Detección de desconexión

- **Frontend (celular):** detecta OFFLINE si no recibe respuesta del ESP32 en **500 ms**.
- **Backend (ESP32):** activa el failsafe si no recibe comandos del frontend en **1000 ms**.

### Acciones de failsafe

Cuando el ESP32 detecta pérdida de conexión:

| Componente | Acción |
|------------|--------|
| Motor de tracción | **Apagado inmediato** (PWM a 0) |
| Tolva (paso a paso) | **Detención** con `stepper.stop()` + driver desenergizado (ENABLE = HIGH) |
| Servo de dirección | **Queda quieto** en su última posición |
| Luces bajas | Apagadas |
| Balizas | Mantienen parpadeo (señal visual de vehículo abandonado) |

### Defensa en profundidad

Los bloqueos cruzados (cardán ↔ motor, motor ↔ tolva, cardán ↔ tolva) se validan **tanto en el frontend como en el backend**. Aunque el frontend bloquea visualmente los botones inválidos, el ESP32 también los ignora si llegan por algún glitch o manipulación del request.
