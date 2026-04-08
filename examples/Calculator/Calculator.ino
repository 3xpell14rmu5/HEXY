#include <hexiOS.h>

HexiOS os;

// ── Layout ────────────────────────────────────────────────────────────────────
#define DISP_Y   0
#define DISP_H   60
#define GRID_X   5
#define GRID_Y   68
#define BTN_W    54
#define BTN_H    46
#define BTN_GAP  3

// ── Key grid (4 cols × 4 rows) ────────────────────────────────────────────────
const char* KEYS[4][4] = {
    { "7", "8", "9", "/" },
    { "4", "5", "6", "*" },
    { "1", "2", "3", "-" },
    { "C", "0", "=", "+" }
};

int curCol = 1;   // start on "0"
int curRow = 3;

// ── Calculator state ──────────────────────────────────────────────────────────
double  valA       = 0;
char    op         = 0;
bool    hasOp      = false;
bool    newInput   = true;
bool    isError    = false;
String  dispStr    = "0";

// ── Drawing ───────────────────────────────────────────────────────────────────
void drawDisplay() {
    os.display.fillRect(0, DISP_Y, 240, DISP_H, (uint16_t)HEXY_COLOR_BG);
    os.display.drawFastHLine(0, DISP_H, 240, (uint16_t)HEXY_COLOR_PANEL);

    // Operator badge top-left
    if (hasOp && !isError) {
        os.display.setFont(&fonts::FreeSansBold9pt7b);
        os.display.setTextSize(1);
        os.display.setTextColor((uint16_t)HEXY_COLOR_ACCENT);
        os.display.setTextDatum(ML_DATUM);
        char opStr[2] = { op, 0 };
        os.display.drawString(opStr, 8, DISP_H / 2);
    }

    // Main number — auto-shrink for long strings
    os.display.setTextSize(1);
    os.display.setTextColor(isError ? (uint16_t)HEXY_COLOR_ERROR : (uint16_t)HEXY_COLOR_TEXT);
    os.display.setTextDatum(MR_DATUM);

    if (dispStr.length() > 11) {
        os.display.setFont(&fonts::FreeSansBold9pt7b);
    } else if (dispStr.length() > 7) {
        os.display.setFont(&fonts::FreeSansBold12pt7b);
    } else {
        os.display.setFont(&fonts::FreeSansBold18pt7b);
    }
    os.display.drawString(dispStr.c_str(), 234, DISP_H / 2);
}

void drawButton(int col, int row, bool selected) {
    int x = GRID_X + col * (BTN_W + BTN_GAP);
    int y = GRID_Y + row * (BTN_H + BTN_GAP);
    char k = KEYS[row][col][0];

    uint16_t fill;
    if (selected) {
        fill = (uint16_t)HEXY_COLOR_PRIMARY;
    } else if (k == '=' ) {
        fill = (uint16_t)HEXY_COLOR_ACCENT;
    } else if (k == 'C') {
        fill = (uint16_t)HEXY_COLOR_ERROR;
    } else if (k == '/' || k == '*' || k == '-' || k == '+') {
        fill = (uint16_t)HEXY_COLOR_SECONDARY;
    } else {
        fill = (uint16_t)HEXY_COLOR_PANEL;
    }

    os.display.fillRoundRect(x, y, BTN_W, BTN_H, 7, fill);
    if (selected) {
        os.display.drawRoundRect(x,     y,     BTN_W,     BTN_H,     7, (uint16_t)HEXY_COLOR_TEXT);
        os.display.drawRoundRect(x + 1, y + 1, BTN_W - 2, BTN_H - 2, 6, (uint16_t)HEXY_COLOR_TEXT);
    }

    os.display.setFont(&fonts::FreeSansBold9pt7b);
    os.display.setTextSize(1);
    os.display.setTextColor(selected ? (uint16_t)HEXY_COLOR_BG : (uint16_t)HEXY_COLOR_TEXT);
    os.display.setTextDatum(MC_DATUM);
    os.display.drawString(KEYS[row][col], x + BTN_W / 2, y + BTN_H / 2);
}

void drawAllButtons() {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            drawButton(c, r, c == curCol && r == curRow);
}

void drawHint() {
    os.display.setFont(&fonts::Font0);
    os.display.setTextSize(1);
    os.display.setTextColor((uint16_t)HEXY_COLOR_TEXT_DIM);
    os.display.setTextDatum(MC_DATUM);
    os.display.drawString("Arrows=move  OK=press  BACK=exit", 120, 272);
}

// ── Calculator logic ──────────────────────────────────────────────────────────
String formatResult(double v) {
    if (v == (long long)v && fabs(v) < 1e12) {
        return String((long long)v);
    }
    String s = String(v, 8);
    // Trim trailing zeros after decimal point
    if (s.indexOf('.') != -1) {
        while (s.endsWith("0")) s.remove(s.length() - 1);
        if (s.endsWith(".")) s.remove(s.length() - 1);
    }
    return s;
}

void pressKey(int col, int row) {
    char k = KEYS[row][col][0];
    isError = false;

    // Clear
    if (k == 'C') {
        valA = 0; op = 0; hasOp = false; newInput = true;
        dispStr = "0";
        return;
    }

    // Digit
    if (k >= '0' && k <= '9') {
        if (newInput || dispStr == "0") {
            dispStr = String(k); newInput = false;
        } else if (dispStr.length() < 13) {
            dispStr += k;
        }
        return;
    }

    // Decimal point
    if (k == '.') {
        if (newInput) { dispStr = "0."; newInput = false; return; }
        if (dispStr.indexOf('.') == -1) dispStr += '.';
        return;
    }

    double current = dispStr.toDouble();

    // Equals
    if (k == '=') {
        if (!hasOp) return;
        double result = 0;
        switch (op) {
            case '+': result = valA + current; break;
            case '-': result = valA - current; break;
            case '*': result = valA * current; break;
            case '/':
                if (current == 0) { dispStr = "Div/0"; isError = true; hasOp = false; return; }
                result = valA / current; break;
        }
        dispStr = formatResult(result);
        valA = result;
        hasOp = false;
        newInput = true;
        return;
    }

    // Operator (+, -, *, /)
    valA = current;
    op = k;
    hasOp = true;
    newInput = true;
}

// ── Setup / Loop ──────────────────────────────────────────────────────────────
void setup() {
    os.begin();
    os.display.fillScreen((uint16_t)HEXY_COLOR_BG);
    drawDisplay();
    drawAllButtons();
    drawHint();
}

void loop() {
    os.update();

    int pc = curCol, pr = curRow;

    if (os.upPressed()    && curRow > 0) curRow--;
    if (os.downPressed()  && curRow < 3) curRow++;
    if (os.leftPressed()  && curCol > 0) curCol--;
    if (os.rightPressed() && curCol < 3) curCol++;

    if (curCol != pc || curRow != pr) {
        drawButton(pc, pr, false);
        drawButton(curCol, curRow, true);
    }

    if (os.selectPressed()) {
        pressKey(curCol, curRow);
        drawDisplay();
        drawAllButtons();
    }

    if (os.backPressed()) os.returnToOS();

    delay(15);
}
