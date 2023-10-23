#include <Arduino_GFX_Library.h>
#include <TAMC_GT911.h>
#include <Wire.h>

// #define SCREEN_HD
#define SCREEN_NORMAL

#define I2C_SDA_PIN 17
#define I2C_SCL_PIN 18
#define TOUCH_INT -1
#define TOUCH_RST 38
#define TFT_BL 10

#ifdef SCREEN_HD
#define SCREEN_W 1024
#define SCREEN_H 600
#define TOUCH_ROTATION ROTATION_INVERTED
#endif

#ifdef SCREEN_NORMAL
#define SCREEN_W 800
#define SCREEN_H 480
#define TOUCH_ROTATION ROTATION_NORMAL
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

TAMC_GT911 ts = TAMC_GT911(I2C_SDA_PIN, I2C_SCL_PIN, TOUCH_INT, TOUCH_RST, 1024, 600);

int w = SCREEN_W;
int h = SCREEN_H;

int touch_last_x = 0;
int touch_last_y = 0;

void setup()
{

    Serial.begin(115200);
    Serial.println("ESP32S3 7inch LCD");

    pin_init();
    touch_init();

    // Init Display
    gfx->begin();

    gfx->fillScreen(WHITE);
    gfx->fillRect(0, 0, 50, 50, RED);
}

void loop()
{
    touch_read();
    gfx->fillRect(touch_last_x, touch_last_y, 10, 10, BLACK);
    delay(10);
}

//-------------------------------------------------------------------------

void pin_init()
{
    pinMode(TFT_BL, OUTPUT);
    pinMode(TOUCH_RST, OUTPUT);

    digitalWrite(TFT_BL, LOW);
    delay(100);
    digitalWrite(TOUCH_RST, LOW);
    delay(1000);
    digitalWrite(TOUCH_RST, HIGH);
    delay(1000);
    digitalWrite(TOUCH_RST, LOW);
    delay(1000);
    digitalWrite(TOUCH_RST, HIGH);
    delay(1000);
}

void touch_init(void)
{
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    ts.begin();
    ts.setRotation(TOUCH_ROTATION);
}

void touch_read()
{
    ts.read();
    if (ts.isTouched)
    {
#ifdef SCREEN_HD
        touch_last_x = map(ts.points[0].x, 0, 1024, 0, SCREEN_W);
        touch_last_y = map(ts.points[0].y, 0, 750, 0, SCREEN_H);

#endif
#ifdef SCREEN_NORMAL
        touch_last_x = map(ts.points[0].x, 1024, 200, 0, SCREEN_W);
        touch_last_y = map(ts.points[0].y, 600, 120, 0, SCREEN_H);

#endif

        Serial.print("TS x: ");
        Serial.print(ts.points[0].x);
        Serial.print("TS y: ");
        Serial.print(ts.points[0].y);

        Serial.print("Display x: ");
        Serial.print(touch_last_x);
        Serial.print("Display y: ");
        Serial.println(touch_last_y);

        ts.isTouched = false;
    }
}