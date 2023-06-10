#include <Arduino_GFX_Library.h>

#define TFT_BL 10

Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    GFX_NOT_DEFINED /* CS */, GFX_NOT_DEFINED /* SCK */, GFX_NOT_DEFINED /* SDA */,
    40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
    45 /* R0 */, 48 /* R1 */, 47 /* R2 */, 21 /* R3 */, 14 /* R4 */,
    5 /* G0 */, 6 /* G1 */, 7 /* G2 */, 15 /* G3 */, 16 /* G4 */, 4 /* G5 */,
    8 /* B0 */, 3 /* B1 */, 46 /* B2 */, 9 /* B3 */, 1 /* B4 */
);

Arduino_RPi_DPI_RGBPanel *gfx = new Arduino_RPi_DPI_RGBPanel(
    bus,
    1024 /* width */, 1 /* hsync_polarity */, 40 /* hsync_front_porch */, 48 /* hsync_pulse_width */, 128 /* hsync_back_porch */,
    600 /* height */, 1 /* vsync_polarity */, 13 /* vsync_front_porch */, 3 /* vsync_pulse_width */, 45 /* vsync_back_porch */,
    1 /* pclk_active_neg */, 12000000 /* prefer_speed */, true /* auto_flush */);

void setup(void)
{
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, LOW);

    gfx->begin();
    gfx->fillScreen(BLACK);

    delay(2000); // 5 seconds
}

void loop()
{
    gfx->fillRect(0, 0, 512, 600, RED);
    gfx->fillRect(512, 0, 512, 600, GREEN);
    delay(3000);

    gfx->fillRect(0, 0, 512, 600, YELLOW);
    gfx->fillRect(512, 0, 512, 600, BLUE);
    delay(3000);

    gfx->fillRect(0, 0, 512, 600, BLACK);
    gfx->fillRect(512, 0, 512, 600, WHITE);
    delay(3000);
}
