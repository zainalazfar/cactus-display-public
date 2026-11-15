# 🌵 Retro Cactus Display (ESP32-S3)

An interactive smart desk companion built on the **ESP32-S3 (JC4827W543 4.3" 480x272 display)** featuring a pixel-art cactus monitor, live internet radio tuner, and live ADS-B flight radar.

---

## ⚡ Quick Start: Configure Your Wi-Fi

Internet Radio, Flight Radar, and NTP time require a **2.4 GHz** Wi-Fi connection.

1. Open [`src/config.h`](src/config.h).
2. Enter your Wi-Fi network name and password:
   ```c
   #pragma once

   #define WIFI_SSID     "YOUR_WIFI_SSID"
   #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
   ```
3. Save the file and compile/upload to your board!

---

## 📻 Features & Views

Press the **Rotary Encoder button** or **Mode switch** to toggle between views:

1. **🌵 Cactus Dashboard**:
   - Pixel-art animated cactus with live DHT11 temperature and humidity readings.
   - Synchronized NTP real-time clock and dynamic day/night background.
2. **📻 Malaysian & International Radio Tuner**:
   - Smooth streaming via `ESP32-audioI2S` (Hot FM, Kool 101, Suria FM, BFM 89.9, Fly FM, etc.).
   - Interactive rotary knob tuning with vintage analog frequency dial, static noise simulation, and pixel fire visualizer.
3. **✈️ Retro Flight Radar & Arrivals Board**:
   - Live ADS-B radar scope with sweep animation.
   - Real-time airport arrival board navigable with rotary knob.

---

## 🛠️ Hardware Requirements

* **Development Board**: ESP32-S3 JC4827W543 (4.3-inch 480×272 NV3041A QSPI display with capacitive touch).
* **Audio DAC/Amp**: NS4168 or MAX98357A I2S Class-D amplifier module.
* **Speaker**: 3W / 4Ω or 8Ω mini speaker.
* **Rotary Encoder**: Standard EC11 quadrature rotary encoder with push button.
* **Sensor**: DHT11 Temperature & Humidity Sensor.
* **Soil Sensor (Optional)**: HW-390 capacitive moisture sensor.

---

## 🔌 Pinout & Wiring

| Peripheral | Board Pin / GPIO | Description |
| :--- | :--- | :--- |
| **I2S Audio BCLK** | `GPIO 42` | Bit Clock |
| **I2S Audio LRC** | `GPIO 2` | Word Select (Left/Right Clock) |
| **I2S Audio DOUT** | `GPIO 41` | Serial Data Out (to Amp DIN) |
| **Rotary CLK** | `GPIO 6` | Encoder Channel A |
| **Rotary DT** | `GPIO 7` | Encoder Channel B |
| **Rotary SW** | `GPIO 15` | Encoder Switch (Push to GND) |
| **Mode Button** | `GPIO 18` | Mode Switch (Toggle view to GND) |
| **DHT11 Data** | `GPIO 17` | Temp / Humidity Data |
| **Soil Sensor** | `GPIO 16` | Analog Soil Moisture (HW-390) |

> 📷 *Check `arduino_wiring_diagram_vertical.jpg` and `arduino_wiring_diagram_vertical_v2.jpg` in this repository for full wiring schematics.*

---

## 🚀 How to Flash & Run

### Method 1: Using PlatformIO (Recommended)
1. Install [VS Code](https://code.visualstudio.com/) and the **PlatformIO IDE** extension.
2. Open this project folder in VS Code.
3. Edit `src/config.h` with your Wi-Fi credentials.
4. Connect your ESP32-S3 via USB-C.
5. Click **PlatformIO: Upload** (arrow icon in the bottom status bar) or run:
   ```bash
   pio run -t upload
   ```

### Method 2: Using Arduino IDE
1. Install the **ESP32 by Espressif Systems** board package in Arduino Board Manager.
2. Install the following libraries via Arduino Library Manager:
   - `Arduino_GFX_Library`
   - `lvgl` (v8.3.x)
   - `ESP32-audioI2S`
   - `DHT sensor library`
   - `ArduinoJson`
3. Copy `src/lv_conf.h` to your Arduino `libraries` folder or alongside `lvgl`.
4. Open `src/main.ino`, set Wi-Fi in `src/config.h`, select **ESP32S3 Dev Module** (with PSRAM enabled - OPI PSRAM), and click **Upload**.

---

## 📄 License
MIT License. Feel free to use and adapt for your own DIY desk setup!
