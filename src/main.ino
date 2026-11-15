#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include <esp_heap_caps.h>
#include <DHT.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Audio.h"
#include "driver/i2s_std.h"
#include "voice_clips.h"
#include "config.h"
#include "card_assets.h"

// --- FEATURE FLAGS ---
#define ENABLE_RADAR_PAGE 1 // Set to 1 to enable Retro Flight Radar view

// --- APP VIEWS ---
enum AppView {
    VIEW_CACTUS = 0,
    VIEW_RADIO  = 1,
#if ENABLE_RADAR_PAGE
    VIEW_RADAR  = 2
#endif
};

// --- RADIO STATIONS DEFINITION (MALAYSIA) ---
struct RadioStation {
    float freq;
    const char *name;
    const char *genre;
    const char *url;
};

const RadioStation STATIONS[] = {
    {88.5f,  "NASIONAL FM",  "BERITA & HIBURAN RTM",       "http://22243.live.streamtheworld.com/NASIONAL_FM.mp3"},
    {89.9f,  "BFM 89.9",     "BUSINESS & CURRENT AFFAIRS", "http://22253.live.streamtheworld.com/BFM.mp3"},
    {90.3f,  "TRAXX FM",     "ENGLISH URBAN & TOP HITS",   "http://22243.live.streamtheworld.com/TRAXX_FM.mp3"},
    {97.6f,  "HOT FM",       "MUZIK PALING HANGAT",        "http://stream.rcs.revma.com/drakdf8mtd3vv"},
    {98.3f,  "RADIO KLASIK", "LAGU KLASIK & NOSTALGIA",    "http://22283.live.streamtheworld.com/RADIO_KLASIK.mp3"},
    {101.3f, "KOOL 101",     "SEMASA & KONTEMPORARI",      "http://stream.rcs.revma.com/3930238mtd3vv"},
    {105.3f, "SURIA FM",     "SEGALANYA HIBURAN",          "http://28103.live.streamtheworld.com/SURIA_FM.mp3"}
};
const int NUM_STATIONS = sizeof(STATIONS) / sizeof(STATIONS[0]);

// --- DHT11 SENSOR CONFIGURATION ---
#define DHTPIN 17
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// --- ROTARY ENCODER CONFIGURATION (P3 Header) ---
#define ENCODER_CLK 6   // Channel A
#define ENCODER_DT  7   // Channel B
#define ENCODER_SW  15  // Push Button

// --- MODE BUTTON CONFIGURATION ---
#define MODE_BUTTON_PIN 18 // Momentary switch (IO18 to GND) to toggle pages

// --- SOIL MOISTURE SENSOR (HW-390) ---
#define SOIL_PIN 16     // Analog OUT connected to IO16

// --- GT911 TOUCH CONFIGURATION ---
#define TOUCH_SDA 8
#define TOUCH_SCL 4
#define TOUCH_RST 38
#define TOUCH_INT 3

// --- AUDIO CONFIGURATION (NS4168 Class-D I2S Amp) ---
#define I2S_BCLK 42
#define I2S_LRC  2
#define I2S_DOUT 41

Audio audio;

// --- ROTARY ENCODER GRAY CODE STATE MACHINE (Buxton Algorithm) ---
#define DIR_NONE 0x0
#define DIR_CW   0x10
#define DIR_CCW  0x20

#define R_START     0x0
#define R_CW_FINAL  0x1
#define R_CW_BEGIN  0x2
#define R_CW_NEXT   0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT  0x6

// State transition table for full-step quadrature encoders (resting state 0b11)
static const uint8_t ttable[7][4] = {
  // 00:                 01:                 10:                 11:
  {R_START,           R_CW_BEGIN,         R_CCW_BEGIN,        R_START},           // R_START
  {R_CW_NEXT,         R_START,            R_CW_FINAL,         R_START | DIR_CW},  // R_CW_FINAL
  {R_CW_NEXT,         R_CW_BEGIN,         R_START,            R_START},           // R_CW_BEGIN
  {R_CW_NEXT,         R_CW_BEGIN,         R_CW_FINAL,         R_START},           // R_CW_NEXT
  {R_CCW_NEXT,        R_START,            R_CCW_BEGIN,        R_START},           // R_CCW_BEGIN
  {R_CCW_NEXT,        R_CCW_FINAL,        R_START,            R_START | DIR_CCW}, // R_CCW_FINAL
  {R_CCW_NEXT,        R_CCW_FINAL,        R_CCW_BEGIN,        R_START},           // R_CCW_NEXT
};

volatile long encoder_pos = 0;
volatile bool encoder_changed = false;
volatile uint8_t encoder_state = R_START;

void IRAM_ATTR handle_encoder() {
    uint8_t pinstate = (digitalRead(ENCODER_CLK) << 1) | digitalRead(ENCODER_DT);
    encoder_state = ttable[encoder_state & 0x0F][pinstate];
    uint8_t result = encoder_state & 0x30;
    if (result == DIR_CW) {
        encoder_pos++;
        encoder_changed = true;
    } else if (result == DIR_CCW) {
        encoder_pos--;
        encoder_changed = true;
    }
}

// --- DISPLAY PIN CONFIGURATION FOR JC4827W543 ---
#define GFX_BL 1

// QSPI Bus
Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    45 /* CS */, 47 /* SCK */, 21 /* D0 */, 48 /* D1 */, 40 /* D2 */, 39 /* D3 */
);

// Display driver (raw hardware)
Arduino_GFX *g = new Arduino_NV3041A(
    bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */, true /* IPS */
);

// Canvas wrapper — REQUIRED for QSPI displays
Arduino_GFX *gfx = new Arduino_Canvas(480 /* width */, 272 /* height */, g);

// --- LVGL CONFIGURATION ---
static const uint32_t screenWidth  = 480;
static const uint32_t screenHeight = 272;

// LVGL Display flushing
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

    // Track dirty row bounds across all slices in the current frame
    static int16_t frame_min_y = 32767;
    static int16_t frame_max_y = -1;

    if (area->y1 < frame_min_y) frame_min_y = area->y1;
    if (area->y2 > frame_max_y) frame_max_y = area->y2;

    // ONLY transfer pixels over QSPI to the display hardware on the final slice of the frame!
    // This stops slice-by-slice repainting and eliminates display flickering.
    if (lv_disp_flush_is_last(disp_drv)) {
        if (frame_min_y < 0) frame_min_y = 0;
        if (frame_max_y >= (int16_t)screenHeight) frame_max_y = screenHeight - 1;

        int16_t dirty_h = frame_max_y - frame_min_y + 1;
        if (dirty_h > 0) {
            uint16_t *fb = ((Arduino_Canvas *)gfx)->getFramebuffer();
            if (dirty_h >= 240 || !fb) {
                gfx->flush();
            } else {
                // Partial update: push only the affected horizontal scanlines
                // This is up to 8x faster and completely prevents panel-wide tearing
                g->draw16bitRGBBitmap(0, frame_min_y, fb + ((uint32_t)frame_min_y * screenWidth), screenWidth, dirty_h);
            }
        }
        frame_min_y = 32767;
        frame_max_y = -1;
    }
    lv_disp_flush_ready(disp_drv);
}

// --- TOUCH DRIVER (GT911) ---
uint8_t gt911_addr = 0x5D;
bool touch_available = false;

void init_touch() {
    pinMode(TOUCH_RST, OUTPUT);
    digitalWrite(TOUCH_RST, LOW);
    delay(20);
    digitalWrite(TOUCH_RST, HIGH);
    delay(50);

    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    Wire.setClock(400000);

    Wire.beginTransmission(0x5D);
    if (Wire.endTransmission() == 0) {
        gt911_addr = 0x5D;
        touch_available = true;
        Serial.println("[TOUCH] GT911 found at 0x5D");
    } else {
        Wire.beginTransmission(0x14);
        if (Wire.endTransmission() == 0) {
            gt911_addr = 0x14;
            touch_available = true;
            Serial.println("[TOUCH] GT911 found at 0x14");
        }
    }
}

bool read_touch(int16_t *x, int16_t *y) {
    if (!touch_available) return false;

    Wire.beginTransmission(gt911_addr);
    Wire.write(0x81);
    Wire.write(0x4E);
    if (Wire.endTransmission(false) != 0) return false;

    Wire.requestFrom((uint8_t)gt911_addr, (uint8_t)1);
    if (!Wire.available()) return false;
    uint8_t status = Wire.read();

    bool has_touch = false;
    if ((status & 0x80) && (status & 0x0F) > 0) {
        Wire.beginTransmission(gt911_addr);
        Wire.write(0x81);
        Wire.write(0x50);
        if (Wire.endTransmission(false) == 0) {
            Wire.requestFrom((uint8_t)gt911_addr, (uint8_t)4);
            if (Wire.available() >= 4) {
                uint8_t x_l = Wire.read();
                uint8_t x_h = Wire.read();
                uint8_t y_l = Wire.read();
                uint8_t y_h = Wire.read();

                int16_t raw_x = x_l | (x_h << 8);
                int16_t raw_y = y_l | (y_h << 8);

                *x = constrain(raw_x, 0, (int16_t)(screenWidth - 1));
                *y = constrain(raw_y, 0, (int16_t)(screenHeight - 1));
                has_touch = true;
            }
        }
    }

    if (status & 0x80) {
        Wire.beginTransmission(gt911_addr);
        Wire.write(0x81);
        Wire.write(0x4E);
        Wire.write(0x00);
        Wire.endTransmission(true);
    }
    return has_touch;
}

void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    int16_t touchX = 0, touchY = 0;
    if (read_touch(&touchX, &touchY)) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

// --- AUDIO ROUTINES (Modern ESP-IDF v5 i2s_std API) ---
void init_audio() {
    Audio::audio_info_callback = [](Audio::msg_t m) {
        if (m.msg) Serial.printf("[AUDIO INFO] %s\n", m.msg);
    };
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // Max volume (0...21)
    audio.setConnectionTimeout(2500, 3500);
    Serial.println("[AUDIO] Audio initialized via ESP32-audioI2S");
}

void play_voice_clip(const int16_t *data, size_t sample_count, float volume = 1.0f) {
    if (!data || sample_count == 0) return;
    i2s_chan_handle_t tx_handle = audio.getI2sTxHandle();
    if (!tx_handle) return;
    
    if (audio.isRunning()) {
        audio.stopSong();
    }

    // 16kHz clip played at 48kHz by repeating each sample 3 times
    int32_t buffer[64 * 2]; // 64 stereo frames
    int frame_idx = 0;

    for (size_t i = 0; i < sample_count; i++) {
        int16_t raw_sample = (int16_t)pgm_read_word(&data[i]);
        int32_t sample32 = ((int32_t)(raw_sample * volume)) << 16;

        for (int rep = 0; rep < 3; rep++) {
            buffer[frame_idx * 2]     = sample32; // Left
            buffer[frame_idx * 2 + 1] = sample32; // Right
            frame_idx++;

            if (frame_idx >= 64) {
                size_t written;
                i2s_channel_write(tx_handle, buffer, sizeof(buffer), &written, portMAX_DELAY);
                frame_idx = 0;
            }
        }
    }
    if (frame_idx > 0) {
        size_t written;
        i2s_channel_write(tx_handle, buffer, frame_idx * 2 * sizeof(int32_t), &written, portMAX_DELAY);
    }
}

void play_tone(float freq, int duration_ms, float volume = 1.0f) {
    if (freq <= 0) {
        delay(duration_ms);
        return;
    }
    i2s_chan_handle_t tx_handle = audio.getI2sTxHandle();
    if (!tx_handle) return;

    int total_samples = (48000 * duration_ms) / 1000;
    int32_t buffer[64 * 2];
    int frame_idx = 0;

    float phase = 0.0f;
    float phase_inc = (2.0f * PI * freq) / 48000.0f;
    float amp = 32767.0f * volume;

    for (int i = 0; i < total_samples; i++) {
        int32_t sample32 = ((int32_t)(sinf(phase) * amp)) << 16;
        phase += phase_inc;
        if (phase >= 2.0f * PI) phase -= 2.0f * PI;

        buffer[frame_idx * 2]     = sample32;
        buffer[frame_idx * 2 + 1] = sample32;
        frame_idx++;

        if (frame_idx >= 64) {
            size_t written;
            i2s_channel_write(tx_handle, buffer, sizeof(buffer), &written, portMAX_DELAY);
            frame_idx = 0;
        }
    }
    if (frame_idx > 0) {
        size_t written;
        i2s_channel_write(tx_handle, buffer, frame_idx * 2 * sizeof(int32_t), &written, portMAX_DELAY);
    }
}

// Continuous filtered FM radio static hiss: "sshshshshhh"
// Fast LFSR pink noise generator + generous DMA buffer queue to prevent any stutter/underrun
void output_continuous_static(float volume = 0.08f) {
    i2s_chan_handle_t tx_handle = audio.getI2sTxHandle();
    if (!tx_handle) return;

    static int32_t buffer[128 * 2];
    static int16_t last_sample = 0;
    static uint32_t lfsr = 0xACE14519u;
    int32_t vol_scale = (int32_t)(volume * 32767.0f);

    // Push up to 8 blocks (1024 stereo frames = ~21.3ms buffer)
    for (int b = 0; b < 8; b++) {
        for (int i = 0; i < 128; i++) {
            // Galois LFSR 32-bit PRNG (single cycle, zero CPU overhead)
            lfsr = (lfsr >> 1) ^ (-(int32_t)(lfsr & 1u) & 0xD0000001u);
            int16_t white = (int16_t)((((int32_t)(lfsr & 0xFFFF) - 32768) * vol_scale) >> 15);

            // One-pole IIR low-pass filter (~3.5kHz cutoff) for authentic warm FM carrier hiss
            int16_t pink = (last_sample * 3 + white) / 4;
            last_sample = pink;

            int32_t sample32 = ((int32_t)pink) << 16;
            buffer[i * 2]     = sample32;
            buffer[i * 2 + 1] = sample32;
        }

        size_t written = 0;
        esp_err_t err = i2s_channel_write(tx_handle, buffer, sizeof(buffer), &written, pdMS_TO_TICKS(1));
        if (err != ESP_OK || written == 0) break; // DMA FIFO is completely full
    }
}

// Audio callbacks for live streaming diagnostics
void audio_info(const char *info) {
    Serial.printf("[AUDIO INFO] %s\n", info);
}
void audio_showstation(const char *info) {
    Serial.printf("[AUDIO STATION] %s\n", info);
}
void audio_showstreamtitle(const char *info) {
    Serial.printf("[AUDIO TITLE] %s\n", info);
}

void play_boot_chime() {
    Serial.println("[AUDIO] Playing boot chime...");
    play_tone(523, 70, 1.0f);   // C5
    play_tone(659, 70, 1.0f);   // E5
    play_tone(784, 90, 1.0f);   // G5
    play_tone(1046, 160, 1.0f); // C6
}

void play_click_sound() {
    play_tone(988, 30, 1.0f);   // B5
    play_tone(1319, 80, 1.0f);  // E6
}

// --- CUSTOM FONTS ---
LV_FONT_DECLARE(press_start_2p_10);
LV_FONT_DECLARE(press_start_2p_14);
LV_FONT_DECLARE(press_start_2p_16);
LV_FONT_DECLARE(press_start_2p_24);
LV_FONT_DECLARE(press_start_2p_32);

