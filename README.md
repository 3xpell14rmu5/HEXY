# HEXY Board Package for hexiOS

This folder contains everything needed to write Arduino sketches that run on the
**HEXY** device as apps inside **hexiOS**.

## How it works

hexiOS reserves a second flash partition (`app1` at `0x310000`, ~2 MB) for user
apps.  When you select the **HEXY** board and click **Upload**, the Arduino IDE
flashes your sketch directly into that partition without touching hexiOS itself.

From the device:
- Press **RIGHT** on the home screen → **App Launcher**
- **LAUNCH** — boots into your sketch immediately (no install needed for testing)
- **INSTALL** — marks the sketch as the permanent user app that appears in the
  **APPS** menu

Your sketch calls `os.returnToOS()` (e.g. when Back is pressed) to boot back
into hexiOS.

---

## Installation — Option A: Board Manager URL (recommended)

> Requires Python 3 installed on your machine.

1. Open a terminal in this folder (`hexy_board/`):
   ```
   python generate_package.py
   ```
2. Copy the `file:///` URL that is printed at the end.
3. Arduino IDE → **File > Preferences** → paste the URL into
   *Additional boards manager URLs* → OK.
4. **Tools > Board > Boards Manager** → search **HEXY** → **Install**.
5. Select **Tools > Board > hexiOS (HEXY App Platform) > HEXY (hexiOS App)**.

---

## Installation — Option B: boards.local.txt (manual)

If you already have the **esp32 by Espressif** core installed:

1. Find your ESP32 core directory:
   - **Windows:** `C:\Users\<you>\AppData\Local\Arduino15\packages\esp32\hardware\esp32\<version>\`
   - **macOS:** `~/Library/Arduino15/packages/esp32/hardware/esp32/<version>/`
   - **Linux:** `~/.arduino15/packages/esp32/hardware/esp32/<version>/`
2. Copy `boards.local.txt` from this folder into that directory.
3. Restart Arduino IDE.
4. **Tools > Board > esp32 > HEXY (hexiOS App)**.

---

## hexiOS Library

The `hexiOS/` folder is an Arduino library.  Install it once:

**Sketch > Include Library > Add .ZIP Library** — select the `hexiOS/` folder
(or zip it first).

### Minimal sketch

```cpp
#include <hexiOS.h>

HexiOS os;

void setup() {
    os.begin();
    os.display.setTextDatum(MC_DATUM);
    os.display.setTextColor(HEXY_COLOR_TEXT);
    os.display.drawString("Hello from HEXY!", 120, 140);
}

void loop() {
    os.update();
    if (os.backPressed()) {
        os.returnToOS();   // goes back to hexiOS
    }
}
```

### API reference

| Method | Description |
|---|---|
| `os.begin(brightness)` | Init display + buttons (call in `setup()`) |
| `os.update()` | Refresh button state (call every `loop()`) |
| `os.display` | `LGFX_HEXY` instance — full LovyanGFX API |
| `os.leftPressed()` / `rightPressed()` / `upPressed()` / `downPressed()` | One-shot press |
| `os.selectPressed()` / `backPressed()` | One-shot press |
| `os.returnToOS()` | Boot back into hexiOS |

### Colour constants

`HEXY_COLOR_BG`, `HEXY_COLOR_PRIMARY` (cyan), `HEXY_COLOR_SECONDARY` (blue),
`HEXY_COLOR_ACCENT` (green), `HEXY_COLOR_WARNING` (orange),
`HEXY_COLOR_ERROR` (red), `HEXY_COLOR_TEXT` (white), `HEXY_COLOR_TEXT_DIM` (grey),
`HEXY_COLOR_PANEL` (dark grey).

---

## File layout

```
hexy_board/
├── generate_package.py       ← generates Board Manager zip + JSON index
├── boards.local.txt          ← manual install alternative
├── README.md
├── package/
│   ├── boards.txt            ← HEXY board definition
│   └── platform.txt          ← extends esp32:esp32s3
└── hexiOS/
    ├── library.properties
    └── src/
        └── hexiOS.h          ← HexiOS class + LGFX_HEXY driver
```
