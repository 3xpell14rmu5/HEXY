#ifndef HEXIOS_H
#define HEXIOS_H

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <esp_ota_ops.h>

// ── Button GPIO pins ──────────────────────────────────────────────────────────
#define HEXY_BTN_LEFT    4
#define HEXY_BTN_RIGHT   5
#define HEXY_BTN_UP      6
#define HEXY_BTN_DOWN   15
#define HEXY_BTN_SELECT 16
#define HEXY_BTN_BACK   17

// ── Display pins (ST7789V2 240x280) ───────────────────────────────────────────
#define HEXY_TFT_SCLK  12
#define HEXY_TFT_MOSI  11
#define HEXY_TFT_MISO  13
#define HEXY_TFT_DC     9
#define HEXY_TFT_CS    10
#define HEXY_TFT_RST    8
#define HEXY_TFT_BL     7

// ── hexiOS colour palette ─────────────────────────────────────────────────────
#define HEXY_COLOR_BG        0x0000   // Black
#define HEXY_COLOR_PRIMARY   0x07FF   // Cyan
#define HEXY_COLOR_SECONDARY 0x243C   // Blue
#define HEXY_COLOR_ACCENT    0x07E0   // Green
#define HEXY_COLOR_WARNING   0xFD20   // Orange
#define HEXY_COLOR_ERROR     0xF800   // Red
#define HEXY_COLOR_TEXT      0xFFFF   // White
#define HEXY_COLOR_TEXT_DIM  0x7BEF   // Light grey
#define HEXY_COLOR_PANEL     0x1082   // Dark grey

// ── Display driver class (mirrors hexiOS display_config.h) ───────────────────
class LGFX_HEXY : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789  _panel;
    lgfx::Bus_SPI       _bus;
    lgfx::Light_PWM     _backlight;

public:
    LGFX_HEXY() {
        // SPI bus
        {
            auto cfg = _bus.config();
            cfg.spi_host    = SPI2_HOST;
            cfg.spi_mode    = 0;
            cfg.freq_write  = 80000000;
            cfg.freq_read   = 16000000;
            cfg.spi_3wire   = false;
            cfg.use_lock    = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            cfg.pin_sclk    = HEXY_TFT_SCLK;
            cfg.pin_mosi    = HEXY_TFT_MOSI;
            cfg.pin_miso    = HEXY_TFT_MISO;
            cfg.pin_dc      = HEXY_TFT_DC;
            _bus.config(cfg);
            _panel.setBus(&_bus);
        }
        // Panel
        {
            auto cfg = _panel.config();
            cfg.pin_cs          = HEXY_TFT_CS;
            cfg.pin_rst         = HEXY_TFT_RST;
            cfg.pin_busy        = -1;
            cfg.panel_width     = 240;
            cfg.panel_height    = 280;
            cfg.offset_x        = 0;
            cfg.offset_y        = 20;
            cfg.offset_rotation = 0;
            cfg.readable        = false;
            cfg.invert          = true;
            cfg.rgb_order       = false;
            cfg.dlen_16bit      = false;
            cfg.bus_shared      = true;
            _panel.config(cfg);
        }
        // Backlight
        {
            auto cfg = _backlight.config();
            cfg.pin_bl      = HEXY_TFT_BL;
            cfg.invert      = false;
            cfg.freq        = 44100;
            cfg.pwm_channel = 1;
            _backlight.config(cfg);
            _panel.setLight(&_backlight);
        }
        setPanel(&_panel);
    }
};

// ── Main hexiOS class ─────────────────────────────────────────────────────────
class HexiOS {
public:
    LGFX_HEXY display;

    // Initialise display and configure button pins.
    // Call once in setup().
    void begin(uint8_t brightness = 200) {
        display.init();
        display.setBrightness(brightness);
        display.fillScreen((uint16_t)HEXY_COLOR_BG);

        pinMode(HEXY_BTN_LEFT,   INPUT_PULLUP);
        pinMode(HEXY_BTN_RIGHT,  INPUT_PULLUP);
        pinMode(HEXY_BTN_UP,     INPUT_PULLUP);
        pinMode(HEXY_BTN_DOWN,   INPUT_PULLUP);
        pinMode(HEXY_BTN_SELECT, INPUT_PULLUP);
        pinMode(HEXY_BTN_BACK,   INPUT_PULLUP);

        // Clear stale button state
        for (int i = 0; i < 6; i++) _lastState[i] = HIGH;
    }

    // Call every loop iteration to update button state
    void update() {
        const uint8_t pins[6] = {
            HEXY_BTN_LEFT, HEXY_BTN_RIGHT, HEXY_BTN_UP,
            HEXY_BTN_DOWN, HEXY_BTN_SELECT, HEXY_BTN_BACK
        };
        for (int i = 0; i < 6; i++) {
            bool cur = digitalRead(pins[i]);
            _pressed[i] = (_lastState[i] == HIGH && cur == LOW);
            _lastState[i] = cur;
        }
    }

    // One-shot press detection (true for one loop iteration after press)
    bool leftPressed()   { return _pressed[0]; }
    bool rightPressed()  { return _pressed[1]; }
    bool upPressed()     { return _pressed[2]; }
    bool downPressed()   { return _pressed[3]; }
    bool selectPressed() { return _pressed[4]; }
    bool backPressed()   { return _pressed[5]; }

    // True while button is physically held
    bool leftHeld()      { return digitalRead(HEXY_BTN_LEFT)   == LOW; }
    bool rightHeld()     { return digitalRead(HEXY_BTN_RIGHT)  == LOW; }
    bool upHeld()        { return digitalRead(HEXY_BTN_UP)     == LOW; }
    bool downHeld()      { return digitalRead(HEXY_BTN_DOWN)   == LOW; }
    bool selectHeld()    { return digitalRead(HEXY_BTN_SELECT) == LOW; }
    bool backHeld()      { return digitalRead(HEXY_BTN_BACK)   == LOW; }

    // Boot back into hexiOS (finds the ota_0 partition and restarts).
    // Call this when your app is done — typically when backPressed() is true.
    void returnToOS() {
        const esp_partition_t* app0 = esp_partition_find_first(
            ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, nullptr);
        if (app0) {
            esp_ota_set_boot_partition(app0);
        }
        esp_restart();
    }

private:
    bool    _pressed[6]   = {};
    uint8_t _lastState[6] = {};
};

#endif // HEXIOS_H
