#!/usr/bin/env python3
import json
import base64
from pathlib import Path

BASE_DIR = Path(__file__).resolve().parent
PROJECT_DIR = BASE_DIR.parent
FONT_FILE = BASE_DIR / "PressStart2P-Regular.ttf"
SOURCE_FILE = PROJECT_DIR / "src" / "main.ino"

if FONT_FILE.exists():
    with open(FONT_FILE, "rb") as f:
        font_b64 = base64.b64encode(f.read()).decode("ascii")
else:
    font_b64 = ""

if SOURCE_FILE.exists():
    with open(SOURCE_FILE, "r", encoding="utf-8") as f:
        initial_code = f.read()
else:
    initial_code = "// src/main.ino not found"

template = """<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>JC4827W543 / ESP32-S3 Web Emulator</title>
  <style>
    @font-face {
      font-family: 'Press Start 2P';
      font-style: normal;
      font-weight: 400;
      src: url('PressStart2P-Regular.ttf') format('truetype'),
           url('data:font/truetype;charset=utf-8;base64,FONT_B64_PLACEHOLDER') format('truetype');
    }

    :root {
      --bg-dark: #0f1117;
      --panel-bg: #1a1d27;
      --card-bg: #232736;
      --border-color: #2e3449;
      --text-main: #f0f3f8;
      --text-dim: #9aa4bf;
      --accent-green: #10b981;
      --accent-blue: #3b82f6;
      --accent-orange: #f59e0b;
      --accent-red: #ef4444;
    }

    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }

    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
      background: var(--bg-dark);
      color: var(--text-main);
      display: flex;
      flex-direction: column;
      min-height: 100vh;
      overflow-x: hidden;
    }

    header {
      background: var(--panel-bg);
      border-bottom: 1px solid var(--border-color);
      padding: 12px 24px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      flex-wrap: wrap;
      gap: 12px;
      z-index: 100;
    }

    .header-left {
      display: flex;
      align-items: center;
      gap: 14px;
    }

    .logo-badge {
      background: #059669;
      color: white;
      font-weight: bold;
      font-size: 11px;
      padding: 4px 8px;
      border-radius: 6px;
      letter-spacing: 0.5px;
      text-transform: uppercase;
    }

    .title-group h1 {
      font-size: 16px;
      font-weight: 700;
      color: #fff;
    }

    .title-group p {
      font-size: 12px;
      color: var(--text-dim);
    }

    .header-right {
      display: flex;
      align-items: center;
      gap: 10px;
    }

    .badge-status {
      display: inline-flex;
      align-items: center;
      gap: 6px;
      font-size: 12px;
      padding: 5px 12px;
      border-radius: 20px;
      background: rgba(16, 185, 129, 0.15);
      border: 1px solid rgba(16, 185, 129, 0.3);
      color: #34d399;
    }

    .status-dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: #10b981;
      box-shadow: 0 0 8px #10b981;
      animation: pulse 2s infinite;
    }

    @keyframes pulse {
      0%, 100% { opacity: 1; transform: scale(1); }
      50% { opacity: 0.5; transform: scale(0.85); }
    }

    main {
      display: flex;
      flex: 1;
      gap: 20px;
      padding: 20px;
      max-width: 1600px;
      margin: 0 auto;
      width: 100%;
    }

    @media (max-width: 1150px) {
      main {
        flex-direction: column;
      }
    }

    .viewport-column {
      flex: 1;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 14px;
    }

    .viewport-toolbar {
      display: flex;
      align-items: center;
      justify-content: space-between;
      width: 100%;
      background: var(--panel-bg);
      border: 1px solid var(--border-color);
      border-radius: 8px;
      padding: 8px 14px;
      font-size: 13px;
    }

    .tool-group {
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .btn {
      background: var(--card-bg);
      color: var(--text-main);
      border: 1px solid var(--border-color);
      border-radius: 6px;
      padding: 6px 12px;
      font-size: 12px;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      gap: 6px;
      transition: all 0.15s ease;
      font-weight: 500;
    }

    .btn:hover {
      background: #2f3448;
      border-color: #434c69;
    }

    .btn.active {
      background: #2563eb;
      border-color: #3b82f6;
      color: white;
    }

    .btn-primary {
      background: #059669;
      border-color: #10b981;
      color: white;
    }
    .btn-primary:hover {
      background: #047857;
    }

    .device-bezel {
      background: linear-gradient(145deg, #26272b, #151618);
      padding: 18px 24px 22px 24px;
      border-radius: 28px;
      box-shadow: 0 25px 60px rgba(0,0,0,0.85), inset 0 1px 2px rgba(255,255,255,0.15), inset 0 -3px 6px rgba(0,0,0,0.9);
      border: 3px solid #1c1d20;
      position: relative;
      display: flex;
      flex-direction: column;
      align-items: center;
      user-select: none;
    }

    .bezel-screw {
      position: absolute;
      width: 14px;
      height: 14px;
      border-radius: 50%;
      background: radial-gradient(circle, #e5a93c 20%, #8c5315 70%, #3d2104 100%);
      border: 1.5px solid #2b1704;
      box-shadow: inset 0 1px 1px rgba(255,255,255,0.4), 0 2px 4px rgba(0,0,0,0.6);
      z-index: 10;
    }
    .bezel-screw::after {
      content: '';
      position: absolute;
      top: 50%; left: 50%;
      transform: translate(-50%, -50%);
      width: 5px; height: 5px;
      border-radius: 50%;
      background: #1a0f03;
      box-shadow: inset 0 1px 1px rgba(0,0,0,0.8);
    }
    .bezel-screw.tl { top: 10px; left: 12px; }
    .bezel-screw.tr { top: 10px; right: 12px; }
    .bezel-screw.bl { bottom: 10px; left: 12px; }
    .bezel-screw.br { bottom: 10px; right: 12px; }

    .bezel-header {
      display: none;
    }

    .screen-wrapper {
      position: relative;
      background: #000;
      border: 3px solid #8c5315;
      border-radius: 14px;
      box-shadow: 0 0 0 2px #261605, inset 0 0 15px rgba(0,0,0,0.95), 0 6px 16px rgba(0,0,0,0.8);
      overflow: hidden;
      transform-origin: top center;
      transition: transform 0.2s ease;
    }

    #lvgl-screen {
      width: 480px;
      height: 272px;
      position: relative;
      background-color: #0c0b09;
      overflow: hidden;
      image-rendering: pixelated;
      font-family: 'Press Start 2P', monospace;
    }

    /* Retro Radio Specific UI */
    .retro-radio-card {
      position: absolute;
      left: 14px;
      width: 452px;
      background: linear-gradient(180deg, #12100d 0%, #0a0907 100%);
      border: 2px solid #b87333;
      outline: 1.5px solid #452405;
      border-radius: 12px;
      box-shadow: inset 0 0 10px rgba(0,0,0,0.9), 0 2px 6px rgba(0,0,0,0.6);
      box-sizing: border-box;
    }

    .eq-visualizer {
      display: flex;
      gap: 3px;
      align-items: flex-end;
      height: 44px;
    }
    .eq-col {
      display: flex;
      flex-direction: column-reverse;
      gap: 2px;
      width: 8px;
    }
    .eq-seg {
      height: 5px;
      background: #FF6D00;
      border-radius: 1px;
      box-shadow: 0 0 3px rgba(255, 109, 0, 0.6);
      opacity: 0.12;
      transition: opacity 0.08s ease;
    }
    .eq-seg.active {
      opacity: 1;
    }

    /* Retro Flight Radar Specific UI */
    .radar-panel {
      position: absolute;
      background: #020804;
      border: 1.5px solid #00D26A;
      border-radius: 6px;
      box-shadow: 0 0 8px rgba(0,210,106,0.25), inset 0 0 6px rgba(0,210,106,0.08);
      box-sizing: border-box;
      overflow: hidden;
      font-family: 'Press Start 2P', monospace;
    }

    .radar-table-row {
      display: flex;
      align-items: center;
      height: 16px;
      padding: 0 4px;
      font-size: 6px;
      color: #00FF66;
      cursor: pointer;
      user-select: none;
      transition: background 0.1s ease;
      white-space: nowrap;
      border-bottom: 1px solid rgba(0, 77, 37, 0.4);
    }

    .radar-table-row:hover {
      background: rgba(0, 255, 102, 0.12);
    }

    .radar-table-row.selected {
      color: #FFA500 !important;
      background: rgba(255, 165, 0, 0.15) !important;
      border-left: 2px solid #FFA500;
      font-weight: bold;
      text-shadow: 0 0 4px rgba(255, 165, 0, 0.5);
    }

    #radar-flight-list::-webkit-scrollbar {
      width: 6px;
    }
    #radar-flight-list::-webkit-scrollbar-track {
      background: #020d05;
      border-radius: 3px;
    }
    #radar-flight-list::-webkit-scrollbar-thumb {
      background: #00AA44;
      border-radius: 3px;
      box-shadow: 0 0 4px rgba(0, 170, 68, 0.6);
    }

    .lv-obj {
      position: absolute;
      box-sizing: border-box;
      pointer-events: auto;
      transition: outline 0.1s;
    }

    .lv-obj.inspect-hover {
      outline: 2px dashed #38bdf8 !important;
      outline-offset: 1px;
      z-index: 9999;
    }

    .lv-btn {
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      user-select: none;
      transition: filter 0.1s ease;
    }
    .lv-btn:hover {
      filter: brightness(1.15);
    }
    .lv-btn:active {
      filter: brightness(0.85);
    }

    .lv-label {
      white-space: pre;
      line-height: 1;
      user-select: none;
      pointer-events: none;
      font-family: 'Press Start 2P', monospace;
      font-size: 10px;
    }

    .scanlines {
      position: absolute;
      inset: 0;
      pointer-events: none;
      background: linear-gradient(rgba(18, 16, 16, 0) 50%, rgba(0, 0, 0, 0.25) 50%);
      background-size: 100% 4px;
      z-index: 1000;
      opacity: 0.6;
      display: none;
    }
    .scanlines.show {
      display: block;
    }

    .grid-overlay {
      position: absolute;
      inset: 0;
      pointer-events: none;
      background-size: 20px 20px;
      background-image:
        linear-gradient(to right, rgba(255, 255, 255, 0.05) 1px, transparent 1px),
        linear-gradient(to bottom, rgba(255, 255, 255, 0.05) 1px, transparent 1px);
      z-index: 999;
      display: none;
    }
    .grid-overlay.show {
      display: block;
    }

    .coords-bar {
      margin-top: 10px;
      font-size: 11px;
      font-family: monospace;
      color: #636e8b;
      display: flex;
      gap: 16px;
    }

    .control-column {
      width: 480px;
      background: var(--panel-bg);
      border: 1px solid var(--border-color);
      border-radius: 12px;
      display: flex;
      flex-direction: column;
      overflow: hidden;
    }

    .tab-bar {
      display: flex;
      border-bottom: 1px solid var(--border-color);
      background: #141721;
    }

    .tab-btn {
      flex: 1;
      padding: 12px 10px;
      background: transparent;
      border: none;
      color: var(--text-dim);
      font-size: 12px;
      font-weight: 600;
      cursor: pointer;
      border-bottom: 2px solid transparent;
      transition: all 0.15s;
      text-align: center;
    }

    .tab-btn:hover {
      color: var(--text-main);
      background: rgba(255, 255, 255, 0.02);
    }

    .tab-btn.active {
      color: #38bdf8;
      border-bottom-color: #38bdf8;
      background: var(--panel-bg);
    }

    .tab-pane {
      padding: 18px;
      display: none;
      flex-direction: column;
      gap: 16px;
      overflow-y: auto;
      max-height: calc(100vh - 180px);
    }

    .tab-pane.active {
      display: flex;
    }

    .card {
      background: var(--card-bg);
      border: 1px solid var(--border-color);
      border-radius: 8px;
      padding: 14px;
    }

    .card-title {
      font-size: 12px;
      font-weight: 600;
      color: var(--text-dim);
      text-transform: uppercase;
      letter-spacing: 0.5px;
      margin-bottom: 12px;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }

    .view-switcher-row {
      display: flex;
      gap: 10px;
      margin-bottom: 12px;
    }

    .view-btn {
      flex: 1;
      padding: 10px;
      border-radius: 6px;
      border: 1px solid var(--border-color);
      background: #12141c;
      color: var(--text-main);
      font-weight: 600;
      font-size: 12px;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 6px;
      transition: all 0.15s ease;
    }
    .view-btn.active {
      background: #2563eb;
      border-color: #3b82f6;
      color: white;
    }

    .sensor-row {
      display: flex;
      flex-direction: column;
      gap: 6px;
      margin-bottom: 14px;
    }

    .sensor-header {
      display: flex;
      justify-content: space-between;
      font-size: 13px;
      font-weight: 500;
    }

    .sensor-value {
      font-family: 'Press Start 2P', monospace;
      font-size: 11px;
      padding: 2px 6px;
      border-radius: 4px;
      background: #12141c;
    }

    input[type=range] {
      width: 100%;
      height: 6px;
      background: #12141c;
      border-radius: 3px;
      outline: none;
      -webkit-appearance: none;
    }

    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 16px;
      height: 16px;
      border-radius: 50%;
      background: #3b82f6;
      cursor: pointer;
      border: 2px solid #fff;
      box-shadow: 0 0 5px rgba(0,0,0,0.5);
    }

    .station-grid {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 8px;
      margin-top: 10px;
    }

    .station-btn {
      background: #12141c;
      border: 1px solid var(--border-color);
      color: var(--text-main);
      padding: 6px 8px;
      border-radius: 6px;
      font-size: 11px;
      cursor: pointer;
      text-align: left;
      transition: all 0.15s;
    }
    .station-btn:hover {
      background: #2a3044;
      border-color: #3b82f6;
    }
    .station-btn.active {
      background: #1e3a8a;
      border-color: #3b82f6;
      color: #93c5fd;
    }

    .tune-controls {
      display: flex;
      gap: 8px;
      margin-top: 8px;
    }
    .tune-controls .btn {
      flex: 1;
      justify-content: center;
    }

    .color-grid {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 12px;
    }

    .color-item {
      display: flex;
      flex-direction: column;
      gap: 6px;
    }

    .color-item label {
      font-size: 11px;
      font-weight: 500;
      color: var(--text-dim);
      font-family: monospace;
    }

    .color-item input[type=color] {
      width: 100%;
      height: 34px;
      border-radius: 6px;
      border: 1px solid var(--border-color);
      background: #12141c;
      cursor: pointer;
      padding: 2px;
    }

    #code-editor {
      width: 100%;
      height: 480px;
      background: #11131a;
      border: 1px solid var(--border-color);
      border-radius: 8px;
      color: #e2e8f0;
      font-family: Menlo, Monaco, Consolas, "Courier New", monospace;
      font-size: 12px;
      line-height: 1.5;
      padding: 12px;
      resize: vertical;
      outline: none;
      tab-size: 4;
    }
    #code-editor:focus {
      border-color: #3b82f6;
    }

    .inspector-tree {
      display: flex;
      flex-direction: column;
      gap: 4px;
      max-height: 380px;
      overflow-y: auto;
    }

    .tree-node {
      font-family: monospace;
      font-size: 11px;
      padding: 6px 10px;
      border-radius: 4px;
      background: #161822;
      border: 1px solid var(--border-color);
      display: flex;
      justify-content: space-between;
      cursor: pointer;
      transition: all 0.1s ease;
    }
    .tree-node:hover {
      background: #252a3d;
      border-color: #38bdf8;
    }
    .tree-node-title {
      color: #93c5fd;
    }
    .tree-node-meta {
      color: #64748b;
    }

    .toast {
      position: fixed;
      bottom: 24px;
      right: 24px;
      background: #065f46;
      color: #ecfdf5;
      border: 1px solid #059669;
      padding: 10px 18px;
      border-radius: 8px;
      font-size: 13px;
      box-shadow: 0 10px 25px rgba(0,0,0,0.5);
      transform: translateY(100px);
      opacity: 0;
      transition: all 0.25s cubic-bezier(0.16, 1, 0.3, 1);
      z-index: 10000;
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .toast.show {
      transform: translateY(0);
      opacity: 1;
    }
  </style>
</head>
<body>

  <header>
    <div class="header-left">
      <span class="logo-badge">ESP32-S3</span>
      <div class="title-group">
        <h1>JC4827W543 UI Emulator</h1>
        <p>480 &times; 272 NV3041A IPS &bull; LVGL 8.3.11 Environment</p>
      </div>
    </div>
    <div class="header-right">
      <div id="sync-status" class="badge-status">
        <span class="status-dot"></span>
        <span id="sync-text">Checking local server...</span>
      </div>
    </div>
  </header>

  <main>
    <!-- Viewport Column -->
    <div class="viewport-column">
      <div class="viewport-toolbar">
        <div class="tool-group">
          <span>Zoom:</span>
          <button class="btn" onclick="setZoom(1)">1x (480p)</button>
          <button class="btn active" onclick="setZoom(1.5)">1.5x</button>
          <button class="btn" onclick="setZoom(2)">2x (HiDPI)</button>
        </div>
        <div class="tool-group">
          <button id="btn-grid" class="btn" onclick="toggleGrid()">Grid</button>
          <button id="btn-scanlines" class="btn" onclick="toggleScanlines()">CRT Scanlines</button>
          <button id="btn-inspect" class="btn" onclick="toggleInspect()">Inspect Elements</button>
        </div>
      </div>

      <!-- Physical Bezel -->
      <div class="device-bezel">
        <div class="bezel-screw tl"></div>
        <div class="bezel-screw tr"></div>
        <div class="bezel-screw bl"></div>
        <div class="bezel-screw br"></div>

        <div class="bezel-header">
          <span class="bezel-label">JC4827W543 / QSPI-IPS</span>
          <div class="bezel-led-group">
            <span>PWR</span>
            <div class="power-led"></div>
          </div>
        </div>

        <div id="screen-wrapper" class="screen-wrapper" style="transform: scale(1.5); margin-bottom: 70px;">
          <div id="lvgl-screen"></div>
          <div id="grid-overlay" class="grid-overlay"></div>
          <div id="scanlines" class="scanlines"></div>
        </div>

        <div class="coords-bar">
          <span id="mouse-coords">X: 0, Y: 0</span>
          <span>Resolution: 480 &times; 272</span>
          <span id="fps-counter">Loop: Active</span>
        </div>
      </div>
    </div>

    <!-- Control Column -->
    <div class="control-column">
      <div class="tab-bar">
        <button class="tab-btn active" onclick="openTab('controls')">Tuner &amp; Sensors</button>
        <button class="tab-btn" onclick="openTab('editor')">Code &amp; Live Reload</button>
        <button class="tab-btn" onclick="openTab('palette')">Colors</button>
        <button class="tab-btn" onclick="openTab('tree')">Objects</button>
      </div>

      <!-- Tab: Controls (Radio & Sensors) -->
      <div id="tab-controls" class="tab-pane active">
        <div class="card">
          <div class="card-title">View Mode</div>
          <div class="view-switcher-row">
            <button id="vbtn-cactus" class="view-btn" onclick="switchView('CACTUS')">🌵 Cactus Monitor</button>
            <button id="vbtn-radio" class="view-btn" onclick="switchView('RADIO')">📻 Retro Radio Tuner</button>
            <button id="vbtn-radar" class="view-btn active" onclick="switchView('RADAR')">✈ Flight Radar</button>
          </div>
        </div>

        <!-- Flight Radar Controls Card -->
        <div class="card" id="radar-controls-card">
          <div class="card-title">Penang Flight Radar (WMKP)</div>
          <div style="display: flex; gap: 8px; margin-bottom: 10px;">
            <button id="btn-mode-live" class="btn btn-primary" style="flex: 1;" onclick="setRadarDataSource('live')">📡 Live ADS-B</button>
            <button id="btn-mode-demo" class="btn" style="flex: 1;" onclick="setRadarDataSource('demo')">🎯 Demo Snapshot</button>
          </div>
          <div class="sensor-row">
            <div class="sensor-header">
              <span>Radar Range</span>
              <span id="radar-range-display" class="sensor-value" style="color: #00FF66;">15 NM</span>
            </div>
            <input type="range" id="radar-range-slider" min="10" max="50" step="5" value="15" oninput="onRadarRangeChange(this.value)">
          </div>
          <div style="display: flex; justify-content: space-between; align-items: center; margin-top: 10px; font-size: 11px; color: var(--text-dim);">
            <span id="radar-status-badge">Status: <b style="color: #00FF66;">Live ADS-B</b></span>
            <button class="btn" style="padding: 3px 8px; font-size: 10px;" onclick="refreshFlightData(true)">↻ Fetch Now</button>
          </div>
        </div>

        <!-- Radio Tuner Controls -->
        <div class="card">
          <div class="card-title">Malaysian FM Radio Tuner</div>
          <div class="sensor-row">
            <div class="sensor-header">
              <span>Frequency</span>
              <span id="radio-display" class="sensor-value" style="color: #00e676;">89.9 MHz</span>
            </div>
            <input type="range" id="radio-slider" min="88.0" max="108.0" step="0.1" value="89.9" oninput="onRadioSliderChange(this.value)">
          </div>

          <div class="tune-controls">
            <button class="btn" onclick="tuneStep(-0.1)">&larr; -0.1 MHz</button>
            <button class="btn" onclick="tuneStep(0.1)">+0.1 MHz &rarr;</button>
          </div>

          <div style="margin-top: 12px; font-size: 11px; color: var(--text-dim); font-weight: 600;">STATION PRESETS:</div>
          <div class="station-grid" id="station-presets"></div>
        </div>

        <!-- Sensor Controls -->
        <div class="card">
          <div class="card-title">Sensor Simulator (DHT11 &amp; Soil)</div>
          <div class="sensor-row">
            <div class="sensor-header">
              <span>Temperature</span>
              <span id="temp-display" class="sensor-value" style="color: #ff5252;">24&deg;C</span>
            </div>
            <input type="range" id="temp-slider" min="15" max="40" step="1" value="24" oninput="onTempSliderChange(this.value)">
          </div>

          <div class="sensor-row">
            <div class="sensor-header">
              <span>Humidity</span>
              <span id="humid-display" class="sensor-value" style="color: #81c784;">42%</span>
            </div>
            <input type="range" id="humid-slider" min="20" max="95" step="1" value="42" oninput="onHumidSliderChange(this.value)">
          </div>

          <div style="margin-top: 10px; padding-top: 10px; border-top: 1px solid var(--border-color);">
            <label style="display: flex; align-items: center; gap: 8px; font-size: 12px; color: var(--text-dim); cursor: pointer;">
              <input type="checkbox" id="sim-loop-check" checked onchange="toggleSimLoop(this.checked)">
              <span>Run ESP32 loop() jitter (fluctuate every 2s)</span>
            </label>
          </div>
        </div>
      </div>

      <!-- Tab: Code Live Editor -->
      <div id="tab-editor" class="tab-pane">
        <div style="display: flex; justify-content: space-between; align-items: center;">
          <span style="font-size: 12px; color: var(--text-dim);" id="file-path-label">src/main.ino</span>
          <div style="display: flex; gap: 8px;">
            <button class="btn btn-primary" onclick="applyEditorCode()">Re-render</button>
            <button id="btn-save-disk" class="btn" onclick="saveCodeToDisk()">Save to Disk</button>
          </div>
        </div>
        <textarea id="code-editor" spellcheck="false"></textarea>
        <div id="editor-error" style="display: none; background: rgba(239, 68, 68, 0.15); border: 1px solid rgba(239, 68, 68, 0.4); color: #f87171; padding: 10px; border-radius: 6px; font-size: 11px; font-family: monospace;"></div>
      </div>

      <!-- Tab: Colors -->
      <div id="tab-palette" class="tab-pane">
        <div class="card">
          <div class="card-title">Theme #define Palette</div>
          <div class="color-grid">
            <div class="color-item">
              <label>COLOR_BG</label>
              <input type="color" id="picker-COLOR_BG" value="#121212" onchange="updatePaletteColor('COLOR_BG', this.value)">
            </div>
            <div class="color-item">
              <label>COLOR_NAV_BG</label>
              <input type="color" id="picker-COLOR_NAV_BG" value="#1a1a22" onchange="updatePaletteColor('COLOR_NAV_BG', this.value)">
            </div>
            <div class="color-item">
              <label>COLOR_TITLE</label>
              <input type="color" id="picker-COLOR_TITLE" value="#64b5f6" onchange="updatePaletteColor('COLOR_TITLE', this.value)">
            </div>
            <div class="color-item">
              <label>COLOR_SUN_BODY</label>
              <input type="color" id="picker-COLOR_SUN_BODY" value="#ffb300" onchange="updatePaletteColor('COLOR_SUN_BODY', this.value)">
            </div>
            <div class="color-item">
              <label>COLOR_SUN_RAY</label>
              <input type="color" id="picker-COLOR_SUN_RAY" value="#ff8f00" onchange="updatePaletteColor('COLOR_SUN_RAY', this.value)">
            </div>
            <div class="color-item">
              <label>COLOR_SUN_LIGHT</label>
              <input type="color" id="picker-COLOR_SUN_LIGHT" value="#ffe082" onchange="updatePaletteColor('COLOR_SUN_LIGHT', this.value)">
            </div>
            <div class="color-item">
              <label>COLOR_TEXT_TEMP</label>
              <input type="color" id="picker-COLOR_TEXT_TEMP" value="#ff5252" onchange="updatePaletteColor('COLOR_TEXT_TEMP', this.value)">
            </div>
            <div class="color-item">
              <label>COLOR_TEXT_HUM</label>
              <input type="color" id="picker-COLOR_TEXT_HUM" value="#81c784" onchange="updatePaletteColor('COLOR_TEXT_HUM', this.value)">
            </div>
          </div>
        </div>
      </div>

      <!-- Tab: Objects -->
      <div id="tab-tree" class="tab-pane">
        <div class="card">
          <div class="card-title">Instantiated LVGL Widgets</div>
          <div id="inspector-tree-list" class="inspector-tree"></div>
        </div>
      </div>
    </div>
  </main>

  <div id="toast" class="toast">
    <span id="toast-msg">Saved!</span>
  </div>

  <script>
    let currentSourceCode = INITIAL_CODE_PLACEHOLDER;
    let lastMtime = 0;
    let serverAvailable = false;
    let inspectMode = false;

    let simTemp = 24.0;
    let simHumid = 42.0;
    let simRadioFreq = 89.9;
    let currentViewMode = 'RADAR';
    let simLoopActive = true;

    const STATIONS = [
      { freq: 88.5,  name: "NASIONAL FM",  genre: "BERITA & HIBURAN RTM" },
      { freq: 89.9,  name: "BFM 89.9",     genre: "BUSINESS & CURRENT AFFAIRS" },
      { freq: 90.3,  name: "TRAXX FM",     genre: "ENGLISH URBAN & TOP HITS" },
      { freq: 97.6,  name: "HOT FM",       genre: "MUZIK PALING HANGAT" },
      { freq: 98.3,  name: "RADIO KLASIK", genre: "LAGU KLASIK & NOSTALGIA" },
      { freq: 101.3, name: "KOOL 101",     genre: "SEMASA & KONTEMPORARI" },
      { freq: 105.3, name: "SURIA FM",     genre: "SEGALANYA HIBURAN" }
    ];

    let screenElement = document.getElementById('lvgl-screen');
    let registeredObjects = [];

    // Global UI references
    let cactus_view = null;
    let radio_view = null;
    let radar_view = null;
    let btn_cactus = null;
    let btn_radio = null;
    let btn_radar = null;
    let lbl_temp_val = null;
    let lbl_humid_val = null;
    let r_freq_val = null;
    let r_station_name = null;
    let r_genre_name = null;
    let r_status_badge = null;
    let r_needle = null;

    // Flight Radar state
    let radarSweepAngle = 0;
    let radarRadius = 15; // in Nautical Miles
    let radarDataSource = 'live'; // 'live' or 'demo'
    let radarAnimationId = null;
    let radarFetchTimer = null;
    let currentFlights = [];
    let selectedFlightCallsign = 'AXM429';
    let metarInfo = {
      windDir: 40,
      windSpeed: 10,
      temp: 31,
      clouds: 'FEW020',
      runway: '04',
      raw: 'WMKP 070830Z 04010KT 9999 FEW020 31/25 Q1007'
    };

    const DEMO_FLIGHTS = [
      { flight: "AXM429", origin: "KUL", dest: "PEN", alt_baro: 4200, gs: 250, track: 42, dst: 8.7, dir: 222, baro_rate: -1200, status: "ARRIVING RWY 04" },
      { flight: "TRZ136", origin: "SIN", dest: "PEN", alt_baro: 3300, gs: 210, track: 48, dst: 10.9, dir: 228, baro_rate: -1000, status: "ARRIVING RWY 04" },
      { flight: "SIA324", origin: "SIN", dest: "PEN", alt_baro: 5600, gs: 260, track: 63, dst: 14.2, dir: 105, baro_rate: -800, status: "ARRIVING" },
      { flight: "MAS738", origin: "KCH", dest: "PEN", alt_baro: 8000, gs: 280, track: 219, dst: 16.1, dir: 35, baro_rate: -950, status: "ARRIVING" },
      { flight: "FDV612", origin: "DMK", dest: "PEN", alt_baro: 12000, gs: 320, track: 289, dst: 18.4, dir: 125, baro_rate: 1500, status: "ENROUTE" },
      { flight: "MXD318", origin: "SZB", dest: "PEN", alt_baro: 6800, gs: 240, track: 350, dst: 21.5, dir: 175, baro_rate: -600, status: "ENROUTE" },
      { flight: "BAO201", origin: "BKK", dest: "PEN", alt_baro: 15000, gs: 380, track: 185, dst: 24.2, dir: 10, baro_rate: -1500, status: "ENROUTE" },
      { flight: "CLX7334", origin: "LUX", dest: "PEN", alt_baro: 9200, gs: 290, track: 172, dst: 27.5, dir: 195, baro_rate: 2800, status: "DEPARTING" }
    ];

    const LV_ALIGN_TOP_LEFT = 'TOP_LEFT';
    const LV_ALIGN_TOP_MID = 'TOP_MID';
    const LV_ALIGN_TOP_RIGHT = 'TOP_RIGHT';
    const LV_ALIGN_BOTTOM_LEFT = 'BOTTOM_LEFT';
    const LV_ALIGN_BOTTOM_MID = 'BOTTOM_MID';
    const LV_ALIGN_BOTTOM_RIGHT = 'BOTTOM_RIGHT';
    const LV_ALIGN_LEFT_MID = 'LEFT_MID';
    const LV_ALIGN_RIGHT_MID = 'RIGHT_MID';
    const LV_ALIGN_CENTER = 'CENTER';

    const LV_PART_MAIN = 0;
    const LV_OPA_TRANSP = 0;
    const LV_OPA_COVER = 255;
    const LV_OBJ_FLAG_SCROLLABLE = 1;
    const LV_OBJ_FLAG_HIDDEN = 2;
    const LV_BORDER_SIDE_RIGHT = 1;
    const LV_EVENT_CLICKED = 'clicked';

    function lv_color_hex(hex) {
      if (typeof hex === 'number') {
        return '#' + hex.toString(16).padStart(6, '0');
      }
      return '#' + String(hex).replace(/^0x|^#/, '').padStart(6, '0');
    }

    function lv_color_make(r, g, b) {
      return `rgb(${r}, ${g}, ${b})`;
    }

    const press_start_2p_16 = "16px 'Press Start 2P', monospace";
    const press_start_2p_24 = "24px 'Press Start 2P', monospace";
    const press_start_2p_32 = "32px 'Press Start 2P', monospace";

    class LVObject {
      constructor(parent, isLabel = false, isButton = false) {
        this.isLabel = isLabel;
        this.isButton = isButton;
        this.parent = parent;
        this.children = [];
        this.x = 0;
        this.y = 0;
        this.width = isLabel ? 0 : 100;
        this.height = isLabel ? 0 : 50;
        this.alignType = LV_ALIGN_TOP_LEFT;
        this.alignX = 0;
        this.alignY = 0;
        this.name = isLabel ? 'Label' : (isButton ? 'Button' : 'Container');
        this.text = '';
        this.hidden = false;
        this.eventListeners = {};

        this.dom = document.createElement('div');
        this.dom.className = 'lv-obj' + (isLabel ? ' lv-label' : '') + (isButton ? ' lv-btn' : '');
        
        if (parent && parent.dom) {
          if (parent.children) parent.children.push(this);
          parent.dom.appendChild(this.dom);
        } else {
          screenElement.appendChild(this.dom);
        }

        this.dom.__lvObj = this;

        this.dom.addEventListener('click', (e) => {
          if (this.eventListeners[LV_EVENT_CLICKED]) {
            e.stopPropagation();
            this.eventListeners[LV_EVENT_CLICKED].forEach(cb => cb({ target: this }));
          }
        });

        this.dom.addEventListener('mouseenter', (e) => {
          if (inspectMode) {
            e.stopPropagation();
            this.dom.classList.add('inspect-hover');
            showCoords(this);
          }
        });
        this.dom.addEventListener('mouseleave', () => {
          this.dom.classList.remove('inspect-hover');
        });

        registeredObjects.push(this);
        this.applyStyle();
      }

      applyStyle() {
        this.recomputePosition();
        this.dom.style.left = this.x + 'px';
        this.dom.style.top = this.y + 'px';
        if (this.width > 0) this.dom.style.width = this.width + 'px';
        if (this.height > 0) this.dom.style.height = this.height + 'px';
        this.dom.style.display = this.hidden ? 'none' : 'block';
      }

      recomputePosition() {
        let parentW = this.parent && this.parent.width ? this.parent.width : 480;
        let parentH = this.parent && this.parent.height ? this.parent.height : 272;

        let w = this.width;
        let h = this.height;

        if (this.isLabel) {
          if (this.dom && this.dom.offsetWidth > 0) {
            w = this.dom.offsetWidth;
            h = this.dom.offsetHeight;
          } else if (this.text) {
            let fontStr = (this.dom && this.dom.style.font) ? this.dom.style.font : '';
            let charSize = 10;
            if (fontStr.includes('32px')) charSize = 32;
            else if (fontStr.includes('24px')) charSize = 24;
            else if (fontStr.includes('16px')) charSize = 16;
            w = this.text.length * charSize;
            h = charSize;
          }
          this.width = w;
          this.height = h;
        }

        if (this.alignType === LV_ALIGN_TOP_LEFT) {
          this.x = this.alignX;
          this.y = this.alignY;
        } else if (this.alignType === LV_ALIGN_CENTER) {
          this.x = Math.round((parentW - w) / 2 + this.alignX);
          this.y = Math.round((parentH - h) / 2 + this.alignY);
        } else if (this.alignType === LV_ALIGN_TOP_MID) {
          this.x = Math.round((parentW - w) / 2 + this.alignX);
          this.y = this.alignY;
        } else if (this.alignType === LV_ALIGN_TOP_RIGHT) {
          this.x = Math.round(parentW - w + this.alignX);
          this.y = this.alignY;
        } else if (this.alignType === LV_ALIGN_BOTTOM_LEFT) {
          this.x = this.alignX;
          this.y = Math.round(parentH - h + this.alignY);
        } else if (this.alignType === LV_ALIGN_BOTTOM_MID) {
          this.x = Math.round((parentW - w) / 2 + this.alignX);
          this.y = Math.round(parentH - h + this.alignY);
        } else if (this.alignType === LV_ALIGN_BOTTOM_RIGHT) {
          this.x = Math.round(parentW - w + this.alignX);
          this.y = Math.round(parentH - h + this.alignY);
        }
      }
    }

    function lv_scr_act() {
      return {
        dom: screenElement,
        children: [],
        width: 480,
        height: 272
      };
    }

    function lv_obj_create(parent) {
      return new LVObject(parent, false, false);
    }

    function lv_btn_create(parent) {
      return new LVObject(parent, false, true);
    }

    function lv_label_create(parent) {
      return new LVObject(parent, true, false);
    }

    function lv_obj_set_pos(obj, x, y) {
      if (!obj) return;
      obj.alignType = LV_ALIGN_TOP_LEFT;
      obj.alignX = x;
      obj.alignY = y;
      obj.applyStyle();
    }

    function lv_obj_set_x(obj, x) {
      if (!obj) return;
      obj.x = x;
      obj.dom.style.left = x + 'px';
    }

    function lv_obj_set_y(obj, y) {
      if (!obj) return;
      obj.y = y;
      obj.dom.style.top = y + 'px';
    }

    function lv_obj_set_size(obj, w, h) {
      if (!obj) return;
      obj.width = w;
      obj.height = h;
      obj.applyStyle();
    }

    function lv_obj_align(obj, align, x_off, y_off) {
      if (!obj) return;
      obj.alignType = align;
      obj.alignX = x_off;
      obj.alignY = y_off;
      obj.applyStyle();
    }

    function lv_obj_center(obj) {
      lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
    }

    function lv_label_set_text(obj, text) {
      if (!obj || !obj.dom) return;
      obj.text = String(text);
      obj.dom.textContent = obj.text;
      obj.applyStyle();
      requestAnimationFrame(() => {
        if (obj.dom && obj.dom.offsetWidth > 0) {
          obj.width = obj.dom.offsetWidth;
          obj.height = obj.dom.offsetHeight;
        }
        obj.recomputePosition();
        obj.dom.style.left = obj.x + 'px';
        obj.dom.style.top = obj.y + 'px';
      });
    }

    function lv_obj_set_style_bg_color(obj, color, part) {
      if (!obj || !obj.dom) return;
      obj.dom.style.backgroundColor = color;
    }

    function lv_obj_set_style_bg_opa(obj, opa, part) {
      if (!obj || !obj.dom) return;
      if (opa === LV_OPA_TRANSP || opa === 0) {
        obj.dom.style.backgroundColor = 'transparent';
      }
    }

    function lv_obj_set_style_border_color(obj, color, part) {
      if (!obj || !obj.dom) return;
      obj.dom.style.borderColor = color;
      obj.dom.style.borderStyle = 'solid';
    }

    function lv_obj_set_style_border_width(obj, width, part) {
      if (!obj || !obj.dom) return;
      obj.dom.style.borderWidth = width + 'px';
      obj.dom.style.borderStyle = width > 0 ? 'solid' : 'none';
    }

    function lv_obj_set_style_border_side(obj, side, part) {
      if (!obj || !obj.dom) return;
      if (side === LV_BORDER_SIDE_RIGHT) {
        obj.dom.style.borderTopWidth = '0px';
        obj.dom.style.borderBottomWidth = '0px';
        obj.dom.style.borderLeftWidth = '0px';
      }
    }

    function lv_obj_set_style_radius(obj, radius, part) {
      if (!obj || !obj.dom) return;
      obj.dom.style.borderRadius = radius + 'px';
    }

    function lv_obj_set_style_pad_all(obj, pad, part) {
      if (!obj || !obj.dom) return;
      obj.dom.style.padding = pad + 'px';
    }

    function lv_obj_set_style_text_color(obj, color, part) {
      if (!obj || !obj.dom) return;
      obj.dom.style.color = color;
    }

    function lv_obj_set_style_text_font(obj, font, part) {
      if (!obj || !obj.dom) return;
      obj.dom.style.font = font;
    }

    function lv_obj_clear_flag(obj, flag) {
      if (!obj || !obj.dom) return;
      if (flag === LV_OBJ_FLAG_SCROLLABLE) {
        obj.dom.style.overflow = 'hidden';
      } else if (flag === LV_OBJ_FLAG_HIDDEN) {
        obj.hidden = false;
        obj.dom.style.display = 'block';
      }
    }

    function lv_obj_add_flag(obj, flag) {
      if (!obj || !obj.dom) return;
      if (flag === LV_OBJ_FLAG_HIDDEN) {
        obj.hidden = true;
        obj.dom.style.display = 'none';
      }
    }

    function lv_obj_add_event_cb(obj, cb, eventType, userData) {
      if (!obj) return;
      if (!obj.eventListeners[eventType]) {
        obj.eventListeners[eventType] = [];
      }
      obj.eventListeners[eventType].push(cb);
    }

    function lv_event_get_target(e) {
      return e.target;
    }

    function extractColors(code) {
      let colors = {
        COLOR_BG: '#121212',
        COLOR_NAV_BG: '#1A1A22',
        COLOR_TITLE: '#64B5F6',
        COLOR_SUN_BODY: '#FFB300',
        COLOR_SUN_RAY: '#FF8F00',
        COLOR_SUN_LIGHT: '#FFE082',
        COLOR_TEXT_TEMP: '#FF5252',
        COLOR_TEXT_HUM: '#81C784'
      };

      let re = /#define\s+(COLOR_\w+)\s+lv_color_hex\((0x[0-9a-fA-F]+)\)/g;
      let m;
      while ((m = re.exec(code)) !== null) {
        colors[m[1]] = lv_color_hex(m[2]);
      }
      return colors;
    }

    function renderUI(colors) {
      document.getElementById('editor-error').style.display = 'none';
      screenElement.innerHTML = '';
      registeredObjects = [];

      // Update color pickers
      for (const [key, val] of Object.entries(colors)) {
        let picker = document.getElementById('picker-' + key);
        if (picker) picker.value = val;
      }

      let screen = lv_scr_act();
      screenElement.style.backgroundColor = colors.COLOR_BG;

      // --- VIEW 1: CACTUS MONITOR (Full Screen) ---
      cactus_view = lv_obj_create(screen);
      lv_obj_set_pos(cactus_view, 0, 0);
      lv_obj_set_size(cactus_view, 480, 272);
      lv_obj_set_style_bg_opa(cactus_view, LV_OPA_TRANSP, LV_PART_MAIN);
      lv_obj_set_style_border_width(cactus_view, 0, LV_PART_MAIN);
      lv_obj_clear_flag(cactus_view, LV_OBJ_FLAG_SCROLLABLE);

      let title = lv_label_create(cactus_view);
      lv_label_set_text(title, "CACTUS MONITOR");
      lv_obj_set_style_text_color(title, colors.COLOR_TITLE, LV_PART_MAIN);
      lv_obj_set_style_text_font(title, press_start_2p_16, LV_PART_MAIN);
      lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 22);

      // Pixel Art Sun
      let sun_cont = lv_obj_create(cactus_view);
      lv_obj_set_pos(sun_cont, 50, 68);
      lv_obj_set_size(sun_cont, 150, 150);
      lv_obj_set_style_bg_opa(sun_cont, LV_OPA_TRANSP, LV_PART_MAIN);
      lv_obj_set_style_border_width(sun_cont, 0, LV_PART_MAIN);
      lv_obj_clear_flag(sun_cont, LV_OBJ_FLAG_SCROLLABLE);

      let body = lv_obj_create(sun_cont);
      lv_obj_set_size(body, 74, 74);
      lv_obj_align(body, LV_ALIGN_CENTER, 0, 0);
      lv_obj_set_style_bg_color(body, colors.COLOR_SUN_BODY, LV_PART_MAIN);
      lv_obj_set_style_bg_opa(body, LV_OPA_COVER, LV_PART_MAIN);
      lv_obj_set_style_border_color(body, '#000000', LV_PART_MAIN);
      lv_obj_set_style_border_width(body, 3, LV_PART_MAIN);
      lv_obj_set_style_radius(body, 37, LV_PART_MAIN);

      let highlight = lv_obj_create(body);
      lv_obj_set_size(highlight, 20, 20);
      lv_obj_set_pos(highlight, 10, 10);
      lv_obj_set_style_bg_color(highlight, colors.COLOR_SUN_LIGHT, LV_PART_MAIN);
      lv_obj_set_style_bg_opa(highlight, LV_OPA_COVER, LV_PART_MAIN);
      lv_obj_set_style_border_width(highlight, 0, LV_PART_MAIN);
      lv_obj_set_style_radius(highlight, 10, LV_PART_MAIN);

      const rays = [
        { x_off: 0, y_off: -54, w: 12, h: 20 },
        { x_off: 0, y_off: 54, w: 12, h: 20 },
        { x_off: -54, y_off: 0, w: 20, h: 12 },
        { x_off: 54, y_off: 0, w: 20, h: 12 },
        { x_off: -38, y_off: -38, w: 14, h: 14 },
        { x_off: 38, y_off: -38, w: 14, h: 14 },
        { x_off: -38, y_off: 38, w: 14, h: 14 },
        { x_off: 38, y_off: 38, w: 14, h: 14 }
      ];

      for (const r of rays) {
        let ray = lv_obj_create(sun_cont);
        lv_obj_set_size(ray, r.w, r.h);
        lv_obj_align(ray, LV_ALIGN_CENTER, r.x_off, r.y_off);
        lv_obj_set_style_bg_color(ray, colors.COLOR_SUN_RAY, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(ray, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_color(ray, '#000000', LV_PART_MAIN);
        lv_obj_set_style_border_width(ray, 2, LV_PART_MAIN);
        lv_obj_set_style_radius(ray, 2, LV_PART_MAIN);
      }

      lbl_temp_val = lv_label_create(cactus_view);
      lv_label_set_text(lbl_temp_val, Math.round(simTemp) + "°C");
      lv_obj_set_style_text_color(lbl_temp_val, colors.COLOR_TEXT_TEMP, LV_PART_MAIN);
      lv_obj_set_style_text_font(lbl_temp_val, press_start_2p_32, LV_PART_MAIN);
      lv_obj_align(lbl_temp_val, LV_ALIGN_TOP_LEFT, 235, 95);

      lbl_humid_val = lv_label_create(cactus_view);
      lv_label_set_text(lbl_humid_val, Math.round(simHumid) + "%");
      lv_obj_set_style_text_color(lbl_humid_val, colors.COLOR_TEXT_HUM, LV_PART_MAIN);
      lv_obj_set_style_text_font(lbl_humid_val, press_start_2p_32, LV_PART_MAIN);
      lv_obj_align(lbl_humid_val, LV_ALIGN_TOP_LEFT, 235, 160);

      // --- VIEW 2: RETRO RADIO (Full Screen) ---
      radio_view = lv_obj_create(screen);
      lv_obj_set_pos(radio_view, 0, 0);
      lv_obj_set_size(radio_view, 480, 272);
      lv_obj_set_style_bg_opa(radio_view, LV_OPA_TRANSP, LV_PART_MAIN);
      lv_obj_set_style_border_width(radio_view, 0, LV_PART_MAIN);
      lv_obj_clear_flag(radio_view, LV_OBJ_FLAG_SCROLLABLE);

      // 1. TOP STATUS BAR (Centered over the 380px rectangle border at x=38..418)
      let r_hdr_bar = document.createElement('div');
      r_hdr_bar.style.cssText = "position:absolute; top:32px; left:38px; width:380px; text-align:center; z-index:5;";
      r_hdr_bar.innerHTML = `
        <span style="font-family:'Press Start 2P', monospace; font-size:11px; color:#FF6D00; letter-spacing:0.5px; text-shadow:0 0 6px rgba(255,109,0,0.6);">MALAYSIA FM TUNER</span>
      `;
      radio_view.dom.appendChild(r_hdr_bar);

      // 2. MAIN FREQUENCY & STATION CARD
      let r_card = lv_obj_create(radio_view);
      lv_obj_set_pos(r_card, 38, 56);
      lv_obj_set_size(r_card, 380, 94);
      r_card.dom.className += ' retro-radio-card';
      r_card.dom.style.cssText += "position:absolute; left:38px; top:56px; width:380px; height:94px; padding:6px 14px 6px 10px; border-radius:8px; border:3px solid #5c2b14; outline:1.5px solid #2e1408; background:linear-gradient(180deg, #12100d 0%, #0a0907 100%); box-shadow:inset 0 0 10px rgba(0,0,0,0.9), 0 2px 6px rgba(0,0,0,0.6); display:flex; justify-content:space-between; align-items:center;";

      let left_col = document.createElement('div');
      left_col.style.cssText = "display:flex; flex-direction:column; justify-content:space-between; height:100%; max-width:250px;";
      left_col.innerHTML = `
        <div style="display:flex; align-items:baseline; gap:6px;">
          <span id="r-freq-num" style="font-family:'Press Start 2P', monospace; font-size:30px; color:#228B22; letter-spacing:-1px; text-shadow:0 0 8px rgba(34,139,34,0.4);">89.9</span>
          <span id="r-freq-unit" style="font-family:'Press Start 2P', monospace; font-size:14px; color:#228B22;">MHz</span>
        </div>
        <div id="r-station-name-text" style="font-family:'Press Start 2P', monospace; font-size:13px; color:#FFE082; letter-spacing:0.5px; text-shadow:0 0 5px rgba(255,224,130,0.4);">BFM 89.9</div>
        <div id="r-genre-name-text" style="font-family:'Press Start 2P', monospace; font-size:8px; color:#C7BDB3; line-height:1.3; max-width:245px;">BUSINESS &amp; CURRENT AFFAIRS</div>
      `;
      r_card.dom.appendChild(left_col);

      let right_col = document.createElement('div');
      right_col.style.cssText = "display:flex; flex-direction:column; align-items:center; justify-content:space-between; height:100%; padding:2px 0;";
      right_col.innerHTML = `
        <div id="r-tuned-badge" style="border:2px solid #FF6D00; border-radius:5px; padding:2px 6px; font-family:'Press Start 2P', monospace; font-size:9px; color:#FF6D00; background:rgba(255,109,0,0.1); box-shadow:0 0 8px rgba(255,109,0,0.4); text-transform:uppercase;">[TUNED]</div>
        <div id="r-eq-visualizer" class="eq-visualizer" style="display:flex; gap:3px; align-items:flex-end;">
          <div class="eq-col"><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div></div>
          <div class="eq-col"><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div></div>
          <div class="eq-col"><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div></div>
          <div class="eq-col"><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div></div>
          <div class="eq-col"><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div></div>
          <div class="eq-col"><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div></div>
          <div class="eq-col"><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div></div>
          <div class="eq-col"><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div><div class="eq-seg"></div></div>
        </div>
      `;
      r_card.dom.appendChild(right_col);

      // 3. ANALOG TUNING DIAL CARD (Single Brown Border)
      let dial_box = lv_obj_create(radio_view);
      lv_obj_set_pos(dial_box, 38, 158);
      lv_obj_set_size(dial_box, 380, 72);
      dial_box.dom.className += ' retro-radio-card';
      dial_box.dom.style.cssText += "position:absolute; left:38px; top:158px; width:380px; height:72px; border-radius:8px; border:3px solid #5c2b14; outline:1.5px solid #2e1408; background:#080705; box-shadow:inset 0 0 10px rgba(0,0,0,0.95), 0 2px 6px rgba(0,0,0,0.6); overflow:hidden;";

      // Dial scale markers: 88, 92, 96, 100, 104, 108
      const scale_nums = ["88", "92", "96", "100", "104", "108"];
      const major_x = [32, 94, 156, 218, 280, 342];

      for (let i = 0; i < 6; i++) {
        let x = major_x[i];
        // Major Tick (Taller Orange Marker: 3px wide, 22px high)
        let tick = document.createElement('div');
        tick.style.cssText = `position:absolute; left:${x - 1}px; top:6px; width:3px; height:22px; background:#FF6D00; box-shadow:0 0 3px rgba(255,109,0,0.7);`;
        dial_box.dom.appendChild(tick);

        // Major Label (Pixelated Font: Press Start 2P, 10% smaller: 8px)
        let scale_lbl = document.createElement('div');
        let lbl_w = (i >= 3) ? 42 : 28;
        scale_lbl.style.cssText = `position:absolute; left:${x - (lbl_w / 2)}px; top:38px; width:${lbl_w}px; text-align:center; font-family:'Press Start 2P', monospace; font-size:8px; color:#FFFFFF;`;
        scale_lbl.textContent = scale_nums[i];
        dial_box.dom.appendChild(scale_lbl);

        // Intermediate ticks (Taller White Markers: 2px wide, 12px high)
        if (i < 5) {
          for (let s = 1; s <= 3; s++) {
            let sub_x = Math.round(x + (62 * s) / 4);
            let subtick = document.createElement('div');
            subtick.style.cssText = `position:absolute; left:${sub_x - 1}px; top:9px; width:2px; height:12px; background:#EEEEEE;`;
            dial_box.dom.appendChild(subtick);
          }
        }
      }

      // Pure Vivid Red Tuning Needle (no purple tint, 60px high)
      r_needle = document.createElement('div');
      r_needle.id = "r-tuning-needle";
      r_needle.style.cssText = "position:absolute; top:6px; left:30px; width:4px; height:60px; border-radius:2px; background:#FF0000; box-shadow:0 0 8px rgba(255,0,0,0.9); z-index:10; transition:left 0.05s ease-out;";
      dial_box.dom.appendChild(r_needle);

      // --- VIEW 3: RETRO FLIGHT RADAR (Penang Airport WMKP) ---
      initRadarView();

      // Set initial view visibility
      if (currentViewMode === 'CACTUS') {
        lv_obj_clear_flag(cactus_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(radio_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(radar_view, LV_OBJ_FLAG_HIDDEN);
      } else if (currentViewMode === 'RADIO') {
        lv_obj_add_flag(cactus_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(radio_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(radar_view, LV_OBJ_FLAG_HIDDEN);
      } else {
        lv_obj_add_flag(cactus_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(radio_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(radar_view, LV_OBJ_FLAG_HIDDEN);
        refreshFlightData();
        startRadarLoop();
      }

      updateRadioDisplay();
      updateObjectTree();
    }

    function switchView(mode) {
      currentViewMode = mode;
      if (cactus_view) lv_obj_add_flag(cactus_view, LV_OBJ_FLAG_HIDDEN);
      if (radio_view) lv_obj_add_flag(radio_view, LV_OBJ_FLAG_HIDDEN);
      if (radar_view) lv_obj_add_flag(radar_view, LV_OBJ_FLAG_HIDDEN);

      document.getElementById('vbtn-cactus')?.classList.remove('active');
      document.getElementById('vbtn-radio')?.classList.remove('active');
      document.getElementById('vbtn-radar')?.classList.remove('active');

      if (mode === 'CACTUS') {
        if (cactus_view) lv_obj_clear_flag(cactus_view, LV_OBJ_FLAG_HIDDEN);
        document.getElementById('vbtn-cactus')?.classList.add('active');
      } else if (mode === 'RADIO') {
        if (radio_view) lv_obj_clear_flag(radio_view, LV_OBJ_FLAG_HIDDEN);
        document.getElementById('vbtn-radio')?.classList.add('active');
      } else if (mode === 'RADAR') {
        if (radar_view) lv_obj_clear_flag(radar_view, LV_OBJ_FLAG_HIDDEN);
        document.getElementById('vbtn-radar')?.classList.add('active');
        refreshFlightData();
        startRadarLoop();
      }
      requestAnimationFrame(() => {
        registeredObjects.forEach(obj => {
          if (obj.isLabel && obj.dom && obj.dom.offsetWidth > 0) {
            obj.width = obj.dom.offsetWidth;
            obj.height = obj.dom.offsetHeight;
          }
          if (obj.applyStyle) obj.applyStyle();
        });
      });
    }

    function updateRadioDisplay() {
      simRadioFreq = Math.max(88.0, Math.min(108.0, simRadioFreq));
      let freqStr = simRadioFreq.toFixed(1);

      let freqNumEl = document.getElementById('r-freq-num');
      if (freqNumEl) freqNumEl.textContent = freqStr;

      let radioDisplay = document.getElementById('radio-display');
      if (radioDisplay) radioDisplay.textContent = freqStr + " MHz";

      let slider = document.getElementById('radio-slider');
      if (slider) slider.value = freqStr;

      // Position needle: range 88 - 108 MHz maps to x: 32px to 342px (span: 310px)
      const SCALE_MIN_X = 32;
      const SCALE_MAX_X = 342;
      let pct = (simRadioFreq - 88.0) / (108.0 - 88.0);
      pct = Math.max(0, Math.min(1, pct));
      let needle_x = Math.round(SCALE_MIN_X + pct * (SCALE_MAX_X - SCALE_MIN_X) - 2);
      if (r_needle) {
        r_needle.style.left = needle_x + 'px';
      }

      // Check tuned station (within 0.25 MHz)
      let matched = null;
      for (const st of STATIONS) {
        if (Math.abs(st.freq - simRadioFreq) <= 0.25) {
          matched = st;
          break;
        }
      }

      let badgeEl = document.getElementById('r-tuned-badge');
      let nameEl = document.getElementById('r-station-name-text');
      let genreEl = document.getElementById('r-genre-name-text');

      if (matched) {
        if (badgeEl) {
          badgeEl.textContent = "[TUNED]";
          badgeEl.style.borderColor = "#FF6D00";
          badgeEl.style.color = "#FF6D00";
          badgeEl.style.background = "rgba(255, 109, 0, 0.12)";
          badgeEl.style.boxShadow = "0 0 8px rgba(255, 109, 0, 0.4)";
        }
        if (nameEl) nameEl.textContent = matched.name;
        if (genreEl) genreEl.textContent = matched.genre;
      } else {
        if (badgeEl) {
          badgeEl.textContent = "[STATIC]";
          badgeEl.style.borderColor = "#607D8B";
          badgeEl.style.color = "#90A4AE";
          badgeEl.style.background = "rgba(96, 125, 139, 0.1)";
          badgeEl.style.boxShadow = "none";
        }
        if (nameEl) nameEl.textContent = "NO SIGNAL";
        if (genreEl) genreEl.textContent = "FM STATIC NOISE • ROTATE KNOB";
      }

      // Update preset buttons active state
      document.querySelectorAll('.station-btn').forEach(btn => {
        let f = parseFloat(btn.dataset.freq);
        if (Math.abs(f - simRadioFreq) <= 0.1) {
          btn.classList.add('active');
        } else {
          btn.classList.remove('active');
        }
      });
    }

    // ==========================================
    // VIEW 3: RETRO FLIGHT RADAR IMPLEMENTATION
    // ==========================================
    function initRadarView() {
      let screen = lv_scr_act();
      radar_view = lv_obj_create(screen);
      lv_obj_set_pos(radar_view, 0, 0);
      lv_obj_set_size(radar_view, 480, 272);
      lv_obj_set_style_bg_opa(radar_view, LV_OPA_TRANSP, LV_PART_MAIN);
      lv_obj_set_style_border_width(radar_view, 0, LV_PART_MAIN);
      lv_obj_clear_flag(radar_view, LV_OBJ_FLAG_SCROLLABLE);

      // --- 1. LEFT PPI RADAR SCOPE CONTAINER ---
      let box_radar = document.createElement('div');
      box_radar.className = 'radar-panel';
      box_radar.style.cssText = "position:absolute; left:24px; top:28px; width:206px; height:218px; border:1.5px solid #00D26A; border-radius:6px; box-sizing:border-box; background:#020804; box-shadow:0 0 8px rgba(0,210,106,0.25), inset 0 0 6px rgba(0,210,106,0.08); overflow:hidden; cursor:crosshair;";
      
      let canvas = document.createElement('canvas');
      canvas.id = 'radar-ppi-canvas';
      canvas.width = 206;
      canvas.height = 218;
      canvas.style.cssText = "position:absolute; left:0; top:0; width:100%; height:100%;";
      box_radar.appendChild(canvas);

      // Canvas click to select flight
      canvas.addEventListener('click', (e) => {
        let rect = canvas.getBoundingClientRect();
        let scaleX = canvas.width / rect.width;
        let scaleY = canvas.height / rect.height;
        let clickX = (e.clientX - rect.left) * scaleX;
        let clickY = (e.clientY - rect.top) * scaleY;
        
        let found = null;
        for (let flight of currentFlights) {
          if (flight._px !== undefined) {
            let dx = clickX - flight._px;
            let dy = clickY - flight._py;
            if (Math.hypot(dx, dy) < 18) {
              found = flight;
              break;
            }
          }
        }
        if (found) {
          selectedFlightCallsign = found.flight;
          renderFlightTable();
          drawRadarPPI();
        }
      });

      radar_view.dom.appendChild(box_radar);

      // --- 2. RIGHT ARRIVAL BOARD TABLE CONTAINER ---
      let box_table = document.createElement('div');
      box_table.className = 'radar-panel';
      box_table.style.cssText = "position:absolute; left:238px; top:28px; width:218px; height:218px; border:1.5px solid #00D26A; border-radius:6px; box-sizing:border-box; background:#020804; box-shadow:0 0 8px rgba(0,210,106,0.25), inset 0 0 6px rgba(0,210,106,0.08); display:flex; flex-direction:column; overflow:hidden;";
      box_table.innerHTML = `
        <div style="display:flex; align-items:center; height:20px; padding:0 4px; background:rgba(0,50,25,0.4); border-bottom:1px solid #005528; font-size:6px; color:#00FF66; letter-spacing:0.5px; flex-shrink:0;">
          <span style="width:64px;">FLIGHT</span>
          <span style="width:36px;">FROM</span>
          <span style="width:44px;">DIST</span>
          <span style="width:70px;">STATUS</span>
        </div>
        <div id="radar-flight-list" style="flex:1; overflow-y:auto; padding:2px 0;"></div>
      `;
      radar_view.dom.appendChild(box_table);
    }

    function drawRadarPPI() {
      let canvas = document.getElementById('radar-ppi-canvas');
      if (!canvas) return;
      let ctx = canvas.getContext('2d');
      let w = canvas.width;
      let h = canvas.height;

      ctx.clearRect(0, 0, w, h);

      // Center WMKP
      let cx = 103;
      let cy = 118;
      let scale = 75.0 / radarRadius; // 15 NM ring at 75px (scale = 5.0)

      // Header & Count Badge
      ctx.font = "6px 'Press Start 2P', monospace";
      ctx.fillStyle = '#00FF66';
      ctx.fillText(`RADAR (${radarRadius}NM)`, 6, 12);

      let countStr = `[${String(currentFlights.length).padStart(2, '0')}]`;
      ctx.fillStyle = '#FFA500';
      ctx.fillText(countStr, w - 38, 12);

      ctx.beginPath();
      ctx.moveTo(6, 17);
      ctx.lineTo(w - 6, 17);
      ctx.strokeStyle = '#004D25';
      ctx.lineWidth = 1;
      ctx.stroke(); // 15 NM ring at ~42px, 25 NM at 70px

      // 1. Concentric Range Rings
      const ringDistances = (radarRadius <= 25) ? [5, 10, 15] : [10, 25, 50];
      for (let dist of ringDistances) {
        let r = dist * scale;
        if (r < w * 1.2) {
          ctx.beginPath();
          ctx.arc(cx, cy, r, 0, 2 * Math.PI);
          ctx.strokeStyle = '#004422';
          ctx.lineWidth = 1;
          ctx.setLineDash([3, 4]);
          ctx.stroke();

          // Label
          ctx.font = "5.5px 'Press Start 2P', monospace";
          ctx.fillStyle = '#008844';
          ctx.fillText(`${dist} NM`, cx + r * 0.707 + 3, cy - r * 0.707);
        }
      }
      ctx.setLineDash([]);

      // 2. Crosshairs
      ctx.beginPath();
      ctx.moveTo(cx - 6, cy); ctx.lineTo(cx + 6, cy);
      ctx.moveTo(cx, cy - 6); ctx.lineTo(cx, cy + 6);
      ctx.strokeStyle = '#005528';
      ctx.lineWidth = 1;
      ctx.stroke();

      // 3. Penang Island Landmass Vector
      const islandPtsNM = [
        [-6.1, 10.1], [-4.0, 9.8], [-1.6, 10.7], [0.8, 10.1],
        [4.1, 7.4],   [2.3, 4.4],  [2.0, 3.5],   [1.4, 0.8],
        [0.8, -1.0],  [0.2, -2.2], [-5.2, -1.3], [-5.8, 0.8],
        [-5.2, 3.2],  [-6.1, 7.5]
      ];

      ctx.beginPath();
      for (let i = 0; i < islandPtsNM.length; i++) {
        let px = cx + islandPtsNM[i][0] * scale;
        let py = cy - islandPtsNM[i][1] * scale;
        if (i === 0) ctx.moveTo(px, py);
        else ctx.lineTo(px, py);
      }
      ctx.closePath();
      ctx.fillStyle = 'rgba(0, 42, 18, 0.65)';
      ctx.fill();
      ctx.strokeStyle = '#00883a';
      ctx.lineWidth = 1;
      ctx.stroke();

      // Mainland Coastline (Seberang Perai)
      ctx.beginPath();
      let mainlandX = cx + 8.5 * scale;
      ctx.moveTo(mainlandX + 8, 0);
      ctx.lineTo(mainlandX + 4, cy - 6 * scale);
      ctx.lineTo(mainlandX + 2, cy - 1 * scale);
      ctx.lineTo(mainlandX + 6, cy + 5 * scale);
      ctx.lineTo(mainlandX + 12, h);
      ctx.lineTo(w, h);
      ctx.lineTo(w, 0);
      ctx.closePath();
      ctx.fillStyle = 'rgba(0, 32, 14, 0.45)';
      ctx.fill();
      ctx.strokeStyle = '#00662c';
      ctx.lineWidth = 1;
      ctx.stroke();

      // Island Name
      ctx.font = "6px 'Press Start 2P', monospace";
      ctx.fillStyle = '#00B359';
      ctx.fillText("PENANG", cx - 4.5 * scale, cy - 5.5 * scale);
      ctx.fillText("ISLAND", cx - 4.5 * scale, cy - 4.0 * scale);

      // 4. Runway 04/22 Strip at WMKP
      ctx.save();
      ctx.translate(cx, cy);
      ctx.rotate((44 - 90) * Math.PI / 180); // Runway heading 044°
      ctx.fillStyle = '#C8E6C9';
      ctx.fillRect(-2, -8, 4, 16);
      ctx.strokeStyle = '#00FF66';
      ctx.lineWidth = 0.8;
      ctx.strokeRect(-2, -8, 4, 16);
      ctx.restore();

      ctx.font = "5px 'Press Start 2P', monospace";
      ctx.fillStyle = '#00FF66';
      ctx.fillText("PEN (WMKP)", cx - 18, cy + 14);

      // 5. Extended Approach Centerline for Runway 04 (from Southwest 224°)
      let rwyAngle = (224 - 90) * Math.PI / 180;
      let appLen = 16 * scale;
      let appEndX = cx + appLen * Math.cos(rwyAngle);
      let appEndY = cy + appLen * Math.sin(rwyAngle);

      ctx.beginPath();
      ctx.setLineDash([4, 4]);
      ctx.moveTo(cx, cy);
      ctx.lineTo(appEndX, appEndY);
      ctx.strokeStyle = '#009944';
      ctx.lineWidth = 1;
      ctx.stroke();
      ctx.setLineDash([]);

      // Approach tick marks
      for (let t = 4; t <= 14; t += 4) {
        let tx = cx + (t * scale) * Math.cos(rwyAngle);
        let ty = cy + (t * scale) * Math.sin(rwyAngle);
        let perpAngle = rwyAngle + Math.PI / 2;
        ctx.beginPath();
        ctx.moveTo(tx - 3 * Math.cos(perpAngle), ty - 3 * Math.sin(perpAngle));
        ctx.lineTo(tx + 3 * Math.cos(perpAngle), ty + 3 * Math.sin(perpAngle));
        ctx.strokeStyle = '#00883a';
        ctx.stroke();
      }

      // 6. Rotating Sweep Needle & Phosphor Trail
      let sweepRad = radarSweepAngle * Math.PI / 180;
      let sweepLen = Math.max(w, h) * 1.2;
      let sweepX = cx + sweepLen * Math.cos(sweepRad);
      let sweepY = cy + sweepLen * Math.sin(sweepRad);

      // Trailing fade wedge
      let trailGrad = ctx.createRadialGradient(cx, cy, 2, cx, cy, sweepLen);
      trailGrad.addColorStop(0, 'rgba(0, 255, 102, 0.25)');
      trailGrad.addColorStop(1, 'rgba(0, 255, 102, 0.0)');

      ctx.save();
      ctx.beginPath();
      ctx.moveTo(cx, cy);
      ctx.arc(cx, cy, sweepLen, sweepRad - 0.45, sweepRad, false);
      ctx.closePath();
      ctx.fillStyle = trailGrad;
      ctx.fill();

      // Sharp sweep line
      ctx.beginPath();
      ctx.moveTo(cx, cy);
      ctx.lineTo(sweepX, sweepY);
      ctx.strokeStyle = 'rgba(0, 255, 102, 0.85)';
      ctx.lineWidth = 1.5;
      ctx.shadowColor = '#00FF66';
      ctx.shadowBlur = 6;
      ctx.stroke();
      ctx.restore();

      // 7. Scale & North Arrow
      ctx.font = "6px 'Press Start 2P', monospace";
      ctx.fillStyle = '#00FF66';
      ctx.fillText("▲", w - 24, 14);
      ctx.fillText("N", w - 16, 14);

      // Scale bar (bottom right)
      let scaleBarNM = (radarRadius <= 25) ? 10 : 25;
      let scaleBarPx = scaleBarNM * scale;
      let barX = w - scaleBarPx - 16;
      let barY = h - 10;
      ctx.beginPath();
      ctx.moveTo(barX, barY);
      ctx.lineTo(barX + scaleBarPx, barY);
      ctx.moveTo(barX, barY - 3); ctx.lineTo(barX, barY + 3);
      ctx.moveTo(barX + scaleBarPx / 2, barY - 2); ctx.lineTo(barX + scaleBarPx / 2, barY + 2);
      ctx.moveTo(barX + scaleBarPx, barY - 3); ctx.lineTo(barX + scaleBarPx, barY + 3);
      ctx.strokeStyle = '#00CC66';
      ctx.lineWidth = 1;
      ctx.stroke();
      ctx.font = "5px 'Press Start 2P', monospace";
      ctx.fillStyle = '#00AA44';
      ctx.fillText(`0`, barX - 4, barY - 5);
      ctx.fillText(`${scaleBarNM} NM`, barX + scaleBarPx - 14, barY - 5);

      // 8. Aircraft Blips & Callout Badges
      currentFlights.forEach((ac, idx) => {
        let dist = ac.dst || 0;
        let bearing = ac.dir || 0;
        let rad = (bearing - 90) * Math.PI / 180;
        let px = cx + (dist * scale) * Math.cos(rad);
        let py = cy + (dist * scale) * Math.sin(rad);

        ac._px = px;
        ac._py = py;

        let isSelected = (ac.flight === selectedFlightCallsign);
        let color = isSelected ? '#FFA500' : '#00FF66';

        // Draw Airplane Icon
        ctx.save();
        ctx.translate(px, py);
        ctx.rotate((ac.track || 0) * Math.PI / 180);
        
        ctx.font = "12px sans-serif";
        ctx.textAlign = "center";
        ctx.textBaseline = "middle";
        ctx.fillStyle = color;
        if (isSelected) {
          ctx.shadowColor = '#FFA500';
          ctx.shadowBlur = 8;
        }
        ctx.fillText("✈", 0, 0);
        ctx.restore();

        // Draw Callout Box for selected or top flights within canvas bounds
        if ((isSelected || idx < 4) && px > 10 && px < w - 40 && py > 15 && py < h - 15) {
          let tagX = px + (px < cx ? -48 : 12);
          let tagY = py + (py < cy ? -24 : 10);
          
          tagX = Math.max(4, Math.min(w - 62, tagX));
          tagY = Math.max(4, Math.min(h - 22, tagY));

          // Leader line
          ctx.beginPath();
          ctx.moveTo(px, py);
          ctx.lineTo(tagX + (px < tagX ? 0 : 54), tagY + 9);
          ctx.strokeStyle = isSelected ? '#FFA500' : '#00883a';
          ctx.lineWidth = 0.8;
          ctx.stroke();

          // Box
          ctx.fillStyle = 'rgba(2, 10, 4, 0.9)';
          ctx.fillRect(tagX, tagY, 56, 18);
          ctx.strokeStyle = color;
          ctx.lineWidth = 1;
          ctx.strokeRect(tagX, tagY, 56, 18);

          let arrow = (ac.baro_rate < -150) ? '↓' : ((ac.baro_rate > 150) ? '↑' : '─');
          ctx.font = "5px 'Press Start 2P', monospace";
          ctx.fillStyle = color;
          ctx.fillText(ac.flight, tagX + 4, tagY + 7);
          ctx.fillText(`${arrow} ${(ac.alt_baro || 0).toLocaleString()}ft`, tagX + 4, tagY + 15);
        }
      });
    }

    function renderFlightTable() {
      let list = document.getElementById('radar-flight-list');
      if (!list) return;

      list.innerHTML = '';
      currentFlights.forEach((ac, idx) => {
        let isSelected = (ac.flight === selectedFlightCallsign);
        let arrow = (ac.baro_rate < -150) ? '↓' : ((ac.baro_rate > 150) ? '↑' : '─');
        let cursor = isSelected ? '▶' : '&nbsp;';

        let row = document.createElement('div');
        row.className = `radar-table-row ${isSelected ? 'selected' : ''}`;
        row.innerHTML = `
          <span style="width:64px; overflow:hidden; text-overflow:ellipsis;">${cursor}${ac.flight}</span>
          <span style="width:36px; color:#00CC66;">${ac.origin || '---'}</span>
          <span style="width:44px;">${(ac.dst || 0).toFixed(1)}</span>
          <span style="width:70px; color:${isSelected ? '#FFA500' : '#00FF66'}; overflow:hidden; text-overflow:ellipsis;">${ac.status || 'ENROUTE'}</span>
        `;
        row.addEventListener('click', () => {
          selectedFlightCallsign = ac.flight;
          renderFlightTable();
          drawRadarPPI();
        });
        list.appendChild(row);
      });

      // Update Top Status Card
      let acCountEl = document.getElementById('radar-ac-count');
      if (acCountEl) acCountEl.textContent = String(currentFlights.length).padStart(2, '0');

      let rwyValEl = document.getElementById('radar-rwy-val');
      if (rwyValEl) rwyValEl.textContent = metarInfo.runway;

      let windValEl = document.getElementById('radar-wind-val');
      if (windValEl) windValEl.textContent = `${String(metarInfo.windDir).padStart(3, '0')}° ${metarInfo.windSpeed} kt`;

      let wxValEl = document.getElementById('radar-wx-val');
      if (wxValEl) wxValEl.textContent = `${metarInfo.temp}°C ${metarInfo.clouds}`;
    }

    async function fetchFlightData() {
      let statusBadge = document.getElementById('radar-status-badge');
      if (radarDataSource === 'demo') {
        currentFlights = [...DEMO_FLIGHTS];
        if (statusBadge) statusBadge.innerHTML = 'Status: <b style="color:#FFA500;">Demo Snapshot</b>';
        renderFlightTable();
        drawRadarPPI();
        return;
      }

      try {
        if (statusBadge) statusBadge.innerHTML = 'Status: <b style="color:#00FF66;">Fetching...</b>';
        let apiUrl = (window.location.protocol === 'file:')
          ? `https://api.adsb.lol/v2/point/5.297/100.277/${radarRadius}`
          : `/api/flights?radius=${radarRadius}`;
        
        let resp = await fetch(apiUrl, { signal: AbortSignal.timeout(6000) });

        if (resp && resp.ok) {
          let data = await resp.json();
          let rawList = data.ac || [];
          
          if (rawList.length === 0) {
            if (statusBadge) statusBadge.innerHTML = `Status: <span style="color:#FFB300;">Quiet (${radarRadius}NM: 0 planes)</span>`;
            if (currentFlights.length === 0) {
              currentFlights = [...DEMO_FLIGHTS];
            }
          } else {
            if (statusBadge) statusBadge.innerHTML = `Status: <b style="color:#00FF66;">Live ADS-B (${rawList.length})</b>`;
            
            let parsed = rawList.map(ac => {
              let flight = (ac.flight || ac.r || ac.hex || "N/A").trim();
              let alt = ac.alt_baro === "ground" ? 0 : (ac.alt_baro || ac.alt_geom || 0);
              let gs = Math.round(ac.gs || ac.tas || 0);
              let track = Math.round(ac.track || 0);
              let rate = ac.baro_rate || 0;
              
              let dst = ac.dst;
              let dir = ac.dir;
              if (dst === undefined && ac.lat && ac.lon) {
                let dLat = (ac.lat - 5.297) * 60;
                let dLon = (ac.lon - 100.277) * 60 * Math.cos(5.297 * Math.PI / 180);
                dst = Math.hypot(dLat, dLon);
                dir = (Math.atan2(dLon, dLat) * 180 / Math.PI + 360) % 360;
              }
              dst = dst || 0;
              dir = dir || 0;

              let prefix = flight.substring(0, 3).toUpperCase();
              let origin = "KUL";
              if (prefix === "SIA" || prefix === "TRZ") origin = "SIN";
              else if (prefix === "FDV" || prefix === "AIQ") origin = "DMK";
              else if (prefix === "MAS" && dst > 40) origin = "KCH";
              else if (prefix === "BAO") origin = "BKK";
              else if (prefix === "CLX") origin = "LUX";
              else if (prefix === "MXD") origin = "SZB";

              let dest = "PEN";
              let status = "ENROUTE";
              if (rate < -150 && dst < 25) {
                if (track >= 20 && track <= 70) status = "ARRIVING RWY 04";
                else if (track >= 200 && track <= 250) status = "ARRIVING RWY 22";
                else status = "ARRIVING";
              } else if (rate > 200 && dst < 20) {
                status = "DEPARTING";
                origin = "PEN";
                dest = "---";
              }

              return {
                flight,
                origin,
                dest,
                alt_baro: alt,
                gs,
                track,
                dst,
                dir,
                baro_rate: rate,
                status
              };
            });

            parsed.sort((a, b) => a.dst - b.dst);
            currentFlights = parsed;
            if (!currentFlights.some(f => f.flight === selectedFlightCallsign) && currentFlights.length > 0) {
              selectedFlightCallsign = currentFlights[0].flight;
            }
          }
          renderFlightTable();
          drawRadarPPI();
        }
      } catch (err) {
        console.warn("ADS-B fetch warning:", err);
        if (statusBadge) statusBadge.innerHTML = 'Status: <b style="color:#FFA500;">ADS-B Offline (Showing Demo)</b>';
        if (currentFlights.length === 0) {
          currentFlights = [...DEMO_FLIGHTS];
          renderFlightTable();
          drawRadarPPI();
        }
      }
    }

    async function fetchMetarData() {
      try {
        let metarUrl = (window.location.protocol === 'file:')
          ? 'https://aviationweather.gov/api/data/metar?ids=WMKP&format=json'
          : '/api/metar';
        
        let resp = await fetch(metarUrl, { signal: AbortSignal.timeout(6000) });
        if (resp && resp.ok) {
          let data = await resp.json();
          if (Array.isArray(data) && data.length > 0) {
            let ob = data[0];
            metarInfo.windDir = ob.wdir !== undefined ? ob.wdir : 40;
            metarInfo.windSpeed = ob.wspd !== undefined ? ob.wspd : 10;
            metarInfo.temp = ob.temp !== undefined ? ob.temp : 31;
            metarInfo.clouds = (ob.clouds && ob.clouds[0]) ? `${ob.clouds[0].cover}${String(ob.clouds[0].base).padStart(3, '0')}` : (ob.cover || 'FEW020');
            metarInfo.raw = ob.rawOb || '';

            metarInfo.runway = (metarInfo.windDir >= 130 && metarInfo.windDir <= 310) ? '22' : '04';
            renderFlightTable();
          }
        }
      } catch(e) {
        console.warn("METAR fetch warning:", e);
      }
    }

    function setRadarDataSource(src) {
      radarDataSource = src;
      document.getElementById('btn-mode-live')?.classList.toggle('btn-primary', src === 'live');
      document.getElementById('btn-mode-demo')?.classList.toggle('btn-primary', src === 'demo');
      refreshFlightData(true);
    }

    function onRadarRangeChange(val) {
      radarRadius = parseInt(val, 10);
      let rangeDisp = document.getElementById('radar-range-display');
      if (rangeDisp) rangeDisp.textContent = `${radarRadius} NM`;
      refreshFlightData(true);
    }

    function refreshFlightData(force) {
      fetchFlightData();
      fetchMetarData();
    }

    function startRadarLoop() {
      if (!radarAnimationId) {
        function loop() {
          if (currentViewMode === 'RADAR') {
            radarSweepAngle = (radarSweepAngle + 1.2) % 360;
            drawRadarPPI();
            radarAnimationId = requestAnimationFrame(loop);
          } else {
            radarAnimationId = null;
          }
        }
        radarAnimationId = requestAnimationFrame(loop);
      }
      if (!radarFetchTimer) {
        radarFetchTimer = setInterval(() => {
          if (currentViewMode === 'RADAR' && radarDataSource === 'live') {
            fetchFlightData();
          }
        }, 6000);
      }
    }
    function animateRetroRadio() {
      // Clock
      let now = new Date();
      let h = String(now.getHours()).padStart(2, '0');
      let m = String(now.getMinutes()).padStart(2, '0');
      let clockEl = document.getElementById('r-clock');
      if (clockEl) clockEl.textContent = `${h}:${m}`;

      // Equalizer bars
      if (currentViewMode !== 'RADIO') return;
      let cols = document.querySelectorAll('#r-eq-visualizer .eq-col');
      if (!cols.length) return;

      let isTuned = false;
      for (const st of STATIONS) {
        if (Math.abs(st.freq - simRadioFreq) <= 0.25) {
          isTuned = true;
          break;
        }
      }

      cols.forEach((col, idx) => {
        let segs = col.querySelectorAll('.eq-seg');
        let max = 0;
        if (isTuned) {
          let base = (idx === 4 || idx === 3 || idx === 5) ? 4 : 2;
          max = Math.floor(Math.random() * 3) + base;
          max = Math.min(6, Math.max(1, max));
        } else {
          max = Math.random() > 0.6 ? 1 : 0;
        }
        segs.forEach((seg, sIdx) => {
          if (sIdx < max) seg.classList.add('active');
          else seg.classList.remove('active');
        });
      });
    }
    setInterval(animateRetroRadio, 100);

    function onRadioSliderChange(val) {
      simRadioFreq = parseFloat(val);
      updateRadioDisplay();
    }

    function tuneStep(delta) {
      simRadioFreq = Math.round((simRadioFreq + delta) * 10) / 10;
      updateRadioDisplay();
    }

    function tuneToStation(freq) {
      simRadioFreq = freq;
      updateRadioDisplay();
      if (currentViewMode !== 'RADIO') switchView('RADIO');
    }

    function updateDisplayValues() {
      if (lbl_temp_val) {
        lv_label_set_text(lbl_temp_val, Math.round(simTemp) + '°C');
      }
      if (lbl_humid_val) {
        lv_label_set_text(lbl_humid_val, Math.round(simHumid) + '%');
      }
      document.getElementById('temp-display').textContent = Math.round(simTemp) + '°C';
      document.getElementById('humid-display').textContent = Math.round(simHumid) + '%';
      document.getElementById('temp-slider').value = Math.round(simTemp);
      document.getElementById('humid-slider').value = Math.round(simHumid);
    }

    function onTempSliderChange(val) {
      simTemp = parseFloat(val);
      updateDisplayValues();
    }

    function onHumidSliderChange(val) {
      simHumid = parseFloat(val);
      updateDisplayValues();
    }

    function toggleSimLoop(enabled) {
      simLoopActive = enabled;
      document.getElementById('fps-counter').textContent = enabled ? 'Loop: Active (2s)' : 'Loop: Paused';
    }

    setInterval(() => {
      if (!simLoopActive) return;
      simTemp += (Math.floor(Math.random() * 11) - 5) * 0.1;
      if (simTemp < 15.0) simTemp = 15.0;
      if (simTemp > 40.0) simTemp = 40.0;

      simHumid += (Math.floor(Math.random() * 11) - 5) * 0.5;
      if (simHumid < 20.0) simHumid = 20.0;
      if (simHumid > 95.0) simHumid = 95.0;

      updateDisplayValues();
    }, 2000);

    function setZoom(scale) {
      let wrapper = document.getElementById('screen-wrapper');
      wrapper.style.transform = `scale(${scale})`;
      wrapper.style.marginBottom = scale === 2 ? '140px' : (scale === 1.5 ? '70px' : '0px');
      
      document.querySelectorAll('.viewport-toolbar .btn').forEach(b => {
        if (b.textContent.includes(scale + 'x')) b.classList.add('active');
        else if (b.textContent.includes('x')) b.classList.remove('active');
      });
    }

    function toggleGrid() {
      let grid = document.getElementById('grid-overlay');
      grid.classList.toggle('show');
      document.getElementById('btn-grid').classList.toggle('active');
    }

    function toggleScanlines() {
      let scan = document.getElementById('scanlines');
      scan.classList.toggle('show');
      document.getElementById('btn-scanlines').classList.toggle('active');
    }

    function toggleInspect() {
      inspectMode = !inspectMode;
      document.getElementById('btn-inspect').classList.toggle('active', inspectMode);
      if (!inspectMode) {
        document.querySelectorAll('.lv-obj').forEach(el => el.classList.remove('inspect-hover'));
        document.getElementById('mouse-coords').textContent = 'X: 0, Y: 0';
      }
    }

    document.getElementById('lvgl-screen').addEventListener('mousemove', (e) => {
      let rect = e.currentTarget.getBoundingClientRect();
      let scaleMatch = document.getElementById('screen-wrapper').style.transform.match(/scale\(([^)]+)\)/);
      let scale = scaleMatch ? parseFloat(scaleMatch[1]) : 1.5;
      let x = Math.round((e.clientX - rect.left) / scale);
      let y = Math.round((e.clientY - rect.top) / scale);
      if (x >= 0 && x <= 480 && y >= 0 && y <= 272) {
        document.getElementById('mouse-coords').textContent = `X: ${x}, Y: ${y}`;
      }
    });

    function showCoords(obj) {
      document.getElementById('mouse-coords').textContent = `Inspecting: ${obj.name} [X: ${obj.x}, Y: ${obj.y}, W: ${obj.width}, H: ${obj.height}]`;
    }

    function openTab(tabId) {
      document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
      document.querySelectorAll('.tab-pane').forEach(p => p.classList.remove('active'));
      
      event.currentTarget.classList.add('active');
      document.getElementById('tab-' + tabId).classList.add('active');
    }

    function updatePaletteColor(defName, newHex) {
      let colors = extractColors(document.getElementById('code-editor').value);
      colors[defName] = newHex;

      let regex = new RegExp(`(#define\\\\s+${defName}\\\\s+lv_color_hex\\\\()(0x[0-9a-fA-F]+)(\\\\))`);
      let cleanHex = '0x' + newHex.replace('#', '').toUpperCase();
      let editor = document.getElementById('code-editor');
      if (regex.test(editor.value)) {
        editor.value = editor.value.replace(regex, `$1${cleanHex}$3`);
      }
      renderUI(colors);
      showToast(`Updated ${defName} to ${newHex}`);
    }

    function updateObjectTree() {
      let tree = document.getElementById('inspector-tree-list');
      tree.innerHTML = '';
      registeredObjects.forEach((obj, idx) => {
        let node = document.createElement('div');
        node.className = 'tree-node';
        let label = obj.isLabel ? `Label: "${obj.text}"` : (obj.isButton ? `Button` : `Container #${idx+1}`);
        node.innerHTML = `
          <span class="tree-node-title">${label}</span>
          <span class="tree-node-meta">x:${obj.x} y:${obj.y} (${obj.width}&times;${obj.height})</span>
        `;
        node.addEventListener('mouseenter', () => {
          obj.dom.classList.add('inspect-hover');
        });
        node.addEventListener('mouseleave', () => {
          obj.dom.classList.remove('inspect-hover');
        });
        tree.appendChild(node);
      });
    }

    function applyEditorCode() {
      let code = document.getElementById('code-editor').value;
      let colors = extractColors(code);
      renderUI(colors);
      showToast('Emulator UI re-rendered!');
    }

    function showToast(msg) {
      let toast = document.getElementById('toast');
      document.getElementById('toast-msg').textContent = msg;
      toast.classList.add('show');
      setTimeout(() => {
        toast.classList.remove('show');
      }, 2600);
    }

    // Populate station preset buttons
    function initStationPresets() {
      let cont = document.getElementById('station-presets');
      cont.innerHTML = '';
      STATIONS.forEach(st => {
        let btn = document.createElement('button');
        btn.className = 'station-btn';
        btn.dataset.freq = st.freq;
        btn.innerHTML = `<strong>${st.freq}</strong> &bull; ${st.name}`;
        btn.onclick = () => tuneToStation(st.freq);
        cont.appendChild(btn);
      });
    }

    async function checkServer() {
      try {
        let res = await fetch('/api/source', { cache: 'no-store' });
        if (res.ok) {
          let data = await res.json();
          serverAvailable = true;
          document.getElementById('sync-status').style.background = 'rgba(16, 185, 129, 0.15)';
          document.getElementById('sync-status').style.borderColor = 'rgba(16, 185, 129, 0.3)';
          document.getElementById('sync-status').style.color = '#34d399';
          document.getElementById('sync-text').textContent = 'Server Live: Watching src/main.ino';
          document.getElementById('btn-save-disk').style.display = 'inline-flex';

          if (data.mtime && data.mtime !== lastMtime) {
            lastMtime = data.mtime;
            currentSourceCode = data.code;
            document.getElementById('code-editor').value = currentSourceCode;
            let colors = extractColors(currentSourceCode);
            renderUI(colors);
            showToast('Auto-synced with src/main.ino');
          }
        } else {
          setStandaloneStatus();
        }
      } catch (e) {
        setStandaloneStatus();
      }
    }

    function setStandaloneStatus() {
      serverAvailable = false;
      document.getElementById('sync-status').style.background = 'rgba(245, 158, 11, 0.15)';
      document.getElementById('sync-status').style.borderColor = 'rgba(245, 158, 11, 0.3)';
      document.getElementById('sync-status').style.color = '#fbbf24';
      document.getElementById('sync-text').textContent = 'Standalone Mode (Direct Preview)';
      document.getElementById('btn-save-disk').style.display = 'none';
    }

    async function saveCodeToDisk() {
      let code = document.getElementById('code-editor').value;
      try {
        let res = await fetch('/api/save', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ code: code })
        });
        if (res.ok) {
          showToast('Successfully saved to src/main.ino!');
          checkServer();
        } else {
          showToast('Error saving to disk');
        }
      } catch (e) {
        showToast('Error connecting to server.py');
      }
    }

    initStationPresets();
    document.getElementById('code-editor').value = currentSourceCode;
    let initialColors = extractColors(currentSourceCode);
    renderUI(initialColors);
    switchView(currentViewMode);
    checkServer();
    setInterval(checkServer, 1000);
  </script>
</body>
</html>
"""

html = template.replace("FONT_B64_PLACEHOLDER", font_b64)
html = html.replace("INITIAL_CODE_PLACEHOLDER", json.dumps(initial_code))

output_path = BASE_DIR / "index.html"
with open(output_path, "w", encoding="utf-8") as f:
    f.write(html)

print(f"Successfully generated {output_path} ({len(html)} bytes)")
