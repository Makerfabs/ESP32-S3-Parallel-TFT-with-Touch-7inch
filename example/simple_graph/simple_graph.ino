#include <Arduino_GFX_Library.h>

// #define SCREEN_HD
#define SCREEN_NORMAL

#define TFT_BL 10

#ifdef SCREEN_HD
#define SCREEN_W 1024
#define SCREEN_H 600
#endif

#ifdef SCREEN_NORMAL
#define SCREEN_W 800
#define SCREEN_H 480
#endif

Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    GFX_NOT_DEFINED /* CS */, GFX_NOT_DEFINED /* SCK */, GFX_NOT_DEFINED /* SDA */,
    40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
    45 /* R0 */, 48 /* R1 */, 47 /* R2 */, 21 /* R3 */, 14 /* R4 */,
    5 /* G0 */, 6 /* G1 */, 7 /* G2 */, 15 /* G3 */, 16 /* G4 */, 4 /* G5 */,
    8 /* B0 */, 3 /* B1 */, 46 /* B2 */, 9 /* B3 */, 1 /* B4 */
);

Arduino_RPi_DPI_RGBPanel *gfx = new Arduino_RPi_DPI_RGBPanel(
    bus,
    SCREEN_W /* width */, 1 /* hsync_polarity */, 40 /* hsync_front_porch */, 48 /* hsync_pulse_width */, 128 /* hsync_back_porch */,
    SCREEN_H /* height */, 1 /* vsync_polarity */, 13 /* vsync_front_porch */, 3 /* vsync_pulse_width */, 45 /* vsync_back_porch */);

int w = SCREEN_W;
int h = SCREEN_H;

void setup(void)
{
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, LOW);

    gfx->begin();
    gfx->fillScreen(WHITE);

    delay(1000); // 5 seconds
}

void loop()
{
    gfx->fillRect(0, 0, w / 2, h, RED);
    gfx->fillRect(w / 2, 0, w / 2, h, GREEN);
    delay(3000);

    gfx->fillRect(0, 0, w / 2, h, YELLOW);
    gfx->fillRect(w / 2, 0, w / 2, h, BLUE);
    delay(3000);

    gfx->fillRect(0, 0, w / 2, h, BLACK);
    gfx->fillRect(w / 2, 0, w / 2, h, WHITE);
    delay(3000);

    gfx->fillRect(0, 0, w, h / 2, RED);
    gfx->fillRect(0, h / 2, w, h / 2, GREEN);
    delay(3000);

    gfx->fillRect(0, 0, w, h / 2, YELLOW);
    gfx->fillRect(0, h / 2, w, h / 2, BLUE);
    delay(3000);

    gfx->fillRect(0, 0, w, h / 2, BLACK);
    gfx->fillRect(0, h / 2, w, h / 2, WHITE);
    delay(3000);
}
