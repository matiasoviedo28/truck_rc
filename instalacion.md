# Instalación del firmware

## Requisitos

- Arduino IDE 1.8+ o 2.x
- Plataforma ESP32 instalada en el IDE ([guía oficial](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html))

## Librerías necesarias

Instalar desde el **Library Manager** del IDE:

- **AccelStepper** (Mike McCauley) — control del NEMA17
- **ESP32Servo** — control del servo
- `WiFi.h` y `WebServer.h` — incluidas en la plataforma ESP32

## Pasos

1. Clonar el repositorio:
   ```bash
   git clone https://github.com/TU_USUARIO/rc-truck-neutronlab.git
   ```
2. Abrir el archivo `.ino` con Arduino IDE.
3. Seleccionar la placa: **ESP32 Dev Module**.
4. Conectar el ESP32 por USB y elegir el puerto correspondiente.
5. Compilar y subir.
6. Abrir el Serial Monitor a **115200 baudios** para ver la IP del AP al iniciar.

## Prueba inicial recomendada

Antes de conectar todo el hardware, sugiero probar en este orden:

1. ✅ Subir el sketch al ESP32 sin nada conectado.
2. ✅ Conectarse al WiFi `Camion-RC-NeutronLab` desde el celular.
3. ✅ Acceder a `192.168.4.1` y verificar que la interfaz cargue.
4. ✅ Conectar el servo y las luces.
5. ✅ Conectar el motor de tracción.
6. ✅ **Último paso:** conectar el driver A4988 + NEMA17 (con Vref ya calibrado).