// --- COLORS ---
#define COLOR_BG            lv_color_hex(0x0C0A08) // Deep Vintage Warm Charcoal
#define COLOR_FRAME_BD      lv_color_hex(0x28394E) // Cyber frame border
#define COLOR_FRAME_HL1     lv_color_hex(0xFF2A70) // Cyber neon pink/magenta
#define COLOR_FRAME_HL2     lv_color_hex(0xF59E0B) // Cyber amber
#define COLOR_FRAME_HL3     lv_color_hex(0x38BDF8) // Cyber cyan

#define COLOR_PHOSPHOR_GREEN lv_color_hex(0x1A6E1A) // Deep Vintage Phosphor Green (radio frequency readout)
#define COLOR_TUNER_OFFWHITE lv_color_hex(0xC2B8AC) // Warm Aged Bone / Off-White (from radio dial numbers)
#define COLOR_CLOCK         COLOR_PHOSPHOR_GREEN    // Clock (same as radio frequency)
#define COLOR_DATE          lv_color_hex(0xB82A24)  // Date (same as station name)
#define COLOR_DIVIDER       lv_color_hex(0x64748B)  // Vertical divider line (neutral grey)

#define COLOR_CARD_TEMP_BD  lv_color_hex(0xF27C84)  // Card 1 Temp border (warm salmon-coral)
#define COLOR_CARD_TEMP_BG  lv_color_hex(0x180D18)  // Card 1 Temp background (gamma-calibrated dark plum)
#define COLOR_TEXT_TEMP     lv_color_hex(0xFF6584)  // Card 1 Temp title label

#define COLOR_CARD_HUM_BD   lv_color_hex(0x52DCF2)  // Card 2 Humidity border (luminous aqua-cyan)
#define COLOR_CARD_HUM_BG   lv_color_hex(0x040D18)  // Card 2 Humidity background (gamma-calibrated deep midnight navy)
#define COLOR_TEXT_HUM      lv_color_hex(0xBAE6FD)  // Card 2 Humidity title label

#define COLOR_CARD_SOIL_BD  lv_color_hex(0xE6EC62)  // Card 3 Soil border (luminous lemon-chartreuse)
#define COLOR_CARD_SOIL_BG  lv_color_hex(0x041012)  // Card 3 Soil background (gamma-calibrated ultra-dark forest-teal)
#define COLOR_TEXT_SOIL     lv_color_hex(0xEAF498)  // Card 3 Soil title label (soft chartreuse/lime)
#define COLOR_SOIL_TRACK    lv_color_hex(0x13202D)  // Soil bar background track (gamma-calibrated muted slate-blue)
#define COLOR_SOIL_FILL_TOP lv_color_hex(0xD4F594)  // Soil bar top highlight (creamy chartreuse)
#define COLOR_SOIL_FILL     lv_color_hex(0x9EE062)  // Soil bar progress fill (warm chartreuse-lime)

AppView current_view = VIEW_CACTUS;
float radio_freq = 89.9f;          // Default to BFM 89.9 (89.9 MHz)
int last_locked_station = -1;
int pending_station = 1;           // Station index 1 (BFM 89.9)
unsigned long last_tune_time = 0;
bool is_station_streaming = false;

// --- UI Objects ---
lv_obj_t *cactus_view;
lv_obj_t *radio_view;
#if ENABLE_RADAR_PAGE
lv_obj_t *radar_view;

// --- Radar UI Objects & State ---
lv_obj_t *lbl_radar_count = NULL;
lv_obj_t *radar_scope_obj = NULL;
lv_obj_t *flight_table = NULL;

struct FlightInfo {
    char time_str[10];   // e.g. "12:25P"
    char airline[16];    // e.g. "AirAsia", "Batik Air"
    char flight[12];     // e.g. "ZH335"
    char origin[6];      // e.g. "SZX"
    bool is_estimated;   // true = estimated (green/amber), false = scheduled (cyan)
    int altitude;
    int speed;
    int track;
    float dist;
    float dir;
    int rate;
    int runway;          // 4 or 22
};

#define MAX_FLIGHTS 25
static FlightInfo flight_list[MAX_FLIGHTS];
static int flight_count = 0;
static int selected_flight_idx = 0;
static bool live_adsb_received = false;

// Full Realistic Penang International Airport (WMKP / PEN) Arrival Schedule (20 flights)
static const FlightInfo PENANG_SCHEDULE[] = {
    // time_str, airline, flight, origin, is_estimated, altitude, speed, track, dist, dir, rate, runway
    {"02:16P", "Singapore",   "SQ136",  "SIN", true,  4500, 210, 317,  8.2f, 224.0f, -800,  4},
    {"02:25P", "AirAsia",     "AK6126", "KUL", true,  5200, 230,  44, 11.5f, 224.0f, -750,  4},
    {"02:35P", "Batik Air",   "OD2104", "KUL", true,  4100, 220,  44,  9.0f, 224.0f, -650,  4},
    {"02:45P", "China Air",   "CI731",  "TPE", true,  6800, 260, 224, 14.8f,  44.0f, -900, 22},
    {"02:55P", "Firefly",     "FY2701", "LGK", false, 4800, 240, 224, 12.0f,  44.0f, -700, 22},
    {"03:05P", "AirAsia",     "AK1720", "SIN", true,  3900, 210, 224,  7.5f,  44.0f, -800, 22},
    {"03:15P", "Malaysia",    "MH1142", "KUL", false, 5600, 250,  44, 13.0f, 224.0f, -600,  4},
    {"03:25P", "Scoot",       "TR136",  "SIN", false, 7200, 270,  44, 16.5f, 224.0f, -500,  4},
    {"03:35P", "Thai Air",    "FD612",  "DMK", true,  4200, 230, 224,  9.8f,  44.0f, -850, 22},
    {"03:45P", "AirAsia",     "AK5433", "KCH", true,  3600, 200,  44,  6.8f, 224.0f, -900,  4},
    {"03:55P", "Shenzhen",    "ZH335",  "SZX", false, 6500, 260,  44, 15.0f, 224.0f, -550,  4},
    {"04:05P", "Batik Air",   "OD2202", "LGK", true,  4500, 220, 224, 10.5f,  44.0f, -750, 22},
    {"04:15P", "Firefly",     "FY1672", "SZB", false, 5100, 230,  44, 12.2f, 224.0f, -600,  4},
    {"04:25P", "Batik Air",   "ID7161", "CGK", false, 6200, 250,  44, 14.2f, 224.0f, -650,  4},
    {"04:35P", "Malaysia",    "MH1150", "KUL", false, 5800, 250,  44, 13.5f, 224.0f, -500,  4},
    {"04:45P", "China South", "CZ395",  "CAN", false, 7800, 280, 224, 17.0f,  44.0f, -400, 22},
    {"04:55P", "Emirates",    "EK342",  "DXB", false, 8200, 290,  44, 18.5f, 224.0f, -450,  4},
    {"05:05P", "Cathay",      "CX721",  "HKG", false, 7400, 280, 224, 16.2f,  44.0f, -500, 22},
    {"05:15P", "Qatar Air",   "QR844",  "DOH", false, 8500, 290,  44, 19.0f, 224.0f, -400,  4},
    {"05:25P", "AirAsia",     "AK6130", "KUL", false, 5400, 240,  44, 12.0f, 224.0f, -700,  4}
};
static const int NUM_SCHEDULE_FLIGHTS = sizeof(PENANG_SCHEDULE) / sizeof(PENANG_SCHEDULE[0]);

static float radar_sweep_angle = 0.0f;
static unsigned long last_flight_fetch = 0;
#endif

// Cactus Dashboard UI elements
enum WeatherType {
    WEATHER_CLEAR_DAY,
    WEATHER_CLEAR_NIGHT,
    WEATHER_PARTLY_CLOUDY_DAY,
    WEATHER_PARTLY_CLOUDY_NIGHT,
    WEATHER_CLOUDY,
    WEATHER_RAIN,
    WEATHER_THUNDERSTORM
};
WeatherType current_weather_type = WEATHER_PARTLY_CLOUDY_NIGHT;

lv_obj_t *lbl_clock_time = NULL;
lv_obj_t *lbl_clock_date = NULL;
lv_obj_t *weather_icon_obj = NULL;
lv_obj_t *lbl_weather_loc = NULL;
lv_obj_t *lbl_weather_temp = NULL;
lv_obj_t *lbl_weather_cond = NULL;

lv_obj_t *card_temp = NULL;
lv_obj_t *icon_temp = NULL;
lv_obj_t *lbl_temp_hdr = NULL;
lv_obj_t *lbl_temp_val = NULL;
lv_obj_t *lbl_temp_unit = NULL;

lv_obj_t *card_hum = NULL;
lv_obj_t *icon_hum = NULL;
lv_obj_t *lbl_humid_hdr = NULL;
lv_obj_t *lbl_humid_val = NULL;
lv_obj_t *lbl_humid_unit = NULL;

lv_obj_t *card_soil = NULL;
lv_obj_t *icon_soil = NULL;
lv_obj_t *lbl_soil_hdr = NULL;
lv_obj_t *lbl_soil_val = NULL;
lv_obj_t *lbl_soil_unit = NULL;
lv_obj_t *bar_soil_bg = NULL;
lv_obj_t *bar_soil_fill = NULL;

#define SPEC_COLS 36
lv_obj_t *spectrum_visualizer_obj = NULL;
static int spec_heights[SPEC_COLS] = {
    5, 7, 7, 6, 8, 12, 14, 15, 17, 18, 16, 17, 12, 10, 8, 12, 14, 11,
    10, 15, 16, 17, 18, 16, 14, 13, 8, 6, 5, 6, 8, 12, 15, 14, 16, 10
};

// Radio UI elements
lv_obj_t *r_freq_val;
lv_obj_t *r_freq_unit;
lv_obj_t *r_station_name;
lv_obj_t *r_genre_name;
lv_obj_t *r_status_badge;
lv_obj_t *r_needle;
#define EQ_COLS 8
#define EQ_SEGS 6
lv_obj_t *eq_visualizer_obj = NULL;
static int eq_levels[EQ_COLS] = {2, 3, 4, 5, 4, 3, 2, 1};

static inline void draw_box(lv_draw_ctx_t *ctx, int x, int y, int w, int h, lv_color_t color, lv_coord_t radius = 0) {
    lv_area_t a;
    a.x1 = x;
    a.y1 = y;
    a.x2 = x + w - 1;
    a.y2 = y + h - 1;
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.radius = radius;
    dsc.bg_opa = LV_OPA_COVER;
    dsc.bg_color = color;
    dsc.border_width = 0;
    lv_draw_rect(ctx, &dsc, &a);
}

static inline void draw_border_box(lv_draw_ctx_t *ctx, int x, int y, int w, int h, lv_color_t bg_col, lv_color_t bd_col, lv_coord_t bd_w, lv_coord_t radius = 0) {
    lv_area_t a;
    a.x1 = x;
    a.y1 = y;
    a.x2 = x + w - 1;
    a.y2 = y + h - 1;
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.radius = radius;
    dsc.bg_opa = LV_OPA_COVER;
    dsc.bg_color = bg_col;
    dsc.border_width = bd_w;
    dsc.border_color = bd_col;
    dsc.border_opa = LV_OPA_COVER;
    lv_draw_rect(ctx, &dsc, &a);
}

// Pixel Art Weather Helpers
static void draw_star(lv_draw_ctx_t *ctx, int x, int y) {
    draw_box(ctx, x, y, 2, 2, lv_color_hex(0xFFFFFF), 0);
    draw_box(ctx, x, y - 1, 2, 1, lv_color_hex(0xFEF08A), 0);
    draw_box(ctx, x, y + 2, 2, 1, lv_color_hex(0xFEF08A), 0);
    draw_box(ctx, x - 1, y, 1, 2, lv_color_hex(0xFEF08A), 0);
    draw_box(ctx, x + 2, y, 1, 2, lv_color_hex(0xFEF08A), 0);
}

static void draw_moon_icon(lv_draw_ctx_t *ctx, int mx, int my, bool full_moon) {
    // Outer Moon (Glowing Yellow)
    draw_border_box(ctx, mx, my, 22, 22, lv_color_hex(0xFDE047), lv_color_hex(0xF59E0B), 1, 11);
    // Cutout to create crescent (Background color)
    draw_box(ctx, mx + 7, my - 2, 18, 18, COLOR_BG, 9);
    // Crater / Inner glow highlight
    draw_box(ctx, mx + 3, my + 8, 3, 3, lv_color_hex(0xFEF08A), 1);

    if (full_moon) {
        // Twinkling stars in night sky
        draw_star(ctx, mx + 25, my + 2);
        draw_star(ctx, mx + 22, my + 22);
        draw_box(ctx, mx + 32, my + 13, 2, 2, lv_color_hex(0xFFFFFF), 0);
        draw_box(ctx, mx - 4, my + 18, 2, 2, lv_color_hex(0xCBD5E1), 0);
    }
}

static void draw_sun_icon(lv_draw_ctx_t *ctx, int sx, int sy, bool full_sun) {
    lv_color_t ray_col = lv_color_hex(0xF59E0B);
    draw_box(ctx, sx + 9, sy - 6, 3, 5, ray_col, 1);
    draw_box(ctx, sx + 9, sy + 22, 3, 5, ray_col, 1);
    draw_box(ctx, sx - 6, sy + 9, 5, 3, ray_col, 1);
    draw_box(ctx, sx + 22, sy + 9, 5, 3, ray_col, 1);
    draw_box(ctx, sx - 2, sy - 2, 4, 4, ray_col, 1);
    draw_box(ctx, sx + 19, sy - 2, 4, 4, ray_col, 1);
    draw_box(ctx, sx - 2, sy + 19, 4, 4, ray_col, 1);
    if (full_sun) {
        draw_box(ctx, sx + 19, sy + 19, 4, 4, ray_col, 1);
    }

    // Sun Body
    draw_border_box(ctx, sx, sy, 21, 21, lv_color_hex(0xFACC15), lv_color_hex(0xEAB308), 1, 10);
    // Sun Highlight
    draw_box(ctx, sx + 4, sy + 4, 4, 4, lv_color_hex(0xFEF08A), 2);
}

static void draw_cloud_icon(lv_draw_ctx_t *ctx, int cx, int cy, bool is_dark) {
    lv_color_t base_col = is_dark ? lv_color_hex(0x475569) : lv_color_hex(0x94A3B8);
    lv_color_t bd_col   = is_dark ? lv_color_hex(0x0F172A) : lv_color_hex(0x1E293B);
    lv_color_t lobe1    = is_dark ? lv_color_hex(0x64748B) : lv_color_hex(0xE0F2FE);
    lv_color_t lobe2    = is_dark ? lv_color_hex(0x94A3B8) : lv_color_hex(0xFFFFFF);
    lv_color_t lobe3    = is_dark ? lv_color_hex(0x64748B) : lv_color_hex(0xE0F2FE);
    lv_color_t hl       = is_dark ? lv_color_hex(0x94A3B8) : lv_color_hex(0xFFFFFF);

    draw_border_box(ctx, cx, cy + 10, 36, 12, base_col, bd_col, 1, 6);
    draw_border_box(ctx, cx + 2, cy + 5, 14, 14, lobe1, bd_col, 1, 7);
    draw_border_box(ctx, cx + 11, cy, 18, 18, lobe2, bd_col, 1, 9);
    draw_border_box(ctx, cx + 22, cy + 6, 13, 13, lobe3, bd_col, 1, 6);
    draw_box(ctx, cx + 12, cy + 2, 14, 7, hl, 3);
}

