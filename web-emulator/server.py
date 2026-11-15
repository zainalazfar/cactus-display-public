#!/usr/bin/env python3
import http.server
import socketserver
import os
import json
import mimetypes
import sys
import time
import urllib.request
import urllib.parse
from pathlib import Path

PORT = 8080
BASE_DIR = Path(__file__).resolve().parent
PROJECT_DIR = BASE_DIR.parent
SOURCE_FILE = PROJECT_DIR / "src" / "main.ino"

# In-memory caches to avoid rate limits
flight_cache = {"timestamp": 0, "radius": 0, "data": None}
metar_cache = {"timestamp": 0, "data": None}

class EmulatorHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(BASE_DIR), **kwargs)

    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.send_header('Cache-Control', 'no-cache, no-store, must-revalidate')
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200)
        self.end_headers()

    def do_GET(self):
        parsed_path = urllib.parse.urlparse(self.path)

        if parsed_path.path == "/api/flights":
            query_params = urllib.parse.parse_qs(parsed_path.query)
            radius = int(query_params.get("radius", ["50"])[0])
            now = time.time()

            global flight_cache
            if flight_cache["data"] and (now - flight_cache["timestamp"] < 5) and flight_cache["radius"] == radius:
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(flight_cache["data"])
                return

            try:
                url = f"https://api.adsb.lol/v2/point/5.297/100.277/{radius}"
                req = urllib.request.Request(url, headers={"User-Agent": "CactusFlightRadar/1.0"})
                with urllib.request.urlopen(req, timeout=6) as response:
                    data = response.read()
                    flight_cache["timestamp"] = now
                    flight_cache["radius"] = radius
                    flight_cache["data"] = data
                    self.send_response(200)
                    self.send_header('Content-Type', 'application/json')
                    self.end_headers()
                    self.wfile.write(data)
                    return
            except Exception as e:
                self.send_response(500)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"error": str(e), "ac": []}).encode('utf-8'))
                return

        if parsed_path.path == "/api/metar":
            now = time.time()
            global metar_cache
            if metar_cache["data"] and (now - metar_cache["timestamp"] < 60):
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(metar_cache["data"])
                return

            try:
                url = "https://aviationweather.gov/api/data/metar?ids=WMKP&format=json"
                req = urllib.request.Request(url, headers={"User-Agent": "CactusFlightRadar/1.0"})
                with urllib.request.urlopen(req, timeout=6) as response:
                    data = response.read()
                    metar_cache["timestamp"] = now
                    metar_cache["data"] = data
                    self.send_response(200)
                    self.send_header('Content-Type', 'application/json')
                    self.end_headers()
                    self.wfile.write(data)
                    return
            except Exception as e:
                self.send_response(500)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"error": str(e)}).encode('utf-8'))
                return

        if self.path == "/api/source":
            if not SOURCE_FILE.exists():
                self.send_response(404)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"error": "src/main.ino not found"}).encode('utf-8'))
                return

            mtime = os.path.getmtime(SOURCE_FILE)
            with open(SOURCE_FILE, "r", encoding="utf-8") as f:
                code = f.read()

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({
                "mtime": mtime,
                "code": code,
                "path": str(SOURCE_FILE)
            }).encode('utf-8'))
            return

        if self.path == "/" or self.path == "":
            self.path = "/index.html"

        return super().do_GET()

    def do_POST(self):
        if self.path == "/api/save":
            content_length = int(self.headers.get('Content-Length', 0))
            body = self.rfile.read(content_length).decode('utf-8')
            try:
                data = json.loads(body)
                code = data.get("code", "")
                with open(SOURCE_FILE, "w", encoding="utf-8") as f:
                    f.write(code)
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"status": "saved", "mtime": os.path.getmtime(SOURCE_FILE)}).encode('utf-8'))
            except Exception as e:
                self.send_response(500)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"error": str(e)}).encode('utf-8'))
            return

        self.send_response(404)
        self.end_headers()

def run_server():
    port = PORT
    for attempt in range(10):
        try:
            with socketserver.TCPServer(("", port), EmulatorHandler) as httpd:
                print("=" * 64)
                print(" 🌵 CACTUS DISPLAY - ESP32 UI WEB EMULATOR")
                print("=" * 64)
                print(f" Web Emulator URL : http://localhost:{port}")
                print(f" Watching Source   : {SOURCE_FILE}")
                print(f" Hot Reload        : Active (polls changes automatically)")
                print("=" * 64)
                print(" Press Ctrl+C to stop the emulator server.\n")
                sys.stdout.flush()
                httpd.serve_forever()
                break
        except OSError:
            port += 1

if __name__ == "__main__":
    run_server()
