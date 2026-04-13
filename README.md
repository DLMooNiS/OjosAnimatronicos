# 👁️ Ojos Animatrónicos con Control por Joystick

Este proyecto consiste en un sistema de ojos animatrónicos de 6 servomotores controlados por un Arduino Uno y un controlador PWM PCA9685. El sistema cuenta con dos modos: un **Modo Centinela** (automático) y un **Modo Manual** mediante un Joystick.

## 🚀 Características
* **6 Servomotores:** Control de pestañas (2) y movimiento de ojos X/Y (4).
* **Movimiento Suave:** Lógica de interpolación para evitar movimientos bruscos y mecánicos.
* **Consonancia Binocular:** Movimiento coordinado de ambos ojos para un efecto realista.
* **Control Dual:** Alternancia entre patrullaje automático y control manual.
* **Indicador LED:** Visualización del estado del sistema (Activo/Pausa).

## 🛠️ Componentes Necesarios
* 1x **Arduino Uno**
* 1x **Controlador PCA9685** (16 canales PWM I2C)
* 6x **Servomotores SG90** (o similares)
* 1x **Joystick Analógico**
* 1x **Botón Pulsador** (Pausa/Reanudar)
* 1x **LED** + Resistencia 220Ω
* 1x **Fuente de alimentación externa** (5V-6V para los servos)

## 📋 Esquema de Conexiones

### PCA9685 (I2C)
| Arduino | PCA9685 |
| :--- | :--- |
| 5V | VCC |
| GND | GND |
| A4 | SDA |
| A5 | SCL |

### Asignación de Servos (Canales PCA)
* **Canal 4/5:** Pestañas (Deri/Izq)
* **Canal 8/9:** Ojo Derecho (X/Y)
* **Canal 12/13:** Ojo Izquierdo (X/Y)

### Entradas (Arduino Directo)
* **Pin A0:** Joystick VRx
* **Pin A1:** Joystick VRy
* **Pin 2:** Botón Joystick (Parpadeo)
* **Pin 4:** Botón de Pausa
* **Pin 12:** LED Indicador

## 🕹️ Funcionamiento del Joystick
El control manual utiliza la función `map()` para traducir los valores analógicos (0-1023) a los límites de giro seguros de los servos. Además, se utiliza una función de **movimiento suave coordinado** que divide el trayecto en micro-pasos para simular el movimiento de un ojo orgánico.

## 💻 Instalación
1.  Clona este repositorio.
2.  Instala la librería **Adafruit PWM Servo Driver** en tu IDE de Arduino.
3.  Carga el archivo `.ino` a tu placa.
4.  ¡Disfruta de tu animatrónico!

---
Proyecto desarrollado para simulación en FRITZING y montaje físico.