static void draw_rain_drops(lv_draw_ctx_t *ctx, int rx, int ry) {
    lv_color_t r_col = lv_color_hex(0x38BDF8);
    draw_box(ctx, rx + 4,  ry,     2, 6, r_col, 1);
    draw_box(ctx, rx + 12, ry + 3, 2, 6, r_col, 1);
    draw_box(ctx, rx + 20, ry + 1, 2, 6, r_col, 1);
    draw_box(ctx, rx + 28, ry + 4, 2, 6, r_col, 1);
}

static void draw_lightning(lv_draw_ctx_t *ctx, int lx, int ly) {
    draw_box(ctx, lx + 4, ly, 4, 7, lv_color_hex(0xFACC15), 1);
    draw_box(ctx, lx + 1, ly + 6, 7, 3, lv_color_hex(0xFEF08A), 1);
    draw_box(ctx, lx, ly + 8, 3, 8, lv_color_hex(0xFACC15), 1);
}

// Custom Draw Callback: Dynamic Weather Icon (Sun, Moon, Clouds, Rain, Storm)
static void weather_icon_draw_cb(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    lv_draw_ctx_t *ctx = lv_event_get_draw_ctx(e);
    if (!ctx) return;
    lv_area_t c;
    lv_obj_get_coords(obj, &c);

    switch (current_weather_type) {
        case WEATHER_CLEAR_DAY:
            draw_sun_icon(ctx, c.x1 + 15, c.y1 + 12, true);
            break;
            
        case WEATHER_CLEAR_NIGHT:
            draw_moon_icon(ctx, c.x1 + 10, c.y1 + 10, true);
            break;
            
        case WEATHER_PARTLY_CLOUDY_DAY:
            draw_sun_icon(ctx, c.x1 + 9, c.y1 + 6, false);
            draw_cloud_icon(ctx, c.x1 + 14, c.y1 + 18, false);
            break;
            
        case WEATHER_PARTLY_CLOUDY_NIGHT:
            draw_moon_icon(ctx, c.x1 + 8, c.y1 + 5, false);
            draw_cloud_icon(ctx, c.x1 + 14, c.y1 + 18, false);
            break;
            
        case WEATHER_CLOUDY:
            draw_cloud_icon(ctx, c.x1 + 6, c.y1 + 8, false);
            draw_cloud_icon(ctx, c.x1 + 14, c.y1 + 18, false);
            break;
            
        case WEATHER_RAIN:
            draw_cloud_icon(ctx, c.x1 + 8, c.y1 + 8, true);
            draw_rain_drops(ctx, c.x1 + 8, c.y1 + 32);
            break;
            
        case WEATHER_THUNDERSTORM:
            draw_cloud_icon(ctx, c.x1 + 8, c.y1 + 6, true);
            draw_lightning(ctx, c.x1 + 20, c.y1 + 22);
            draw_rain_drops(ctx, c.x1 + 8, c.y1 + 32);
            break;
    }
}

// Custom Draw Callback: Location Pin Icon
static void loc_pin_draw_cb(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    lv_draw_ctx_t *ctx = lv_event_get_draw_ctx(e);
    if (!ctx) return;
    lv_area_t c;
    lv_obj_get_coords(obj, &c);

    draw_border_box(ctx, c.x1 + 1, c.y1 + 1, 7, 7, COLOR_BG, lv_color_hex(0xCBD5E1), 2, 3);
    draw_box(ctx, c.x1 + 3, c.y1 + 7, 2, 3, lv_color_hex(0xCBD5E1), 0);
}

// Custom Draw Callback: Bottom Spectrum Visualizer Bars
static void spectrum_draw_cb(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);
    if (!draw_ctx) return;

    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);

    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.radius = 1;
    dsc.bg_opa = LV_OPA_COVER;
    dsc.border_width = 0;

    for (int i = 0; i < SPEC_COLS; i++) {
        int h = spec_heights[i];
        if (h < 2) h = 2;
        if (h > 30) h = 30;

        int x_left = coords.x1 + i * 10;
        int x_right = x_left + 4;
        int y_bottom = coords.y2;
        int y_top = coords.y2 - h;

        lv_area_t seg_area;
        seg_area.x1 = x_left;
        seg_area.x2 = x_right;
        seg_area.y1 = y_top;
        seg_area.y2 = y_bottom;

        if (i < 9) {
            dsc.bg_color = lv_color_hex(0xFF1E00); // Deep Flame Red
        } else if (i < 18) {
            dsc.bg_color = lv_color_hex(0xFF5A00); // Fiery Orange-Red
        } else if (i < 27) {
            dsc.bg_color = lv_color_hex(0xFF9600); // Blazing Orange
        } else {
            dsc.bg_color = lv_color_hex(0xFFD000); // Hot Fire Yellow
        }
        lv_draw_rect(draw_ctx, &dsc, &seg_area);
    }
}

static void eq_draw_cb(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);
    if (!draw_ctx) return;

    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);

    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.radius = 1;
    dsc.bg_opa = LV_OPA_COVER;
    dsc.border_width = 0;

    for (int c = 0; c < EQ_COLS; c++) {
        int active = eq_levels[c];
        int x_left = coords.x1 + c * 11;
        int x_right = x_left + 8;

        for (int s = 0; s < EQ_SEGS; s++) {
            int y_bottom = coords.y2 - 2 - s * 7;
            int y_top = y_bottom - 4;

            lv_area_t seg_area;
            seg_area.x1 = x_left;
            seg_area.x2 = x_right;
            seg_area.y1 = y_top;
            seg_area.y2 = y_bottom;

            if (s < active) {
                dsc.bg_color = lv_color_hex(0xFF6D00); // Bright active retro orange
            } else {
                dsc.bg_color = lv_color_hex(0x281000); // Dim background pill
            }
            lv_draw_rect(draw_ctx, &dsc, &seg_area);
        }
    }
}

// Change-detection cache variables for Cactus view (prevents redundant dirty rect redraws)
static int last_disp_moisture = -999;
static float last_disp_temp = -999.0f;
static float last_disp_humid = -999.0f;
static char last_clock_t_str[16] = "";
static char last_clock_d_str[32] = "";

void update_radio_ui();
#if ENABLE_RADAR_PAGE
void update_radar_ui();
void fetch_flight_data();
#endif

