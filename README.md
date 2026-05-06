# ♾️ Infinity Glove: Edge AI Gesture-Controlled Robotics

![ESP32](https://img.shields.io/badge/Microcontroller-ESP32-blue)
![TinyML](https://img.shields.io/badge/AI-Edge_Impulse_(TinyML)-green)
![Protocol](https://img.shields.io/badge/Protocol-ESP--NOW-orange)
![C++](https://img.shields.io/badge/Language-C++-purple)

## 📌 Abstract
The **Infinity Glove** is a prototype for **Non-Tactile Teleoperation** in hazardous or complex environments. Instead of relying on traditional button-based remote controls, this project utilizes **Embedded AI (TinyML)** running locally on an ESP32 to recognize complex human hand gestures in real-time. 

By interpreting 6-axis motion data via a custom-trained Neural Network, the wearable glove can seamlessly control a robotic rover and auxiliary systems (like a cooling fan) over a low-latency, peer-to-peer wireless network (ESP-NOW).

## ✨ Key Features
* **On-Device Machine Learning:** Runs a quantized Keras Neural Network (trained via Edge Impulse) directly on a $5 microcontroller without requiring an internet connection.
* **Pattern Recognition:** Detects continuous motion patterns (e.g., "Magic Circle" gesture) using Spectral Analysis and triggers autonomous macros (360° spin).
* **Zero-Latency Communication:** Utilizes the ESP-NOW MAC-layer protocol for instant (<10ms) device-to-device communication, bypassing the need for Wi-Fi routers.
* **Dual-Mode Operation:** Features physical mode-switching between driving the chassis and operating auxiliary equipment (Fan Mode vs. Car Mode).
* **Power-Safe Macros:** Implements software-level "Pulse Spinning" to protect battery life and prevent L298N driver brownouts during high-current maneuvers.

---

## 🛠️ Hardware Components

### Transmitter (The Smart Glove)
* 1x ESP32 Development Board
* 1x MPU6050 (6-Axis Accelerometer & Gyroscope)
* 1x Push Button (Mode Switch)
* 1x LED (Mode Indicator)
* 1x Wearable Glove & Breadboard
* Portable Power Bank / Battery

### Receiver (The Rover)
* 1x ESP32 Development Board
* 1x L298N Dual H-Bridge Motor Driver
* 2x DC Gear Motors (Chassis)
* 1x DC Motor with Propeller (Auxiliary Fan)
* 1x Robot Chassis with Wheels
* Battery Pack (High Current for Motors)

---

## 🔌 Pin Configuration

### Glove (Transmitter) Wiring
| Component | ESP32 Pin |
| :--- | :--- |
| MPU6050 SDA | GPIO 21 |
| MPU6050 SCL | GPIO 22 |
| Push Button | GPIO 4 |
| Mode LED | GPIO 2 |

### Car (Receiver) Wiring
| Component | L298N / Device Pin | ESP32 Pin |
| :--- | :--- | :--- |
| Right Motors | IN1 | GPIO 12 |
| Right Motors | IN2 | GPIO 14 |
| Left Motors | IN3 | GPIO 27 |
| Left Motors | IN4 | GPIO 26 |
| Aux Fan | Motor + | GPIO 33 |
| Aux Fan | Motor - | GPIO 32 |

---

## 🧠 Software & AI Architecture

1. **Data Acquisition:** The MPU6050 samples accelerometer and gyroscope data at ~100Hz.
2. **Signal Processing:** Edge Impulse's DSP extracts spectral features (frequency and power) over a sliding 2000ms window.
3. **Classification:** A Keras Neural Network classifies the data into `idle` or `circle_data` with >80% confidence thresholds.
4. **Wireless Transmission:** The Glove maps standard tilts (X/Y axis) to characters (`F`, `B`, `L`, `R`) and transmits them via `esp_now_send()`.
5. **Execution:** The Receiver ESP32 listens via an interrupt-driven callback (`esp_now_recv_cb_t`) and maps characters to GPIO outputs for the motor drivers. *Note: Receiver code is optimized for ESP32 Board Manager v3.0+.*

---

## 🚀 How to Install & Run

### Prerequisites
* Arduino IDE 2.x
* ESP32 Board Manager installed (v3.0.0 or higher recommended)
* Edge Impulse Arduino Library (Exported from your specific project)

### Setup Steps
1. **Find Receiver MAC Address:** Flash a simple script to your Car's ESP32 to find its MAC address.
2. **Update Glove Code:** Open `Glove_Code.ino` and replace the `broadcastAddress[]` array with your Car's MAC address.
3. **Include AI Library:** Ensure your custom `.zip` library from Edge Impulse is added to your Arduino IDE (`Sketch -> Include Library -> Add .ZIP Library`).
4. **Flash Devices:** Upload `Glove_Code.ino` to the Glove ESP32 and `Receiver_Code.ino` to the Car ESP32.
5. **Power On:** Power the Car first, then the Glove. 

---

## 🎮 Operation Manual

* **Default State (Fan Mode):** Blue LED is OFF. Tilting the hand left/right controls the fan speed.
* **Switching Modes:** Long-press the button (>1 sec). Blue LED turns ON. You are now in Car Mode.
* **Driving (Car Mode):** Tilt hand Forward, Backward, Left, or Right to drive.
* **The Magic Spell (Edge AI):** While in Car Mode, hold your hand flat, then physically draw a 2-second circle in the air. The AI will detect the pattern and command the rover to execute a 3-second 360° spin automatically.
* **Emergency Stop:** Short-press the button at any time to halt all motors.

---

## 👥 Meet the Team

This project was designed, built, and programmed by our engineering team:
* **Lithika N** - Team Lead
* **Shruti Rakshana.T** - Hardware & AI Integration
* **Adhokshajan** - Systems Design
* **Ajay Adithya R** - Embedded Programming & Comm. Protocols

*Built as part of a final-year engineering prototype.*
