# 🌵 ESP32 JC4827W543 Web Emulator

A web-based emulator for the **Smart Cactus Monitor** running on the JC4827W543 (ESP32-S3 + NV3041A 480×272 display) with LVGL 8.3.11.

---

## 🚀 How to Run

### Option 1: Live Hot-Reload Server (Recommended)
Run the built-in local server:
```bash
python3 web-emulator/server.py
```
Open **[http://localhost:8080](http://localhost:8080)** in Chrome, Safari, or Firefox.

- **Auto-Sync**: Automatically detects saves in `src/main.ino` and updates the UI instantly in your browser.
- **Code Editor**: Edit the code right in the browser and click **Save to Disk** to update `src/main.ino`.

### Option 2: Standalone Mode (No Server Needed)
Double-click `web-emulator/index.html` or open in any browser:
```bash
open web-emulator/index.html
```

---

## 🎨 Features
1. **Accurate Hardware Simulation**:
   - Matches the physical 480 × 272 resolution of the JC4827W543.
   - Embedded retro pixel font (`Press Start 2P`).
   - Zoom scale: 1x (native), 1.5x, 2x (HiDPI).
   - CRT Scanline toggle & 20px Grid overlay for pixel alignment.
2. **Real-Time Sensor Simulator**:
   - Sliders for Temperature (`15°C` – `40°C`) and Humidity (`20%` – `95%`).
   - ESP32 `loop()` jitter simulation (toggleable random fluctuations every 2 seconds).
3. **Interactive Widget Inspector**:
   - Hover over any UI element to view its bounding box and `(x, y, w, h)` coordinates.
   - Live Object tree inspector.
4. **Theme / Palette Editor**:
   - Color pickers for `COLOR_BG`, `COLOR_TITLE`, `COLOR_SUN_BODY`, `COLOR_SUN_RAY`, `COLOR_SUN_LIGHT`, `COLOR_TEXT_TEMP`, and `COLOR_TEXT_HUM`.