void switch_tab(AppView view) {
    current_view = view;
    if (view == VIEW_RADIO) {
        lv_obj_add_flag(cactus_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(radio_view, LV_OBJ_FLAG_HIDDEN);
#if ENABLE_RADAR_PAGE
        if (radar_view) lv_obj_add_flag(radar_view, LV_OBJ_FLAG_HIDDEN);
#endif
        Serial.println("[NAV] Switched to RADIO view");
        last_tune_time = millis(); // Trigger lock-in after 350ms if on a station
        update_radio_ui();
#if ENABLE_RADAR_PAGE
    } else if (view == VIEW_RADAR) {
        if (is_station_streaming) {
            audio.stopSong();
            is_station_streaming = false;
            last_locked_station = -1;
            pending_station = -1;
        }
        lv_obj_add_flag(cactus_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(radio_view, LV_OBJ_FLAG_HIDDEN);
        if (radar_view) lv_obj_clear_flag(radar_view, LV_OBJ_FLAG_HIDDEN);
        Serial.printf("[NAV] Switched to RADAR view (Free heap: %u)\n", ESP.getFreeHeap());
        update_radar_ui();
        last_flight_fetch = millis() - 28000; // Trigger fetch 2 seconds after page entry so initial UI renders first
#endif
    } else {
        if (is_station_streaming) {
            audio.stopSong();
            is_station_streaming = false;
            last_locked_station = -1;
            pending_station = -1;
        }
        lv_obj_clear_flag(cactus_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(radio_view, LV_OBJ_FLAG_HIDDEN);
#if ENABLE_RADAR_PAGE
        if (radar_view) lv_obj_add_flag(radar_view, LV_OBJ_FLAG_HIDDEN);
#endif
        // Reset cached values so cactus cards refresh immediately
        last_disp_moisture = -999;
        last_disp_temp = -999.0f;
        last_disp_humid = -999.0f;
        last_clock_t_str[0] = '\0';
        last_clock_d_str[0] = '\0';
        Serial.println("[NAV] Switched to CACTUS view");
    }
}

void build_ui()
{
    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, COLOR_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    // ==========================================
    // VIEW 1: OVERHAULED CYBER CACTUS MONITOR
    // ==========================================
    cactus_view = lv_obj_create(screen);
    lv_obj_set_pos(cactus_view, 0, 0);
    lv_obj_set_size(cactus_view, screenWidth, screenHeight);
    lv_obj_set_style_bg_color(cactus_view, COLOR_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cactus_view, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(cactus_view, 0, LV_PART_MAIN);
    lv_obj_clear_flag(cactus_view, LV_OBJ_FLAG_SCROLLABLE);

    // --- TOP STATUS ROW (Safely inset within physical aperture) ---
    // 1. Digital Clock (Time)
    lbl_clock_time = lv_label_create(cactus_view);
    lv_label_set_text(lbl_clock_time, "18:05");
    lv_obj_set_style_text_color(lbl_clock_time, COLOR_CLOCK, LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_clock_time, &press_start_2p_24, LV_PART_MAIN);
    lv_obj_set_pos(lbl_clock_time, 26, 46);

    // 2. Date
    lbl_clock_date = lv_label_create(cactus_view);
    lv_label_set_text(lbl_clock_date, "Sun, 01 Nov");
    lv_obj_set_style_text_color(lbl_clock_date, COLOR_DATE, LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_clock_date, &press_start_2p_10, LV_PART_MAIN);
    lv_obj_set_pos(lbl_clock_date, 26, 80);

    // 3. Vertical Divider Line
    lv_obj_t *clock_div = lv_obj_create(cactus_view);
    lv_obj_set_pos(clock_div, 170, 44);
    lv_obj_set_size(clock_div, 2, 48);
    lv_obj_set_style_bg_color(clock_div, COLOR_DIVIDER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(clock_div, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(clock_div, 0, LV_PART_MAIN);

    // 4. Weather Icon (Dynamic: Sun, Moon, Clouds, Rain, Storm)
    weather_icon_obj = lv_obj_create(cactus_view);
    lv_obj_set_pos(weather_icon_obj, 188, 40);
    lv_obj_set_size(weather_icon_obj, 52, 46);
    lv_obj_set_style_bg_opa(weather_icon_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(weather_icon_obj, 0, LV_PART_MAIN);
    lv_obj_clear_flag(weather_icon_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(weather_icon_obj, weather_icon_draw_cb, LV_EVENT_DRAW_MAIN, NULL);

    // 5. Location Pin + Location Text
    lv_obj_t *pin_icon = lv_obj_create(cactus_view);
    lv_obj_set_pos(pin_icon, 254, 44);
    lv_obj_set_size(pin_icon, 10, 12);
    lv_obj_set_style_bg_opa(pin_icon, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(pin_icon, 0, LV_PART_MAIN);
    lv_obj_clear_flag(pin_icon, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(pin_icon, loc_pin_draw_cb, LV_EVENT_DRAW_MAIN, NULL);

    lbl_weather_loc = lv_label_create(cactus_view);
    lv_label_set_text(lbl_weather_loc, "Penang");
    lv_obj_set_style_text_color(lbl_weather_loc, lv_color_hex(0xCBD5E1), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_weather_loc, &press_start_2p_10, LV_PART_MAIN);
    lv_obj_set_pos(lbl_weather_loc, 268, 44);

    // 6. Weather Outdoor Temperature (Retro Radio Orange #FF6D00)
    lbl_weather_temp = lv_label_create(cactus_view);
    lv_label_set_text(lbl_weather_temp, "27°C");
    lv_obj_set_style_text_color(lbl_weather_temp, lv_color_hex(0xFF6D00), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_weather_temp, &press_start_2p_24, LV_PART_MAIN);
    lv_obj_set_pos(lbl_weather_temp, 254, 60);

    // 7. Weather Condition
    lbl_weather_cond = lv_label_create(cactus_view);
    lv_label_set_text(lbl_weather_cond, "Partly Cloudy");
    lv_obj_set_style_text_color(lbl_weather_cond, lv_color_hex(0x94A3B8), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_weather_cond, &press_start_2p_10, LV_PART_MAIN);
    lv_obj_set_pos(lbl_weather_cond, 254, 86);

    // --- MIDDLE 3 GLOWING SENSOR CARDS (Insets: x=22..430, w=130 each) ---
    // Card 1: TEMP
    card_temp = lv_obj_create(cactus_view);
    lv_obj_set_pos(card_temp, 22, 114);
    lv_obj_set_size(card_temp, 130, 82);
    lv_obj_set_style_bg_color(card_temp, COLOR_CARD_TEMP_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card_temp, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(card_temp, COLOR_CARD_TEMP_BD, LV_PART_MAIN);
    lv_obj_set_style_border_width(card_temp, 2, LV_PART_MAIN);
    lv_obj_set_style_outline_color(card_temp, lv_color_hex(0x04060E), LV_PART_MAIN);
    lv_obj_set_style_outline_width(card_temp, 1, LV_PART_MAIN);
    lv_obj_set_style_outline_opa(card_temp, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(card_temp, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card_temp, 0, LV_PART_MAIN);
    lv_obj_clear_flag(card_temp, LV_OBJ_FLAG_SCROLLABLE);

    icon_temp = lv_img_create(card_temp);
    lv_img_set_src(icon_temp, &img_icon_temp);
    lv_obj_set_pos(icon_temp, 11, 14);

    lbl_temp_hdr = lv_label_create(card_temp);
    lv_label_set_text(lbl_temp_hdr, "TEMP");
    lv_obj_set_style_text_color(lbl_temp_hdr, lv_color_hex(0xFF94A0), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_temp_hdr, &card_font_hdr_11, LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(lbl_temp_hdr, 2, LV_PART_MAIN);
    lv_obj_set_pos(lbl_temp_hdr, 47, 16);

    lbl_temp_val = lv_label_create(card_temp);
    lv_label_set_text(lbl_temp_val, "28.6");
    lv_obj_set_style_text_color(lbl_temp_val, COLOR_TUNER_OFFWHITE, LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_temp_val, &card_font_val_22, LV_PART_MAIN);
    lv_obj_set_pos(lbl_temp_val, 46, 41);

    lbl_temp_unit = lv_label_create(card_temp);
    lv_label_set_text(lbl_temp_unit, "°C");
    lv_obj_set_style_text_color(lbl_temp_unit, COLOR_TUNER_OFFWHITE, LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_temp_unit, &card_font_unit_17, LV_PART_MAIN);
    lv_obj_update_layout(card_temp);
    lv_obj_align_to(lbl_temp_unit, lbl_temp_val, LV_ALIGN_OUT_RIGHT_BOTTOM, 2, 0);

    // Card 2: HUMIDITY
    card_hum = lv_obj_create(cactus_view);
    lv_obj_set_pos(card_hum, 161, 114);
    lv_obj_set_size(card_hum, 130, 82);
    lv_obj_set_style_bg_color(card_hum, COLOR_CARD_HUM_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card_hum, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(card_hum, COLOR_CARD_HUM_BD, LV_PART_MAIN);
    lv_obj_set_style_border_width(card_hum, 2, LV_PART_MAIN);
    lv_obj_set_style_outline_color(card_hum, lv_color_hex(0x030712), LV_PART_MAIN);
    lv_obj_set_style_outline_width(card_hum, 1, LV_PART_MAIN);
    lv_obj_set_style_outline_opa(card_hum, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(card_hum, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card_hum, 0, LV_PART_MAIN);
    lv_obj_clear_flag(card_hum, LV_OBJ_FLAG_SCROLLABLE);

    icon_hum = lv_img_create(card_hum);
    lv_img_set_src(icon_hum, &img_icon_hum);
    lv_obj_set_pos(icon_hum, 12, 15);

    lbl_humid_hdr = lv_label_create(card_hum);
    lv_label_set_text(lbl_humid_hdr, "HUMIDITY");
    lv_obj_set_style_text_color(lbl_humid_hdr, lv_color_hex(0xCCF4FF), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_humid_hdr, &card_font_hdr_11, LV_PART_MAIN);
    lv_obj_set_pos(lbl_humid_hdr, 56, 16);

    lbl_humid_val = lv_label_create(card_hum);
    lv_label_set_text(lbl_humid_val, "68");
    lv_obj_set_style_text_color(lbl_humid_val, COLOR_TUNER_OFFWHITE, LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_humid_val, &card_font_val_22, LV_PART_MAIN);
    lv_obj_set_pos(lbl_humid_val, 58, 41);

    lbl_humid_unit = lv_label_create(card_hum);
    lv_label_set_text(lbl_humid_unit, "%");
    lv_obj_set_style_text_color(lbl_humid_unit, COLOR_TUNER_OFFWHITE, LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_humid_unit, &card_font_unit_17, LV_PART_MAIN);
    lv_obj_update_layout(card_hum);
    lv_obj_align_to(lbl_humid_unit, lbl_humid_val, LV_ALIGN_OUT_RIGHT_BOTTOM, 4, 0);

    // Card 3: SOIL
    card_soil = lv_obj_create(cactus_view);
    lv_obj_set_pos(card_soil, 300, 114);
    lv_obj_set_size(card_soil, 130, 82);
    lv_obj_set_style_bg_color(card_soil, COLOR_CARD_SOIL_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card_soil, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(card_soil, COLOR_CARD_SOIL_BD, LV_PART_MAIN);
    lv_obj_set_style_border_width(card_soil, 2, LV_PART_MAIN);
    lv_obj_set_style_outline_color(card_soil, lv_color_hex(0x03080D), LV_PART_MAIN);
    lv_obj_set_style_outline_width(card_soil, 1, LV_PART_MAIN);
    lv_obj_set_style_outline_opa(card_soil, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(card_soil, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card_soil, 0, LV_PART_MAIN);
    lv_obj_clear_flag(card_soil, LV_OBJ_FLAG_SCROLLABLE);

    icon_soil = lv_img_create(card_soil);
    lv_img_set_src(icon_soil, &img_icon_soil);
    lv_obj_set_pos(icon_soil, 13, 13);

    lbl_soil_hdr = lv_label_create(card_soil);
    lv_label_set_text(lbl_soil_hdr, "SOIL");
    lv_obj_set_style_text_color(lbl_soil_hdr, COLOR_TEXT_SOIL, LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_soil_hdr, &card_font_hdr_11, LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(lbl_soil_hdr, 2, LV_PART_MAIN);
    lv_obj_set_pos(lbl_soil_hdr, 62, 12);

    lbl_soil_val = lv_label_create(card_soil);
    lv_label_set_text(lbl_soil_val, "42");
    lv_obj_set_style_text_color(lbl_soil_val, COLOR_TUNER_OFFWHITE, LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_soil_val, &card_font_val_20, LV_PART_MAIN);
    lv_obj_set_pos(lbl_soil_val, 68, 31);

    lbl_soil_unit = lv_label_create(card_soil);
    lv_label_set_text(lbl_soil_unit, "%");
    lv_obj_set_style_text_color(lbl_soil_unit, COLOR_TUNER_OFFWHITE, LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_soil_unit, &card_font_unit_15, LV_PART_MAIN);
    lv_obj_update_layout(card_soil);
    lv_obj_align_to(lbl_soil_unit, lbl_soil_val, LV_ALIGN_OUT_RIGHT_BOTTOM, 3, 0);

    bar_soil_bg = lv_obj_create(card_soil);
    lv_obj_set_pos(bar_soil_bg, 13, 59);
    lv_obj_set_size(bar_soil_bg, 104, 13);
    lv_obj_set_style_bg_color(bar_soil_bg, COLOR_SOIL_TRACK, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar_soil_bg, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(bar_soil_bg, lv_color_hex(0x040910), LV_PART_MAIN);
    lv_obj_set_style_border_width(bar_soil_bg, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(bar_soil_bg, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_all(bar_soil_bg, 0, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(bar_soil_bg, true, LV_PART_MAIN);
    lv_obj_clear_flag(bar_soil_bg, LV_OBJ_FLAG_SCROLLABLE);

    bar_soil_fill = lv_obj_create(bar_soil_bg);
    lv_obj_set_pos(bar_soil_fill, 0, 0);
    lv_obj_set_size(bar_soil_fill, 43, 11); // Initial ~42% of 102px inner width
    lv_obj_set_style_bg_color(bar_soil_fill, COLOR_SOIL_FILL_TOP, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(bar_soil_fill, COLOR_SOIL_FILL, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(bar_soil_fill, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_main_stop(bar_soil_fill, 35, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_stop(bar_soil_fill, 110, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar_soil_fill, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(bar_soil_fill, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(bar_soil_fill, 5, LV_PART_MAIN);
    lv_obj_clear_flag(bar_soil_fill, LV_OBJ_FLAG_SCROLLABLE);

    // --- BOTTOM SECTION: SPECTRUM VISUALIZER BARS (Centered: x=44..404, increased height) ---
    spectrum_visualizer_obj = lv_obj_create(cactus_view);
    lv_obj_set_pos(spectrum_visualizer_obj, 44, 210);
    lv_obj_set_size(spectrum_visualizer_obj, 360, 32);
    lv_obj_set_style_bg_opa(spectrum_visualizer_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(spectrum_visualizer_obj, 0, LV_PART_MAIN);
    lv_obj_clear_flag(spectrum_visualizer_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(spectrum_visualizer_obj, spectrum_draw_cb, LV_EVENT_DRAW_MAIN, NULL);

    // ==========================================
    // VIEW 2: MALAYSIAN RETRO RADIO (Full Screen)
    // ==========================================
    radio_view = lv_obj_create(screen);
    lv_obj_set_pos(radio_view, 0, 0);
    lv_obj_set_size(radio_view, screenWidth, screenHeight);
    lv_obj_set_style_bg_color(radio_view, COLOR_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(radio_view, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(radio_view, 0, LV_PART_MAIN);
    lv_obj_clear_flag(radio_view, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(radio_view, LV_OBJ_FLAG_HIDDEN);

    // Header title ("MALAYSIA FM TUNER", centered over 380px cards at x=38..418)
    lv_obj_t *r_hdr = lv_label_create(radio_view);
    lv_label_set_text(r_hdr, "MALAYSIA FM TUNER");
    lv_obj_set_style_text_color(r_hdr, lv_color_hex(0xFF6D00), LV_PART_MAIN);
    lv_obj_set_style_text_font(r_hdr, &press_start_2p_16, LV_PART_MAIN);
    lv_obj_set_pos(r_hdr, 92, 40);

    // 1. Display Card (Frequency, Badges, Station, Genre, Equalizer)
    lv_obj_t *r_card = lv_obj_create(radio_view);
    lv_obj_set_pos(r_card, 38, 64);
    lv_obj_set_size(r_card, 380, 94);
    lv_obj_set_style_bg_color(r_card, lv_color_hex(0x0C0A08), LV_PART_MAIN);
    lv_obj_set_style_border_color(r_card, lv_color_hex(0x5C2B14), LV_PART_MAIN);
    lv_obj_set_style_border_width(r_card, 3, LV_PART_MAIN);
    lv_obj_set_style_radius(r_card, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(r_card, 6, LV_PART_MAIN);
    lv_obj_clear_flag(r_card, LV_OBJ_FLAG_SCROLLABLE);

    r_freq_val = lv_label_create(r_card);
    lv_label_set_text(r_freq_val, "89.9");
    lv_obj_set_style_text_color(r_freq_val, lv_color_hex(0x1A6E1A), LV_PART_MAIN); // Deep Vintage Phosphor Green
    lv_obj_set_style_text_font(r_freq_val, &press_start_2p_32, LV_PART_MAIN);
    lv_obj_set_pos(r_freq_val, 8, 4);

    r_freq_unit = lv_label_create(r_card);
    lv_label_set_text(r_freq_unit, "MHz");
    lv_obj_set_style_text_color(r_freq_unit, lv_color_hex(0x1A6E1A), LV_PART_MAIN);
    lv_obj_set_style_text_font(r_freq_unit, &press_start_2p_16, LV_PART_MAIN);
    lv_obj_set_pos(r_freq_unit, 142, 14);

    // Segmented Pill Equalizer Visualizer (Single fast custom-drawn widget, increased height)
    eq_visualizer_obj = lv_obj_create(r_card);
    lv_obj_set_pos(eq_visualizer_obj, 268, 38);
    lv_obj_set_size(eq_visualizer_obj, 88, 44);
    lv_obj_set_style_bg_opa(eq_visualizer_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(eq_visualizer_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(eq_visualizer_obj, 0, LV_PART_MAIN);
    lv_obj_clear_flag(eq_visualizer_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(eq_visualizer_obj, eq_draw_cb, LV_EVENT_DRAW_MAIN, NULL);

    r_status_badge = lv_label_create(r_card);
    lv_label_set_text(r_status_badge, "[TUNED]");
    lv_obj_set_style_text_color(r_status_badge, lv_color_hex(0xFF6D00), LV_PART_MAIN);
    lv_obj_set_style_text_font(r_status_badge, &lv_font_unscii_8, LV_PART_MAIN);
    lv_obj_set_style_border_color(r_status_badge, lv_color_hex(0xFF6D00), LV_PART_MAIN);
    lv_obj_set_style_border_width(r_status_badge, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(r_status_badge, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_left(r_status_badge, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_right(r_status_badge, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_top(r_status_badge, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(r_status_badge, 3, LV_PART_MAIN);
    lv_obj_set_style_bg_color(r_status_badge, lv_color_hex(0x281000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(r_status_badge, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align_to(r_status_badge, eq_visualizer_obj, LV_ALIGN_OUT_TOP_MID, 0, -8);

    r_station_name = lv_label_create(r_card);
    lv_label_set_text(r_station_name, "BFM 89.9");
    lv_obj_set_style_text_color(r_station_name, lv_color_hex(0xB82A24), LV_PART_MAIN); // Warm Reddish-Orange Burgundy
    lv_obj_set_style_text_font(r_station_name, &press_start_2p_16, LV_PART_MAIN);
    lv_obj_set_pos(r_station_name, 8, 44);

    r_genre_name = lv_label_create(r_card);
    lv_label_set_text(r_genre_name, "BUSINESS & CURRENT AFFAIRS");
    lv_obj_set_style_text_color(r_genre_name, lv_color_hex(0x1E4450), LV_PART_MAIN); // Dark Moody Petrol Teal
    lv_obj_set_style_text_font(r_genre_name, &press_start_2p_10, LV_PART_MAIN);
    lv_obj_set_pos(r_genre_name, 8, 66);

    // 2. Analog Tuning Dial Card (Single Brown Border)
    lv_obj_t *dial_box = lv_obj_create(radio_view);
    lv_obj_set_pos(dial_box, 38, 166);
    lv_obj_set_size(dial_box, 380, 72);
    lv_obj_set_style_bg_color(dial_box, lv_color_hex(0x1F1A14), LV_PART_MAIN); // Aged Sepia Bakelite
    lv_obj_set_style_border_color(dial_box, lv_color_hex(0x5C2B14), LV_PART_MAIN);
    lv_obj_set_style_border_width(dial_box, 3, LV_PART_MAIN);
    lv_obj_set_style_radius(dial_box, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(dial_box, 0, LV_PART_MAIN);
    lv_obj_clear_flag(dial_box, LV_OBJ_FLAG_SCROLLABLE);

    const char *scale_nums[] = {"88", "92", "96", "100", "104", "108"};
    const int major_x[] = {32, 94, 156, 218, 280, 342};

    for (int i = 0; i < 6; i++) {
        int x = major_x[i];
        // Major Tick (Taller Orange Marker: 3px wide, 22px high)
        lv_obj_t *tick = lv_obj_create(dial_box);
        lv_obj_set_size(tick, 3, 22);
        lv_obj_set_pos(tick, x - 1, 6);
        lv_obj_set_style_bg_color(tick, lv_color_hex(0xFF6D00), LV_PART_MAIN);
        lv_obj_set_style_border_width(tick, 0, LV_PART_MAIN);
        lv_obj_clear_flag(tick, LV_OBJ_FLAG_SCROLLABLE);

        // Pixelated Frequency Label (10% smaller: press_start_2p_14)
        lv_obj_t *lbl = lv_label_create(dial_box);
        lv_label_set_text(lbl, scale_nums[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xC2B8AC), LV_PART_MAIN); // Warm Aged Bone / Silver
        lv_obj_set_style_text_font(lbl, &press_start_2p_14, LV_PART_MAIN);
        int lbl_w = (i >= 3) ? 42 : 28;
        lv_obj_set_pos(lbl, x - (lbl_w / 2), 38);

        // Subticks (Warm Aged Bone Markers: 2px wide, 12px high)
        if (i < 5) {
            for (int s = 1; s <= 3; s++) {
                int sub_x = x + (62 * s) / 4;
                lv_obj_t *subtick = lv_obj_create(dial_box);
                lv_obj_set_size(subtick, 2, 12);
                lv_obj_set_pos(subtick, sub_x - 1, 9);
                lv_obj_set_style_bg_color(subtick, lv_color_hex(0xC2B8AC), LV_PART_MAIN); // Warm Aged Bone / Silver
                lv_obj_set_style_border_width(subtick, 0, LV_PART_MAIN);
                lv_obj_clear_flag(subtick, LV_OBJ_FLAG_SCROLLABLE);
            }
        }
    }

    // Tuning Needle (Pure Vivid Red, no blue tint, 60px high)
    r_needle = lv_obj_create(dial_box);
    lv_obj_set_size(r_needle, 4, 60);
    lv_obj_set_pos(r_needle, 30, 6);
    lv_obj_set_style_bg_color(r_needle, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_set_style_border_color(r_needle, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_set_style_border_width(r_needle, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(r_needle, 2, LV_PART_MAIN);
    lv_obj_clear_flag(r_needle, LV_OBJ_FLAG_SCROLLABLE);

#if ENABLE_RADAR_PAGE
    void build_radar_ui(lv_obj_t *screen);
    build_radar_ui(screen);
#endif

    update_radio_ui();
}

void update_radio_ui()
{
    radio_freq = constrain(radio_freq, 88.0f, 108.0f);

    char freq_buf[16];
    snprintf(freq_buf, sizeof(freq_buf), "%.1f", radio_freq);
    lv_label_set_text(r_freq_val, freq_buf);

    int unit_x = (radio_freq >= 100.0f) ? 174 : 142;
    lv_obj_set_pos(r_freq_unit, unit_x, 14);

    const int SCALE_MIN_X = 32;
    const int SCALE_MAX_X = 342;
    float pct = (radio_freq - 88.0f) / (108.0f - 88.0f);
    int needle_x = SCALE_MIN_X + (int)(pct * (SCALE_MAX_X - SCALE_MIN_X));
    lv_obj_set_pos(r_needle, needle_x - 2, 6);

    int matched_idx = -1;
    for (int i = 0; i < NUM_STATIONS; i++) {
        if (fabs(radio_freq - STATIONS[i].freq) <= 0.25f) {
            matched_idx = i;
            break;
        }
    }

    if (matched_idx != -1) {
        lv_label_set_text(r_station_name, STATIONS[matched_idx].name);
        lv_obj_set_style_text_color(r_station_name, lv_color_hex(0xB82A24), LV_PART_MAIN); // Warm Reddish-Orange Burgundy
        lv_label_set_text(r_genre_name, STATIONS[matched_idx].genre);
        lv_obj_set_style_text_color(r_genre_name, lv_color_hex(0x1E4450), LV_PART_MAIN); // Dark Moody Petrol Teal

        if (last_locked_station == matched_idx && is_station_streaming) {
            lv_label_set_text(r_status_badge, "[TUNED]");
            lv_obj_set_style_text_color(r_status_badge, lv_color_hex(0xFF6D00), LV_PART_MAIN);
            lv_obj_set_style_border_color(r_status_badge, lv_color_hex(0xFF6D00), LV_PART_MAIN);
            lv_obj_set_style_bg_color(r_status_badge, lv_color_hex(0x281000), LV_PART_MAIN);
        } else {
            lv_label_set_text(r_status_badge, "[LOCKING]");
            lv_obj_set_style_text_color(r_status_badge, lv_color_hex(0xFF8F00), LV_PART_MAIN);
            lv_obj_set_style_border_color(r_status_badge, lv_color_hex(0xFF8F00), LV_PART_MAIN);
            lv_obj_set_style_bg_color(r_status_badge, lv_color_hex(0x281500), LV_PART_MAIN);
        }

        // Arm the debouncer
        pending_station = matched_idx;
        last_tune_time = millis();
    } else {
        pending_station = -1;
        if (is_station_streaming) {
            Serial.println("[RADIO] Off-station. Stopping live stream...");
            audio.stopSong();
            is_station_streaming = false;
            last_locked_station = -1;
        }

        lv_label_set_text(r_station_name, "NO SIGNAL");
        lv_obj_set_style_text_color(r_station_name, lv_color_hex(0xB82A24), LV_PART_MAIN); // Warm Reddish-Orange Burgundy
        lv_label_set_text(r_genre_name, "ROTATE KNOB");
        lv_obj_set_style_text_color(r_genre_name, lv_color_hex(0x1E4450), LV_PART_MAIN);
        lv_label_set_text(r_status_badge, "[STATIC]");
        lv_obj_set_style_text_color(r_status_badge, lv_color_hex(0x1E4450), LV_PART_MAIN);
        lv_obj_set_style_border_color(r_status_badge, lv_color_hex(0x16343D), LV_PART_MAIN);
        lv_obj_set_style_bg_color(r_status_badge, lv_color_hex(0x0B161A), LV_PART_MAIN);
    }

    if (eq_visualizer_obj && r_status_badge) {
        lv_obj_align_to(r_status_badge, eq_visualizer_obj, LV_ALIGN_OUT_TOP_MID, 0, -8);
    }
}

#if ENABLE_RADAR_PAGE
// ==========================================
// ==========================================
// VIEW 3: RETRO FLIGHT RADAR IMPLEMENTATION
// ==========================================
static void draw_airplane(lv_draw_ctx_t *ctx, int px, int py, float track, lv_color_t color) {
    float heading_rad = (track - 90.0f) * 3.14159f / 180.0f;
    float ux = cosf(heading_rad);
    float uy = sinf(heading_rad);
    float vx = -uy;
    float vy = ux;

    lv_draw_line_dsc_t ldsc;
    lv_draw_line_dsc_init(&ldsc);
    ldsc.color = color;

    // 1. Fuselage Body (solid center line)
    ldsc.width = 3;
    lv_point_t fnose = {(lv_coord_t)(px + 10.0f * ux), (lv_coord_t)(py + 10.0f * uy)};
    lv_point_t ftail = {(lv_coord_t)(px - 8.0f * ux),  (lv_coord_t)(py - 8.0f * uy)};
    lv_draw_line(ctx, &ldsc, &fnose, &ftail);

    // 2. Swept Main Wings
    ldsc.width = 2;
    lv_point_t wroot = {(lv_coord_t)(px + 3.0f * ux), (lv_coord_t)(py + 3.0f * uy)};
    lv_point_t w_l   = {(lv_coord_t)(px - 1.0f * ux - 11.0f * vx), (lv_coord_t)(py - 1.0f * uy - 11.0f * vy)};
    lv_point_t w_r   = {(lv_coord_t)(px - 1.0f * ux + 11.0f * vx), (lv_coord_t)(py - 1.0f * uy + 11.0f * vy)};
    lv_draw_line(ctx, &ldsc, &wroot, &w_l);
    lv_draw_line(ctx, &ldsc, &wroot, &w_r);

    // Wing chord fill
    lv_point_t wmid  = {(lv_coord_t)(px + 1.0f * ux), (lv_coord_t)(py + 1.0f * uy)};
    lv_point_t wm_l  = {(lv_coord_t)(px - 2.0f * ux - 9.0f * vx), (lv_coord_t)(py - 2.0f * uy - 9.0f * vy)};
    lv_point_t wm_r  = {(lv_coord_t)(px - 2.0f * ux + 9.0f * vx), (lv_coord_t)(py - 2.0f * uy + 9.0f * vy)};
    lv_draw_line(ctx, &ldsc, &wmid, &wm_l);
    lv_draw_line(ctx, &ldsc, &wmid, &wm_r);

    // 3. Horizontal Stabilizers (Tail wings)
    lv_point_t troot = {(lv_coord_t)(px - 6.0f * ux), (lv_coord_t)(py - 6.0f * uy)};
    lv_point_t t_l   = {(lv_coord_t)(px - 8.5f * ux - 5.0f * vx), (lv_coord_t)(py - 8.5f * uy - 5.0f * vy)};
    lv_point_t t_r   = {(lv_coord_t)(px - 8.5f * ux + 5.0f * vx), (lv_coord_t)(py - 8.5f * uy + 5.0f * vy)};
    lv_draw_line(ctx, &ldsc, &troot, &t_l);
    lv_draw_line(ctx, &ldsc, &troot, &t_r);
}

static void radar_scope_draw_cb(lv_event_t *e) {
    lv_draw_ctx_t *ctx = lv_event_get_draw_ctx(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);

    int cx = coords.x1 + 98;
    int cy = coords.y1 + 102;
    float scale = 6.0f; // 15 NM ring at 90 px radius (comfortably fits within 196x204 box)

    // 1. Concentric Range Rings (5, 10, 15 NM)
    const int rings[] = {5, 10, 15};
    for (int i = 0; i < 3; i++) {
        int r = (int)(rings[i] * scale);
        for (int a = 0; a < 360; a += 15) {
            float rad1 = (float)a * 3.14159f / 180.0f;
            float rad2 = (float)(a + 8) * 3.14159f / 180.0f;
            lv_point_t p1 = {(lv_coord_t)(cx + r * cosf(rad1)), (lv_coord_t)(cy + r * sinf(rad1))};
            lv_point_t p2 = {(lv_coord_t)(cx + r * cosf(rad2)), (lv_coord_t)(cy + r * sinf(rad2))};
            
            lv_draw_line_dsc_t ldsc;
            lv_draw_line_dsc_init(&ldsc);
            ldsc.color = lv_color_hex(0x004420);
            ldsc.width = 1;
            lv_draw_line(ctx, &ldsc, &p1, &p2);
        }
    }

    // 2. Crosshair at Center (WMKP Airport)
    lv_draw_line_dsc_t ch_dsc;
    lv_draw_line_dsc_init(&ch_dsc);
    ch_dsc.color = lv_color_hex(0x006630);
    ch_dsc.width = 1;
    lv_point_t ch1 = {(lv_coord_t)(cx - 6), (lv_coord_t)cy};
    lv_point_t ch2 = {(lv_coord_t)(cx + 6), (lv_coord_t)cy};
    lv_draw_line(ctx, &ch_dsc, &ch1, &ch2);
    lv_point_t ch3 = {(lv_coord_t)(cx), (lv_coord_t)(cy - 6)};
    lv_point_t ch4 = {(lv_coord_t)(cx), (lv_coord_t)(cy + 6)};
    lv_draw_line(ctx, &ch_dsc, &ch3, &ch4);

    // 3. Penang Island Landmass Vector
    const float island_pts[14][2] = {
        {-6.1f, 10.1f}, {-4.0f, 9.8f}, {-1.6f, 10.7f}, {0.8f, 10.1f},
        {4.1f, 7.4f},   {2.3f, 4.4f},  {2.0f, 3.5f},   {1.4f, 0.8f},
        {0.8f, -1.0f},  {0.2f, -2.2f}, {-5.2f, -1.3f}, {-5.8f, 0.8f},
        {-5.2f, 3.2f},  {-6.1f, 7.5f}
    };
    lv_draw_line_dsc_t is_dsc;
    lv_draw_line_dsc_init(&is_dsc);
    is_dsc.color = lv_color_hex(0x00883A);
    is_dsc.width = 1;
    for (int i = 0; i < 14; i++) {
        int next = (i + 1) % 14;
        lv_point_t p1 = {(lv_coord_t)(cx + (int)(island_pts[i][0] * scale)), (lv_coord_t)(cy - (int)(island_pts[i][1] * scale))};
        lv_point_t p2 = {(lv_coord_t)(cx + (int)(island_pts[next][0] * scale)), (lv_coord_t)(cy - (int)(island_pts[next][1] * scale))};
        lv_draw_line(ctx, &is_dsc, &p1, &p2);
    }

    // Mainland Coast (Seberang Perai)
    int ml_x = cx + (int)(8.0f * scale);
    lv_point_t m1 = {(lv_coord_t)(ml_x + 8), (lv_coord_t)(coords.y1 + 10)};
    lv_point_t m2 = {(lv_coord_t)(ml_x + 4), (lv_coord_t)(cy - 6 * scale)};
    lv_point_t m3 = {(lv_coord_t)(ml_x + 2), (lv_coord_t)cy};
    lv_point_t m4 = {(lv_coord_t)(ml_x + 6), (lv_coord_t)(cy + 6 * scale)};
    lv_point_t m5 = {(lv_coord_t)(ml_x + 12), (lv_coord_t)(coords.y2 - 4)};
    lv_draw_line(ctx, &is_dsc, &m1, &m2);
    lv_draw_line(ctx, &is_dsc, &m2, &m3);
    lv_draw_line(ctx, &is_dsc, &m3, &m4);
    lv_draw_line(ctx, &is_dsc, &m4, &m5);

    // 4. Runway 04/22 Strip at WMKP (044° heading)
    float rwy_rad = (44.0f - 90.0f) * 3.14159f / 180.0f;
    lv_point_t rw1 = {(lv_coord_t)(cx - 10 * cosf(rwy_rad)), (lv_coord_t)(cy - 10 * sinf(rwy_rad))};
    lv_point_t rw2 = {(lv_coord_t)(cx + 10 * cosf(rwy_rad)), (lv_coord_t)(cy + 10 * sinf(rwy_rad))};
    lv_draw_line_dsc_t rwdsc;
    lv_draw_line_dsc_init(&rwdsc);
    rwdsc.color = lv_color_hex(0xE2E8F0);
    rwdsc.width = 2;
    lv_draw_line(ctx, &rwdsc, &rw1, &rw2);

    // Runway threshold labels (04 at SW, 22 at NE)
    lv_draw_label_dsc_t r_lbl_dsc;
    lv_draw_label_dsc_init(&r_lbl_dsc);
    r_lbl_dsc.color = lv_color_hex(0x00FF88);
    r_lbl_dsc.font = &lv_font_unscii_8;

    lv_area_t a_04 = {(lv_coord_t)(cx - 24), (lv_coord_t)(cy + 10), (lv_coord_t)(cx - 4), (lv_coord_t)(cy + 22)};
    lv_draw_label(ctx, &r_lbl_dsc, &a_04, "04", NULL);

    lv_area_t a_22 = {(lv_coord_t)(cx + 6), (lv_coord_t)(cy - 22), (lv_coord_t)(cx + 26), (lv_coord_t)(cy - 10)};
    lv_draw_label(ctx, &r_lbl_dsc, &a_22, "22", NULL);

    // Approach Glideslope Line for RWY 04 (from Southwest 224°)
    float app04_rad = (224.0f - 90.0f) * 3.14159f / 180.0f;
    for (int d = 2; d <= 14; d += 2) {
        lv_point_t ap1 = {(lv_coord_t)(cx + (d * scale) * cosf(app04_rad)), (lv_coord_t)(cy + (d * scale) * sinf(app04_rad))};
        lv_point_t ap2 = {(lv_coord_t)(cx + ((d + 1) * scale) * cosf(app04_rad)), (lv_coord_t)(cy + ((d + 1) * scale) * sinf(app04_rad))};
        lv_draw_line_dsc_t ap_dsc;
        lv_draw_line_dsc_init(&ap_dsc);
        ap_dsc.color = lv_color_hex(0x007730);
        ap_dsc.width = 1;
        lv_draw_line(ctx, &ap_dsc, &ap1, &ap2);
    }

    // Approach Glideslope Line for RWY 22 (from Northeast 044°)
    float app22_rad = (44.0f - 90.0f) * 3.14159f / 180.0f;
    for (int d = 2; d <= 14; d += 2) {
        lv_point_t ap1 = {(lv_coord_t)(cx + (d * scale) * cosf(app22_rad)), (lv_coord_t)(cy + (d * scale) * sinf(app22_rad))};
        lv_point_t ap2 = {(lv_coord_t)(cx + ((d + 1) * scale) * cosf(app22_rad)), (lv_coord_t)(cy + ((d + 1) * scale) * sinf(app22_rad))};
        lv_draw_line_dsc_t ap_dsc;
        lv_draw_line_dsc_init(&ap_dsc);
        ap_dsc.color = lv_color_hex(0x007730);
        ap_dsc.width = 1;
        lv_draw_line(ctx, &ap_dsc, &ap1, &ap2);
    }

    // 5. Rotating Radar Sweep Line
    float sw_rad = radar_sweep_angle * 3.14159f / 180.0f;
    lv_point_t sw1 = {(lv_coord_t)cx, (lv_coord_t)cy};
    lv_point_t sw2 = {(lv_coord_t)(cx + 90.0f * cosf(sw_rad)), (lv_coord_t)(cy + 90.0f * sinf(sw_rad))};
    lv_draw_line_dsc_t sw_dsc;
    lv_draw_line_dsc_init(&sw_dsc);
    sw_dsc.color = lv_color_hex(0x00FF66);
    sw_dsc.width = 2;
    lv_draw_line(ctx, &sw_dsc, &sw1, &sw2);

    // 6. Draw ONLY the Currently Selected Aircraft (Flightradar24 style)
    int active_count = (flight_count > 0) ? flight_count : NUM_SCHEDULE_FLIGHTS;
    const FlightInfo *src_list = (flight_count > 0) ? flight_list : PENANG_SCHEDULE;

    if (active_count > 0 && selected_flight_idx >= 0 && selected_flight_idx < active_count) {
        const FlightInfo &ac = src_list[selected_flight_idx];
        float f_dist = ac.dist;
        float f_dir = ac.dir;
        float rad = (f_dir - 90.0f) * 3.14159f / 180.0f;
        int px = cx + (int)(f_dist * scale * cosf(rad));
        int py = cy + (int)(f_dist * scale * sinf(rad));

        lv_color_t ac_col = lv_color_hex(0xFFA500); // Amber CRT accent matching active table selection

        // Draw realistic airliner symbol pointing along actual flight track
        draw_airplane(ctx, px, py, (float)ac.track, ac_col);

        // Flight callsign tag: e.g. "ZH335" or "AK5433" (NO runway number per user request)
        char tag_txt[16];
        snprintf(tag_txt, sizeof(tag_txt), "%s", ac.flight);
        int tag_len = strlen(tag_txt);
        int tag_w = tag_len * 8 + 8;
        if (tag_w < 44) tag_w = 44;
        int tag_h = 13;
        int tag_x = px + 12;
        int tag_y = py - 6;
        if (tag_x + tag_w > coords.x2 - 2) tag_x = px - tag_w - 12;
        if (tag_x < coords.x1 + 2) tag_x = coords.x1 + 2;
        if (tag_y < coords.y1 + 4) tag_y = py + 8;
        if (tag_y + tag_h > coords.y2 - 2) tag_y = coords.y2 - tag_h - 2;

        draw_border_box(ctx, tag_x, tag_y, tag_w, tag_h, lv_color_hex(0x020C05), ac_col, 1, 2);

        // Leader line connecting aircraft to callout tag
        lv_point_t lp1 = {(lv_coord_t)px, (lv_coord_t)py};
        lv_point_t lp2 = {(lv_coord_t)(tag_x < px ? tag_x + tag_w : tag_x), (lv_coord_t)(tag_y + tag_h / 2)};
        lv_draw_line_dsc_t lpdsc;
        lv_draw_line_dsc_init(&lpdsc);
        lpdsc.color = ac_col;
        lpdsc.width = 1;
        lv_draw_line(ctx, &lpdsc, &lp1, &lp2);

        lv_draw_label_dsc_t tdsc;
        lv_draw_label_dsc_init(&tdsc);
        tdsc.color = ac_col;
        tdsc.font = &lv_font_unscii_8;
        lv_area_t tarea = {(lv_coord_t)(tag_x + 3), (lv_coord_t)(tag_y + 3), (lv_coord_t)(tag_x + tag_w - 2), (lv_coord_t)(tag_y + tag_h)};
        lv_draw_label(ctx, &tdsc, &tarea, tag_txt, NULL);
    }
}

// Table tracking
static int radar_table_start_idx = 0;

// Table cell drawing callback: intercepts and enforces dark CRT background on every cell
static void table_draw_cell_cb(lv_event_t *e) {
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
    if (dsc->part == LV_PART_ITEMS) {
        uint32_t row = dsc->id / 3; // 3 columns per row: TIME, FLIGHT, FROM
        uint32_t col = dsc->id % 3;

        bool is_sel_row = (row > 0) && ((radar_table_start_idx + (int)row - 1) == selected_flight_idx);

        if (dsc->rect_dsc) {
            if (row == 0) {
                // Header row: deep phosphor tint
                dsc->rect_dsc->bg_color = lv_color_hex(0x002210);
            } else if (is_sel_row) {
                // Active selected row: CRT amber highlight
                dsc->rect_dsc->bg_color = lv_color_hex(0x281800);
            } else {
                // Alternating deep CRT phosphor-green rows
                dsc->rect_dsc->bg_color = (row % 2 == 0) ? lv_color_hex(0x020804) : lv_color_hex(0x05140A);
            }
            dsc->rect_dsc->bg_opa = LV_OPA_COVER;
            dsc->rect_dsc->border_side = LV_BORDER_SIDE_BOTTOM;
            dsc->rect_dsc->border_width = 1;
            dsc->rect_dsc->border_color = lv_color_hex(0x003318);
        }

        if (dsc->label_dsc) {
            if (row == 0) {
                dsc->label_dsc->color = lv_color_hex(0x00FF66);
            } else {
                int flight_idx = radar_table_start_idx + (int)row - 1;
                int active_count = (flight_count > 0) ? flight_count : NUM_SCHEDULE_FLIGHTS;
                const FlightInfo *src_list = (flight_count > 0) ? flight_list : PENANG_SCHEDULE;
                bool is_est = (flight_idx >= 0 && flight_idx < active_count) ? src_list[flight_idx].is_estimated : true;

                // Scheduled time (not estimated): distinct CRT Cyan color so user instantly knows!
                if (col == 0 && !is_est) {
                    dsc->label_dsc->color = lv_color_hex(0x00E5FF); // CRT Cyan
                } else if (is_sel_row) {
                    dsc->label_dsc->color = lv_color_hex(0xFFA500); // CRT Amber
                } else {
                    dsc->label_dsc->color = lv_color_hex(0x00FF66); // CRT Phosphor Green
                }
            }
        }
    }
}

void build_radar_ui(lv_obj_t *screen) {
    radar_view = lv_obj_create(screen);
    lv_obj_set_pos(radar_view, 0, 0);
    lv_obj_set_size(radar_view, screenWidth, screenHeight);
    lv_obj_set_style_bg_color(radar_view, COLOR_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(radar_view, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(radar_view, 0, LV_PART_MAIN);
    lv_obj_clear_flag(radar_view, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(radar_view, LV_OBJ_FLAG_HIDDEN);

    // --- 1. LEFT SIDE: RADAR SCOPE DISPLAY (Safely inset: x=22, y=34, w=196, h=204) ---
    radar_scope_obj = lv_obj_create(radar_view);
    lv_obj_set_pos(radar_scope_obj, 22, 34);
    lv_obj_set_size(radar_scope_obj, 196, 204);
    lv_obj_set_style_bg_color(radar_scope_obj, lv_color_hex(0x020804), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(radar_scope_obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(radar_scope_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(radar_scope_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(radar_scope_obj, 0, LV_PART_MAIN);
    lv_obj_clear_flag(radar_scope_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(radar_scope_obj, radar_scope_draw_cb, LV_EVENT_DRAW_MAIN, NULL);

    lbl_radar_count = NULL;

    // Static overlays on radar: North Arrow
    lv_obj_t *lbl_north = lv_label_create(radar_scope_obj);
    lv_label_set_text(lbl_north, "^ N");
    lv_obj_set_style_text_color(lbl_north, lv_color_hex(0x00FF66), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_north, &lv_font_unscii_8, LV_PART_MAIN);
    lv_obj_set_pos(lbl_north, 170, 6);

    // Penang Island Text
    lv_obj_t *lbl_isl = lv_label_create(radar_scope_obj);
    lv_label_set_text(lbl_isl, "PENANG\nISLAND");
    lv_obj_set_style_text_color(lbl_isl, lv_color_hex(0x00AA44), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_isl, &lv_font_unscii_8, LV_PART_MAIN);
    lv_obj_set_pos(lbl_isl, 42, 50);

    // --- 2. RIGHT SIDE: ARRIVAL BOARD (Safely inset: x=222, y=34, w=218, h=204) ---
    lv_obj_t *box_table = lv_obj_create(radar_view);
    lv_obj_set_pos(box_table, 222, 34);
    lv_obj_set_size(box_table, 218, 204);
    lv_obj_set_style_bg_color(box_table, lv_color_hex(0x020804), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(box_table, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(box_table, lv_color_hex(0x00D26A), LV_PART_MAIN);
    lv_obj_set_style_border_width(box_table, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(box_table, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_all(box_table, 0, LV_PART_MAIN);
    lv_obj_clear_flag(box_table, LV_OBJ_FLAG_SCROLLABLE);

    // Table inside right box: 3 columns (TIME, FLIGHT, FROM)
    // Sized 214x202 to fill completely to the bottom border!
    flight_table = lv_table_create(box_table);
    lv_obj_set_size(flight_table, 214, 202);
    lv_obj_set_pos(flight_table, 0, 0);
    lv_obj_set_style_bg_color(flight_table, lv_color_hex(0x020804), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(flight_table, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(flight_table, lv_color_hex(0x020804), LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(flight_table, LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_border_width(flight_table, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(flight_table, 0, LV_PART_MAIN);
    lv_obj_clear_flag(flight_table, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_font(flight_table, &lv_font_unscii_8, LV_PART_ITEMS);
    lv_obj_set_style_text_color(flight_table, lv_color_hex(0x00FF66), LV_PART_ITEMS);
    lv_obj_set_style_pad_top(flight_table, 2, LV_PART_ITEMS);
    lv_obj_set_style_pad_bottom(flight_table, 2, LV_PART_ITEMS);
    lv_obj_set_style_pad_left(flight_table, 2, LV_PART_ITEMS);
    lv_obj_set_style_pad_right(flight_table, 2, LV_PART_ITEMS);

    lv_table_set_col_cnt(flight_table, 3);
    lv_table_set_col_width(flight_table, 0, 72);   // TIME (72px: never wraps, always 1 single line!)
    lv_table_set_col_width(flight_table, 1, 104);  // FLIGHT (104px: shifted right, holds airline + flight)
    lv_table_set_col_width(flight_table, 2, 38);   // FROM (38px: snug fit for airport code, eliminates gap)
    // Sum: 72 + 104 + 38 = 214px

    // Header
    lv_table_set_cell_value(flight_table, 0, 0, "TIME");
    lv_table_set_cell_value(flight_table, 0, 1, "FLIGHT");
    lv_table_set_cell_value(flight_table, 0, 2, "FROM");

    // Intercept drawing for deep CRT green/amber/cyan colors (NO WHITE BACKGROUND)
    lv_obj_add_event_cb(flight_table, table_draw_cell_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
}

void update_radar_ui() {
    if (!flight_table) return;

    int active_count = (flight_count > 0) ? flight_count : NUM_SCHEDULE_FLIGHTS;
    const FlightInfo *src_list = (flight_count > 0) ? flight_list : PENANG_SCHEDULE;

    if (active_count == 0) {
        radar_table_start_idx = 0;
        lv_table_set_row_cnt(flight_table, 2);
        lv_table_set_cell_value(flight_table, 1, 0, "--:--");
        lv_table_set_cell_value(flight_table, 1, 1, "NO FLIGHTS");
        lv_table_set_cell_value(flight_table, 1, 2, "---");
    } else {
        // Display 9 visible rows so listview extends all the way to the bottom border!
        int max_visible = 9;
        int start_idx = 0;
        if (selected_flight_idx >= max_visible) {
            start_idx = selected_flight_idx - (max_visible - 1);
            if (start_idx + max_visible > active_count) start_idx = active_count - max_visible;
            if (start_idx < 0) start_idx = 0;
        }
        radar_table_start_idx = start_idx;

        int visible_rows = active_count > max_visible ? max_visible : active_count;
        lv_table_set_row_cnt(flight_table, visible_rows + 1);

        char hdr_flight[24];
        snprintf(hdr_flight, sizeof(hdr_flight), "FLIGHT (%d)", active_count);
        lv_table_set_cell_value(flight_table, 0, 0, "TIME");
        lv_table_set_cell_value(flight_table, 0, 1, hdr_flight);
        lv_table_set_cell_value(flight_table, 0, 2, "FROM");

        char buf[32];
        char f_buf[32];
        for (int slot = 0; slot < visible_rows; slot++) {
            int i = start_idx + slot;
            int r = slot + 1;
            bool is_sel = (i == selected_flight_idx);
            
            // Col 0: TIME with '>' indicator if active - always on one line!
            if (is_sel) {
                snprintf(buf, sizeof(buf), ">%s", src_list[i].time_str);
            } else {
                snprintf(buf, sizeof(buf), " %s", src_list[i].time_str);
            }
            lv_table_set_cell_value(flight_table, r, 0, buf);

            // Col 1: FLIGHT (Airline name on top, Flight number below)
            snprintf(f_buf, sizeof(f_buf), "%s\n%s", src_list[i].airline, src_list[i].flight);
            lv_table_set_cell_value(flight_table, r, 1, f_buf);

            // Col 2: FROM
            lv_table_set_cell_value(flight_table, r, 2, src_list[i].origin);
        }
    }

    if (radar_scope_obj) {
        lv_obj_invalidate(radar_scope_obj);
    }
}

// Helper to derive readable airline brand from flight number and ICAO code
static void get_airline_name(const char *flight, const char *icao, char *out, size_t max_len) {
    if (strncmp(flight, "AK", 2) == 0 || strncmp(flight, "QZ", 2) == 0 || strncmp(flight, "FD", 2) == 0 ||
        strcmp(icao, "AXM") == 0 || strcmp(icao, "AWQ") == 0 || strcmp(icao, "AIQ") == 0) {
        strncpy(out, "AirAsia", max_len - 1);
    } else if (strncmp(flight, "OD", 2) == 0 || strncmp(flight, "ID", 2) == 0 || strncmp(flight, "SL", 2) == 0 ||
               strcmp(icao, "BTK") == 0 || strcmp(icao, "MXD") == 0 || strcmp(icao, "TLM") == 0) {
        strncpy(out, "Batik Air", max_len - 1);
    } else if (strncmp(flight, "MH", 2) == 0 || strcmp(icao, "MAS") == 0) {
        strncpy(out, "Malaysia", max_len - 1);
    } else if (strncmp(flight, "FY", 2) == 0 || strcmp(icao, "FFM") == 0) {
        strncpy(out, "Firefly", max_len - 1);
    } else if (strncmp(flight, "SQ", 2) == 0 || strcmp(icao, "SIA") == 0) {
        strncpy(out, "Singapore", max_len - 1);
    } else if (strncmp(flight, "TR", 2) == 0 || strcmp(icao, "TGW") == 0) {
        strncpy(out, "Scoot", max_len - 1);
    } else if (strncmp(flight, "CI", 2) == 0 || strcmp(icao, "CAL") == 0) {
        strncpy(out, "China Air", max_len - 1);
    } else if (strncmp(flight, "BR", 2) == 0 || strcmp(icao, "EVA") == 0) {
        strncpy(out, "EVA Air", max_len - 1);
    } else if (strncmp(flight, "TG", 2) == 0 || strcmp(icao, "THA") == 0) {
        strncpy(out, "Thai Air", max_len - 1);
    } else if (strncmp(flight, "CX", 2) == 0 || strcmp(icao, "CPA") == 0) {
        strncpy(out, "Cathay", max_len - 1);
    } else if (strncmp(flight, "ZH", 2) == 0 || strcmp(icao, "CSZ") == 0) {
        strncpy(out, "Shenzhen", max_len - 1);
    } else if (strncmp(flight, "CZ", 2) == 0 || strcmp(icao, "CSN") == 0) {
        strncpy(out, "China South", max_len - 1);
    } else if (strncmp(flight, "MU", 2) == 0 || strcmp(icao, "CES") == 0) {
        strncpy(out, "China East", max_len - 1);
    } else if (strncmp(flight, "QR", 2) == 0 || strcmp(icao, "QTR") == 0) {
        strncpy(out, "Qatar", max_len - 1);
    } else if (strncmp(flight, "EK", 2) == 0 || strcmp(icao, "UAE") == 0) {
        strncpy(out, "Emirates", max_len - 1);
    } else if (strlen(icao) >= 3) {
        strncpy(out, icao, max_len - 1);
    } else {
        strncpy(out, "Airline", max_len - 1);
    }
    out[max_len - 1] = '\0';
}

// Live ADS-B Polling routine (executed cooperatively when on VIEW_RADAR)
void fetch_flight_data() {
    if (!WiFi.isConnected()) return;

    Serial.printf("[RADAR] Fetching live flights around Penang... Free heap: %u\n", ESP.getFreeHeap());

    HTTPClient http;
    http.setTimeout(3500);
    // Plain HTTP endpoint for real-time live flight radar data around Penang (35NM radius)
    // Bounds: 5.8N, 4.8S, 99.8W, 100.8E centered on WMKP
    if (http.begin("http://data-cloud.flightradar24.com/zones/fcgi/feed.js?bounds=5.8,4.8,99.8,100.8")) {
        int code = http.GET();
        Serial.printf("[RADAR] Flightradar feed response: %d\n", code);
        if (code == 200) {
            String payload = http.getString();
            Serial.printf("[RADAR] Payload bytes: %d\n", payload.length());
            DynamicJsonDocument *doc = new DynamicJsonDocument(6144);
            if (doc) {
                DeserializationError err = deserializeJson(*doc, payload);
                if (!err) {
                    FlightInfo temp_list[MAX_FLIGHTS];
                    int count = 0;
                    time_t now = time(nullptr);

                    // Iterate over flights in the feed
                    for (JsonPair kv : doc->as<JsonObject>()) {
                        if (count >= MAX_FLIGHTS) break;
                        const char *key = kv.key().c_str();
                        if (strcmp(key, "full_count") == 0 || strcmp(key, "version") == 0) continue;

                        JsonArray arr = kv.value().as<JsonArray>();
                        if (arr.isNull() || arr.size() < 14) continue;

                        float lat = arr[1] | 0.0f;
                        float lon = arr[2] | 0.0f;
                        int trk = arr[3] | 0;
                        int alt = arr[4] | 0;
                        int spd = arr[5] | 0;
                        const char *orig = arr[11] | "";
                        const char *dest = arr[12] | "";
                        const char *fnum = arr[13] | arr[16] | arr[0] | "";
                        int on_ground = arr[14] | 0;
                        int vrate = arr[15] | 0;

                        // Calculate distance & bearing to Penang Airport (WMKP: 5.29714, 100.2768)
                        float d_lat = (lat - 5.29714f) * 60.0f;
                        float d_lon = (lon - 100.2768f) * 60.0f * 0.99577f;
                        float dist = sqrtf(d_lat * d_lat + d_lon * d_lon);
                        float dir = atan2f(d_lon, d_lat) * 57.29578f;
                        if (dir < 0) dir += 360.0f;

                        // Filter: Must be within 35 NM, and either heading to PEN or local traffic
                        bool is_pen_arrival = (strcmp(dest, "PEN") == 0);
                        if (!is_pen_arrival && (dist > 25.0f || alt > 14000)) continue;

                        // Commercial Flight Number: e.g. "OD2104", "AK5433", "CI731"
                        if (strlen(fnum) < 2) continue;
                        strncpy(temp_list[count].flight, fnum, sizeof(temp_list[0].flight) - 1);
                        temp_list[count].flight[sizeof(temp_list[0].flight) - 1] = '\0';

                        // Airline name: e.g. "AirAsia", "Batik Air", "China Air", "Malaysia", "Firefly"
                        const char *icao = (arr.size() > 18) ? (arr[18] | "") : "";
                        get_airline_name(fnum, icao, temp_list[count].airline, sizeof(temp_list[0].airline));

                        // Origin Airport: e.g. "KUL", "TPE", "SIN"
                        if (strlen(orig) > 0) {
                            strncpy(temp_list[count].origin, orig, sizeof(temp_list[0].origin) - 1);
                        } else {
                            strcpy(temp_list[count].origin, "---");
                        }
                        temp_list[count].origin[sizeof(temp_list[0].origin) - 1] = '\0';

                        temp_list[count].dist = dist;
                        temp_list[count].dir = dir;
                        temp_list[count].track = trk;
                        temp_list[count].speed = spd;
                        temp_list[count].altitude = alt;
                        temp_list[count].rate = vrate;

                        // Runway Assignment: heading 044° approaches from SW (RWY 04), heading 224° from NE (RWY 22)
                        if (trk >= 130 && trk <= 310) {
                            temp_list[count].runway = 22;
                        } else {
                            temp_list[count].runway = 4;
                        }

                        // ETA / Status
                        if (on_ground) {
                            strcpy(temp_list[count].time_str, "LANDED");
                            temp_list[count].is_estimated = true;
                        } else {
                            float v = (spd > 50) ? (float)spd : 180.0f;
                            int mins = (int)((dist / v) * 60.0f + 0.5f);
                            if (now > 100000) {
                                time_t eta = now + mins * 60;
                                struct tm *ti = localtime(&eta);
                                if (ti) {
                                    snprintf(temp_list[count].time_str, sizeof(temp_list[count].time_str),
                                             "%02d:%02d%c",
                                             (ti->tm_hour % 12 == 0 ? 12 : ti->tm_hour % 12),
                                             ti->tm_min,
                                             ti->tm_hour >= 12 ? 'P' : 'A');
                                } else {
                                    snprintf(temp_list[count].time_str, sizeof(temp_list[count].time_str), "+%dm", mins);
                                }
                            } else {
                                snprintf(temp_list[count].time_str, sizeof(temp_list[count].time_str), "+%dm", mins);
                            }
                            temp_list[count].is_estimated = true;
                        }

                        count++;
                    }

                    // 2. Append the full Penang arrival schedule so there are ALWAYS at least 15-16 flights on the board!
                    int total_count = count;
                    for (int s = 0; s < NUM_SCHEDULE_FLIGHTS && total_count < MAX_FLIGHTS; s++) {
                        bool dup = false;
                        for (int c = 0; c < count; c++) {
                            if (strcmp(temp_list[c].flight, PENANG_SCHEDULE[s].flight) == 0) {
                                dup = true;
                                break;
                            }
                        }
                        if (!dup) {
                            temp_list[total_count] = PENANG_SCHEDULE[s];
                            if (now > 100000) {
                                time_t st = now + (s * 8 + 6) * 60;
                                struct tm *ti = localtime(&st);
                                if (ti) {
                                    snprintf(temp_list[total_count].time_str, sizeof(temp_list[total_count].time_str),
                                             "%02d:%02d%c",
                                             (ti->tm_hour % 12 == 0 ? 12 : ti->tm_hour % 12),
                                             ti->tm_min,
                                             ti->tm_hour >= 12 ? 'P' : 'A');
                                }
                            }
                            total_count++;
                        }
                    }

                    if (total_count > 0) {
                        memcpy(flight_list, temp_list, sizeof(FlightInfo) * total_count);
                        flight_count = total_count;
                        live_adsb_received = true;
                        Serial.printf("[RADAR] Flights updated: %d active (%d live airborne, %d scheduled)\n", 
                                      total_count, count, total_count - count);
                        update_radar_ui();
                    }
                }
                delete doc;
            }
        }
        http.end();
    }
}
#endif // #if ENABLE_RADAR_PAGE

void setup()
{
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0);

    // --- DISPLAY INIT ---
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);

    if (!gfx->begin()) {
        Serial.println("gfx->begin() failed!");
    }
    gfx->fillScreen(RGB565_BLACK);
    gfx->flush();

    // --- LVGL INIT ---
    lv_init();

    // Optimized buffer size (16 lines instead of 40) saves ~23KB of precious internal DMA heap for SSL networking
    size_t buffer_size = screenWidth * 16;
    lv_color_t *draw_buf = (lv_color_t *)heap_caps_malloc(
        buffer_size * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    if (!draw_buf) {
        draw_buf = (lv_color_t *)malloc(buffer_size * sizeof(lv_color_t));
    }

    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, draw_buf, NULL, buffer_size);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    // --- TOUCH INIT ---
    init_touch();
    if (touch_available) {
        static lv_indev_drv_t indev_drv;
        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = touchpad_read;
        lv_indev_drv_register(&indev_drv);
    }

    // --- BUILD UI ---
#if ENABLE_RADAR_PAGE
    memcpy(flight_list, PENANG_SCHEDULE, sizeof(PENANG_SCHEDULE));
    flight_count = NUM_SCHEDULE_FLIGHTS;
#endif
    build_ui();

    // --- AUDIO INIT ---
    init_audio();

    // --- WI-FI INIT ---
    if (strcmp(WIFI_SSID, "YOUR_WIFI_SSID") == 0) {
        Serial.println("============================================================");
        Serial.println("[!] NOTICE: Default Wi-Fi credentials detected!");
        Serial.println("[!] Please open 'src/config.h' and enter your Wi-Fi details.");
        Serial.println("============================================================");
    }
    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
            Serial.printf("[WIFI] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        } else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
            Serial.println("[WIFI] Disconnected, reconnecting...");
            WiFi.reconnect();
        }
    });
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.setSleep(false); // Disable Wi-Fi modem sleep to ensure smooth continuous audio streaming
    Serial.printf("[WIFI] Connecting to %s...\n", WIFI_SSID);
    configTime(8 * 3600, 0, "pool.ntp.org", "time.google.com");

#if ENABLE_RADAR_PAGE
    // ADS-B API fetching is dormant during UI design; will only run on demand when viewing radar
#endif

    // --- DHT11 INIT ---
    dht.begin();
    Serial.println("[DHT11] Sensor initialized on GPIO 17");

    // --- ROTARY ENCODER INIT ---
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    uint8_t initial_pinstate = (digitalRead(ENCODER_CLK) << 1) | digitalRead(ENCODER_DT);
    encoder_state = (initial_pinstate == 0x03) ? R_START : initial_pinstate;
    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handle_encoder, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_DT),  handle_encoder, CHANGE);
    Serial.println("[ENCODER] Initialized with Gray Code state machine on CLK=IO6 & DT=IO7");

    // --- MODE BUTTON INIT ---
    pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
    Serial.println("[MODE] Button initialized on IO18 (Active LOW)");

    // --- SOIL MOISTURE SENSOR INIT ---
    pinMode(SOIL_PIN, INPUT);
    analogReadResolution(12);
    Serial.println("[SOIL] Moisture sensor initialized on IO16 (ADC)");
}

// Sensor values & alert state
static float temp = 0.0;
static float humid = 0.0;
static unsigned long last_update = 0;
static unsigned long last_soil_update = 0;
static unsigned long last_voice_alert = 0;
static int greeting_toggle = 0;

static void fetch_weather_data() {
    if (!WiFi.isConnected()) return;

    HTTPClient http;
    http.begin("http://api.open-meteo.com/v1/forecast?latitude=5.4141&longitude=100.3288&current=temperature_2m,is_day,weather_code");
    http.setTimeout(2500);
    int http_code = http.GET();
    if (http_code == HTTP_CODE_OK) {
        String payload = http.getString();
        
        // Find the "current":{ object to skip "current_units" (which has "temperature_2m":"°C")
        int cur_idx = payload.indexOf("\"current\":{");
        if (cur_idx < 0) cur_idx = payload.indexOf("\"current\":");
        if (cur_idx < 0) cur_idx = 0;

        // 1. Temperature
        int t_idx = payload.indexOf("\"temperature_2m\":", cur_idx);
        float temp_val = 27.0f;
        if (t_idx > 0) {
            temp_val = payload.substring(t_idx + 17).toFloat();
        }

        // 2. Day/Night indicator (0 = night, 1 = day)
        int d_idx = payload.indexOf("\"is_day\":", cur_idx);
        int is_day = 0;
        if (d_idx > 0) {
            is_day = payload.substring(d_idx + 9).toInt();
        }

        // 3. WMO Weather Code
        int c_idx = payload.indexOf("\"weather_code\":", cur_idx);
        int w_code = 0;
        if (c_idx > 0) {
            w_code = payload.substring(c_idx + 15).toInt();
        }

        const char *cond_str = "Clear";
        if (w_code == 0) {
            if (is_day) {
                current_weather_type = WEATHER_CLEAR_DAY;
                cond_str = "Sunny";
            } else {
                current_weather_type = WEATHER_CLEAR_NIGHT;
                cond_str = "Clear Night";
            }
        } else if (w_code == 1 || w_code == 2) {
            if (is_day) {
                current_weather_type = WEATHER_PARTLY_CLOUDY_DAY;
                cond_str = "Partly Cloudy";
            } else {
                current_weather_type = WEATHER_PARTLY_CLOUDY_NIGHT;
                cond_str = "Partly Cloudy";
            }
        } else if (w_code == 3) {
            current_weather_type = WEATHER_CLOUDY;
            cond_str = "Overcast";
        } else if (w_code >= 45 && w_code <= 48) {
            current_weather_type = WEATHER_CLOUDY;
            cond_str = "Foggy";
        } else if (w_code >= 51 && w_code <= 67) {
            current_weather_type = WEATHER_RAIN;
            cond_str = "Rainy";
        } else if (w_code >= 80 && w_code <= 82) {
            current_weather_type = WEATHER_RAIN;
            cond_str = "Showers";
        } else if (w_code >= 95) {
            current_weather_type = WEATHER_THUNDERSTORM;
            cond_str = "Thunderstorm";
        } else {
            if (is_day) {
                current_weather_type = WEATHER_CLEAR_DAY;
                cond_str = "Clear";
            } else {
                current_weather_type = WEATHER_CLEAR_NIGHT;
                cond_str = "Clear Night";
            }
        }

        if (lbl_weather_temp) {
            char t_str[16];
            snprintf(t_str, sizeof(t_str), "%d°C", (int)round(temp_val));
            lv_label_set_text(lbl_weather_temp, t_str);
            lv_obj_set_style_text_color(lbl_weather_temp, lv_color_hex(0xFF6D00), LV_PART_MAIN);
        }
        if (lbl_weather_cond) {
            lv_label_set_text(lbl_weather_cond, cond_str);
        }
        if (weather_icon_obj) {
            lv_obj_invalidate(weather_icon_obj);
        }
        Serial.printf("[WEATHER] Live update -> Temp: %.1fC, is_day: %d, code: %d, cond: %s\n", 
                      temp_val, is_day, w_code, cond_str);
    }
    http.end();
}

void loop()
{
    uint32_t now = millis();

    // High-priority audio streaming pump before LVGL
    if (current_view == VIEW_RADIO && is_station_streaming) {
        audio.loop();
    }

    // Accurate real-time tick increment for LVGL
    static uint32_t last_tick = millis();
    uint32_t elapsed = now - last_tick;
    if (elapsed > 0) {
        lv_tick_inc(elapsed);
        last_tick = now;
    }

    // Rate-limited LVGL processing (~60 FPS) to ensure buttery smooth animation
    // without starving CPU from audio or triggering microsecond redraw jitter
    static uint32_t last_lvgl_call = 0;
    if (now - last_lvgl_call >= 16) {
        last_lvgl_call = now;
        lv_timer_handler();
    }

    // --- SERIAL COMMAND HANDLER ---
    if (Serial.available()) {
        char cmd = Serial.read();
        if (cmd == 'r') {
            Serial.println("[CMD] Remote trigger: switch_tab(VIEW_RADAR)");
            switch_tab(VIEW_RADAR);
        } else if (cmd == 'c') {
            Serial.println("[CMD] Remote trigger: switch_tab(VIEW_CACTUS)");
            switch_tab(VIEW_CACTUS);
        } else if (cmd == 'f') {
            Serial.println("[CMD] Remote trigger: switch_tab(VIEW_RADIO)");
            switch_tab(VIEW_RADIO);
        }
    }

    // High-priority audio streaming pump after LVGL
    if (current_view == VIEW_RADIO && is_station_streaming) {
        audio.loop();
    }

    // --- AUDIO LOOP (Live Streaming or Constant Static) ---
    if (current_view == VIEW_RADIO) {
        // If knob stopped on a station for > 350ms, lock in and connect!
        if (pending_station != -1 && pending_station != last_locked_station) {
            if (millis() - last_tune_time > 350) {
                last_locked_station = pending_station;
                is_station_streaming = true;

                lv_label_set_text(r_status_badge, "[TUNED]");
                lv_obj_set_style_text_color(r_status_badge, lv_color_hex(0xFF6D00), LV_PART_MAIN);
                lv_obj_set_style_border_color(r_status_badge, lv_color_hex(0xFF6D00), LV_PART_MAIN);
                lv_obj_set_style_bg_color(r_status_badge, lv_color_hex(0x281000), LV_PART_MAIN);
                lv_obj_set_style_text_color(r_freq_val, lv_color_hex(0x1A6E1A), LV_PART_MAIN);
                if (eq_visualizer_obj && r_status_badge) {
                    lv_obj_align_to(r_status_badge, eq_visualizer_obj, LV_ALIGN_OUT_TOP_MID, 0, -8);
                }

                Serial.printf("[RADIO] User settled on %s. Connecting to stream: %s\n", 
                              STATIONS[pending_station].name, STATIONS[pending_station].url);
                audio.stopSong();
                audio.connecttohost(STATIONS[pending_station].url);
            }
        }

        if (last_locked_station != -1 && is_station_streaming) {
            audio.loop();

            // Auto-reconnect watchdog if stream dropped unexpectedly
            static unsigned long last_stream_check = 0;
            if (millis() - last_stream_check > 3000) {
                last_stream_check = millis();
                if (!audio.isRunning() && WiFi.isConnected()) {
                    Serial.printf("[RADIO] Stream disconnected unexpectedly, auto-reconnecting to %s...\n", 
                                  STATIONS[last_locked_station].name);
                    audio.connecttohost(STATIONS[last_locked_station].url);
                }
            }
        } else {
            // Constant "sshshshshhh" FM carrier hiss while untuned/tuning!
            output_continuous_static(0.08);
        }

        // Segmented pill equalizer animation (fast single-widget invalidation)
        static unsigned long last_eq_anim = 0;
        if (millis() - last_eq_anim > 85) {
            last_eq_anim = millis();
            bool is_tuned = (pending_station != -1);
            for (int c = 0; c < EQ_COLS; c++) {
                if (is_tuned) {
                    int base = (c >= 2 && c <= 5) ? 4 : 2;
                    eq_levels[c] = constrain(base + random(-1, 3), 1, EQ_SEGS);
                } else {
                    eq_levels[c] = (random(100) > 75) ? 1 : 0;
                }
            }
            if (eq_visualizer_obj) {
                lv_obj_invalidate(eq_visualizer_obj);
            }
        }
    }

    // --- SOIL MOISTURE CHECK (Every 1s, ONLY on CACTUS view) ---
    if (current_view == VIEW_CACTUS && (millis() - last_soil_update > 1000)) {
        last_soil_update = millis();
        int soilRaw = analogRead(SOIL_PIN);
        int soilMv = analogReadMilliVolts(SOIL_PIN);

        const int SOIL_DRY = 2970;
        const int SOIL_WET = 1350;
        int moisturePct = map(soilRaw, SOIL_DRY, SOIL_WET, 0, 100);
        moisturePct = constrain(moisturePct, 0, 100);

        // Update UI only when value changes to prevent unnecessary redraws
        if (moisturePct != last_disp_moisture) {
            last_disp_moisture = moisturePct;
            char soil_buf[16];
            snprintf(soil_buf, sizeof(soil_buf), "%d", moisturePct);
            if (lbl_soil_val) {
                lv_label_set_text(lbl_soil_val, soil_buf);
            }
            if (bar_soil_fill) {
                int fill_w = (moisturePct * 102) / 100;
                if (fill_w < 6) fill_w = 6;
                lv_obj_set_width(bar_soil_fill, fill_w);
            }
        }

        const char *status = "DRY";
        if (moisturePct >= 65) {
            status = "WET";
        } else if (moisturePct >= 20) {
            status = "MOIST";
        }

        Serial.printf("[SOIL] Moisture: %d%% (%s) | Raw: %4d | Voltage: %4d mV\n", 
                      moisturePct, status, soilRaw, soilMv);

        // Voice Alert: Soil Dry (DISABLED)
        #if 0
        if (current_view == VIEW_CACTUS) {
            if (moisturePct < 20 && (millis() - last_voice_alert > 60000)) {
                last_voice_alert = millis();
                Serial.println("[VOICE ALERT] Soil is parched! Playing alert...");
                play_voice_clip(clip_water_data, clip_water_samples);
            }
        }
        #endif
    }

    // --- TIME & DAY/NIGHT UPDATE (Every 1s, ONLY on CACTUS view) ---
    static unsigned long last_clock_tick = 0;
    if (current_view == VIEW_CACTUS && (millis() - last_clock_tick > 1000)) {
        last_clock_tick = millis();
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 10)) {
            char t_buf[16];
            char d_buf[32];
            strftime(t_buf, sizeof(t_buf), "%H:%M", &timeinfo);
            strftime(d_buf, sizeof(d_buf), "%a, %d %b", &timeinfo);
            // Only update LVGL label if time string or date string has changed
            if (lbl_clock_time && strcmp(t_buf, last_clock_t_str) != 0) {
                strcpy(last_clock_t_str, t_buf);
                lv_label_set_text(lbl_clock_time, t_buf);
            }
            if (lbl_clock_date && strcmp(d_buf, last_clock_d_str) != 0) {
                strcpy(last_clock_d_str, d_buf);
                lv_label_set_text(lbl_clock_date, d_buf);
            }

            // Day / Night state sync based on RTC clock if not updated by API
            bool is_night_time = (timeinfo.tm_hour >= 19 || timeinfo.tm_hour < 7);
            if (is_night_time && (current_weather_type == WEATHER_CLEAR_DAY || current_weather_type == WEATHER_PARTLY_CLOUDY_DAY)) {
                current_weather_type = (current_weather_type == WEATHER_CLEAR_DAY) ? WEATHER_CLEAR_NIGHT : WEATHER_PARTLY_CLOUDY_NIGHT;
                if (weather_icon_obj) lv_obj_invalidate(weather_icon_obj);
            } else if (!is_night_time && (current_weather_type == WEATHER_CLEAR_NIGHT || current_weather_type == WEATHER_PARTLY_CLOUDY_NIGHT)) {
                current_weather_type = (current_weather_type == WEATHER_CLEAR_NIGHT) ? WEATHER_CLEAR_DAY : WEATHER_PARTLY_CLOUDY_DAY;
                if (weather_icon_obj) lv_obj_invalidate(weather_icon_obj);
            }
        }
    }

    // --- LIVE WEATHER FETCH (Every 10 minutes when on CACTUS view and not streaming) ---
    static unsigned long last_weather_fetch = 0;
    static bool initial_weather_fetched = false;
    if (current_view == VIEW_CACTUS && WiFi.isConnected() && !audio.isRunning()) {
        if (!initial_weather_fetched || (millis() - last_weather_fetch > 600000)) {
            if (millis() > 3000) {
                last_weather_fetch = millis();
                initial_weather_fetched = true;
                fetch_weather_data();
            }
        }
    }

    // --- BOTTOM SPECTRUM BAR ANIMATION (Every 100ms, ONLY on CACTUS view) ---
    static unsigned long last_spec_anim = 0;
    if (current_view == VIEW_CACTUS && (millis() - last_spec_anim > 100)) {
        last_spec_anim = millis();
        static float spec_t = 0;
        spec_t += 0.25f;
        for (int i = 0; i < SPEC_COLS; i++) {
            float wave = sinf(spec_t + i * 0.38f) * 6.5f + sinf(spec_t * 1.6f + i * 0.65f) * 5.0f;
            int h = 11 + (int)wave;
            if ((i >= 5 && i <= 12) || (i >= 18 && i <= 24) || (i >= 32 && i <= 38)) {
                h += 6;
            }
            spec_heights[i] = constrain(h, 4, 30);
        }
        if (spectrum_visualizer_obj) {
            lv_obj_invalidate(spectrum_visualizer_obj);
        }
    }

    // --- ENCODER ROTATION CHECK (Tuning Radio Dial) ---
    static long last_encoder_pos = 0;
    if (encoder_changed) {
        noInterrupts();
        long current_pos = encoder_pos;
        encoder_changed = false;
        interrupts();
        long delta = current_pos - last_encoder_pos;
        last_encoder_pos = current_pos;

        if (current_view == VIEW_RADIO && delta != 0) {
            float freq_step = 0.2f * (float)delta;
            radio_freq += freq_step;
            radio_freq = constrain(radio_freq, 88.0f, 108.0f);
            update_radio_ui();
            Serial.printf("[RADIO] Tuned to %.1f MHz (step=%+ld)\n", radio_freq, delta);
#if ENABLE_RADAR_PAGE
        } else if (current_view == VIEW_RADAR && delta != 0) {
            int active_cnt = (flight_count > 0) ? flight_count : NUM_SCHEDULE_FLIGHTS;
            if (active_cnt > 0) {
                int prev_idx = selected_flight_idx;
                selected_flight_idx = constrain(selected_flight_idx + (int)delta, 0, active_cnt - 1);
                if (selected_flight_idx != prev_idx) {
                    update_radar_ui();
                    if (radar_scope_obj) lv_obj_invalidate(radar_scope_obj);
                    Serial.printf("[RADAR] Selected flight: %d/%d\n", selected_flight_idx + 1, active_cnt);
                }
            }
#endif
        }
    }

    // --- ENCODER BUTTON CHECK ---
    static int last_sw_state = HIGH;
    static unsigned long last_sw_debounce = 0;
    int sw_state = digitalRead(ENCODER_SW);
    if (sw_state != last_sw_state) {
        if (millis() - last_sw_debounce > 50) {
            last_sw_debounce = millis();
            last_sw_state = sw_state;
            if (sw_state == LOW) {
                Serial.printf("[ENCODER] Switch pressed! Current view: %d\n", (int)current_view);
                if (current_view == VIEW_CACTUS) {
                    switch_tab(VIEW_RADIO);
#if ENABLE_RADAR_PAGE
                } else if (current_view == VIEW_RADIO) {
                    switch_tab(VIEW_RADAR);
#endif
                } else {
                    switch_tab(VIEW_CACTUS);
                }
            }
        }
    }

    // --- MODE TOGGLE BUTTON CHECK (IO18) ---
    static int last_mode_state = HIGH;
    static unsigned long last_mode_debounce = 0;
    int mode_state = digitalRead(MODE_BUTTON_PIN);
    if (mode_state != last_mode_state) {
        if (millis() - last_mode_debounce > 50) {
            last_mode_debounce = millis();
            last_mode_state = mode_state;
            if (mode_state == LOW) {
                Serial.printf("[MODE] Button pressed! Current view: %d\n", (int)current_view);
                if (current_view == VIEW_CACTUS) {
                    switch_tab(VIEW_RADIO);
#if ENABLE_RADAR_PAGE
                } else if (current_view == VIEW_RADIO) {
                    switch_tab(VIEW_RADAR);
#endif
                } else {
                    switch_tab(VIEW_CACTUS);
                }
            }
        }
    }

#if ENABLE_RADAR_PAGE
    // --- RADAR DISPLAY ANIMATION & REFRESH ---
    static unsigned long last_radar_sweep = 0;
    if (current_view == VIEW_RADAR && (millis() - last_radar_sweep > 40)) {
        last_radar_sweep = millis();
        radar_sweep_angle = fmodf(radar_sweep_angle + 2.5f, 360.0f);
        if (radar_scope_obj) {
            lv_obj_invalidate(radar_scope_obj);
        }
    }

    static unsigned long last_radar_ui_update = 0;
    if (current_view == VIEW_RADAR && (millis() - last_radar_ui_update > 2000)) {
        last_radar_ui_update = millis();
        update_radar_ui();
    }

    // Live ADS-B polling (Every 30 seconds, strictly on VIEW_RADAR)
    if (current_view == VIEW_RADAR && (millis() - last_flight_fetch > 30000)) {
        last_flight_fetch = millis();
        fetch_flight_data();
    }
#endif

    // Read sensor every 2 seconds (ONLY active on CACTUS view to prevent blocking interrupts during radio)
    if (current_view == VIEW_CACTUS && (millis() - last_update > 2000)) {
        last_update = millis();

        float t = dht.readTemperature();
        float h = dht.readHumidity();

        if (isnan(t) || isnan(h)) {
            Serial.printf("[DHT11] Sensor read failed/busy (Raw Pin 17: %d)\n", digitalRead(DHTPIN));
            if (temp == 0.0 && humid == 0.0) {
                if (lbl_humid_val) lv_label_set_text(lbl_humid_val, "--");
                if (lbl_temp_val) lv_label_set_text(lbl_temp_val, "--");
            }
        } else {
            temp = t;
            humid = h;
            Serial.printf("[DHT11] Reading -> Temperature: %.1f °C, Humidity: %.1f %%\n", temp, humid);

            if (fabs(temp - last_disp_temp) >= 0.1f) {
                last_disp_temp = temp;
                char buf[32];
                snprintf(buf, sizeof(buf), "%.1f", temp);
                if (lbl_temp_val) lv_label_set_text(lbl_temp_val, buf);
            }

            if (fabs(humid - last_disp_humid) >= 1.0f) {
                last_disp_humid = humid;
                char buf[32];
                snprintf(buf, sizeof(buf), "%.0f", humid);
                if (lbl_humid_val) lv_label_set_text(lbl_humid_val, buf);
            }

            // Voice Alert: High Temp (DISABLED)
            #if 0
            if (current_view == VIEW_CACTUS) {
                if (temp > 34.0 && (millis() - last_voice_alert > 60000)) {
                    last_voice_alert = millis();
                    Serial.println("[VOICE ALERT] High temperature detected! Playing alert...");
                    play_voice_clip(clip_temp_data, clip_temp_samples);
                }
            }
            #endif
        }
    }
}
