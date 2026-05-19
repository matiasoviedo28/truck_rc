# Control WiFi e interfaz web

## Arquitectura

El ESP32 funciona como **Access Point WiFi independiente**. No se conecta a ninguna red externa: crea su propia red a la que el celular se conecta directamente. Esto significa:

- ✅ Funciona en cualquier lugar sin necesidad de WiFi externo
- ✅ Latencia mínima (sin pasar por router)
- ✅ Sin dependencia de internet
- ❌ El celular no tendrá acceso a internet mientras esté conectado al truck

---

## Conexión desde el celular

![Conexión WiFi](images/19216841.jpeg)

1. **Red WiFi:** `Camion-RC-NeutronLab`
2. **Contraseña:** `12345678`
3. **Abrir navegador en:** `http://192.168.4.1`

Ambos valores se pueden modificar al inicio del sketch:

```cpp
const char* ssid     = "Camion-RC-NeutronLab";
const char* password = "12345678";
```

---

## Protocolo de comunicación

El frontend hace **polling HTTP cada 100 ms** al endpoint `/c` con los siguientes parámetros:

| Parámetro | Tipo | Valores | Descripción |
|-----------|------|---------|-------------|
| `m` | int | 0, 1, 2 | Estado del motor: 0=stop, 1=adelante, 2=reversa |
| `d` | int | -360 a 360 | Ángulo del volante virtual |
| `l` | int | 0, 1 | Luces bajas |
| `h` | int | 0, 1 | Balizas (4 intermitentes) |
| `c` | int | 0, 1 | Cardán acoplado |
| `t` | char | U, D, espacio | Comando de tolva: Up, Down, sin acción |

**Ejemplo de request:**
```
GET /c?m=1&d=45&l=1&h=0&c=0&t=U
```

**Respuesta del ESP32:** estado actual de la tolva (`LISTO`, `SUBIENDO` o `BAJANDO`) en texto plano.

---

## Interfaz visual

La interfaz está optimizada para uso vertical en celulares y cuenta con:

- **Header con chips de estado:** CONEXIÓN (ONLINE/OFFLINE) y TOLVA (estado actual).
- **Indicadores de giro:** flechas ◄ ► que parpadean en naranja según ángulo del volante o si las balizas están activas.
- **Volante SVG estilo camión clásico** con marcador rojo central y 3 rayos.
- **Triggers verticales:** ACELERAR (verde), FRENO (naranja), REVERSA (rojo).
- **Controles de tolva:** TOLVA UP / TOLVA DOWN con bloqueos automáticos.
- **Botón CARDÁN** con feedback de estado (ACOPLADO / DESACOPL.).
- **Modo de manejo:** RÁPIDO (toque y mantiene) o LENTO (mantener presionado).
