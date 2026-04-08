/*
 * Counter — hexiOS App Example
 *
 * UP/DOWN to change the count, SELECT to reset, BACK to exit.
 * Demonstrates button handling, display update, and the hexiOS colour palette.
 */

#include <hexiOS.h>

HexiOS os;
int count = 0;

void drawCount() {
    // Clear number area
    os.display.fillRect(0, 80, 240, 80, (uint16_t)HEXY_COLOR_BG);

    // Big number
    os.display.setFont(&fonts::FreeSansBold18pt7b);
    os.display.setTextDatum(MC_DATUM);

    uint16_t col;
    if (count > 0)      col = (uint16_t)HEXY_COLOR_ACCENT;
    else if (count < 0) col = (uint16_t)HEXY_COLOR_ERROR;
    else                 col = (uint16_t)HEXY_COLOR_TEXT;

    os.display.setTextColor(col);

    char buf[12];
    snprintf(buf, sizeof(buf), "%d", count);
    os.display.drawString(buf, 120, 120);
}

void setup() {
    os.begin();
    os.display.fillScreen((uint16_t)HEXY_COLOR_BG);

    // Title
    os.display.setFont(&fonts::FreeSansBold12pt7b);
    os.display.setTextColor((uint16_t)HEXY_COLOR_PRIMARY);
    os.display.setTextDatum(MC_DATUM);
    os.display.drawString("Counter", 120, 30);

    // Controls hint
    os.display.setFont(&fonts::Font0);
    os.display.setTextColor((uint16_t)HEXY_COLOR_TEXT_DIM);
    os.display.drawString("UP +1   DOWN -1   OK reset", 120, 200);
    os.display.drawString("BACK to exit", 120, 260);

    drawCount();
}

void loop() {
    os.update();

    bool changed = false;

    if (os.upPressed())     { count++; changed = true; }
    if (os.downPressed())   { count--; changed = true; }
    if (os.selectPressed()) { count = 0; changed = true; }

    if (changed) drawCount();
    if (os.backPressed()) os.returnToOS();

    delay(20);
}
