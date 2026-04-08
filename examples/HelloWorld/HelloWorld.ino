/*
 * HelloWorld — hexiOS App Template
 *
 * This is the simplest possible hexiOS app.  It shows a message on screen
 * and returns to hexiOS when the user presses BACK.
 *
 * SETUP:
 *   1. Install "esp32 by Espressif" (v2.x) from Boards Manager.
 *   2. Install the hexiOS board package (adds "HEXY (hexiOS App)" board).
 *   3. Install the hexiOS library (Sketch > Include Library > Add .ZIP Library).
 *   4. Select board:  Tools > Board > hexiOS > HEXY (hexiOS App)
 *   5. Upload this sketch.  hexiOS will detect the app automatically.
 */

#include <hexiOS.h>

HexiOS os;

void setup() {
    os.begin();

    // Black background
    os.display.fillScreen((uint16_t)HEXY_COLOR_BG);

    // Title
    os.display.setFont(&fonts::FreeSansBold18pt7b);
    os.display.setTextColor((uint16_t)HEXY_COLOR_PRIMARY);
    os.display.setTextDatum(MC_DATUM);
    os.display.drawString("Hello", 120, 100);

    os.display.setFont(&fonts::FreeSansBold12pt7b);
    os.display.setTextColor((uint16_t)HEXY_COLOR_ACCENT);
    os.display.drawString("from hexiOS!", 120, 140);

    // Hint
    os.display.setFont(&fonts::Font0);
    os.display.setTextColor((uint16_t)HEXY_COLOR_TEXT_DIM);
    os.display.setTextDatum(MC_DATUM);
    os.display.drawString("Press BACK to return to hexiOS", 120, 260);
}

void loop() {
    os.update();

    if (os.backPressed()) {
        os.returnToOS();
    }

    delay(20);
}
