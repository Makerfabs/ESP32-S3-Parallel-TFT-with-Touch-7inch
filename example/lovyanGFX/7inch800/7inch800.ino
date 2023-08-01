#define LGFX_USE_V1

#include <LovyanGFX.hpp>
#include <vector>
#include <SD_MMC.h>
#include "Matouch7.h"
#include "JpegFunc.h"

// #define SCREEN_HD
#define SCREEN_NORMAL

#ifdef SCREEN_HD
#define SCREEN_W 1024
#define SCREEN_H 600
#endif

#ifdef SCREEN_NORMAL
#define SCREEN_W 800
#define SCREEN_H 480
#endif

#define JPEG_FILENAME_LOGO "/logo.jpg"

// microSD card
#define PIN_SD_CMD 11
#define PIN_SD_CLK 12
#define PIN_SD_D0 13

String img_list[5] =
    {
        "/image01.jpg",
        "/image02.jpg",
        "/image03.jpg",
        "/image04.jpg",
        "/image05.jpg"};

LGFX lcd;

// pixel drawing callback
static int jpegDrawCallback(JPEGDRAW *pDraw)
{
    // Serial.printf("Draw pos = %d,%d. size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
    // gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
    lcd.pushImageDMA(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, (lgfx::swap565_t *)pDraw->pPixels);

    return 1;
}

void setup(void)
{
    Serial.begin(115200);
    Serial.println("ESP32S3 7inch LCD");

    lcd.init();

    lcd.fillScreen(TFT_WHITE);
    lcd.setTextColor(TFT_BLACK);
    lcd.setTextFont(4);
    lcd.setCursor(40, 100);
    lcd.printf("MaTouch ESP32 S3 Parallel TFT with Touch 7inch");
    lcd.setTextFont(4);
    lcd.setCursor(40, 160);
    lcd.printf("LovyanGFX Display Demo");
    delay(3000);

    SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
    if (!SD_MMC.begin("/sdcard", true, true))
    {

        while (1)
        {
            Serial.println("Card Mount Failed");
            delay(1000);
        }
    }

    jpegDraw(JPEG_FILENAME_LOGO, jpegDrawCallback, true /* useBigEndian */,
             0 /* x */, 0 /* y */, SCREEN_W /* widthLimit */, SCREEN_H /* heightLimit */);
    delay(1000);
}

static int colors[] = {
    TFT_RED,
    TFT_GREEN,
    TFT_BLUE,
    TFT_YELLOW,
    TFT_WHITE,
};

int i = 0;

void loop(void)
{
    // fps
    //   static int prev_sec;
    //   static int fps;
    //   ++fps;
    //   int sec = millis() / 1000;
    //   if (prev_sec != sec)
    //   {
    //     prev_sec = sec;
    //     lcd.setCursor(0, 310);
    //     lcd.printf("fps:%03d", fps);
    //     fps = 0;
    //   }

    //   lcd.fillRect(0, 0, 240, 310, colors[i++]);
    // lcd.fillScreen(colors[i++]);
    // if (i > 5)
    //     i = 0;

    for (int i = 0; i < 5; i++)
    {
        Serial.println(img_list[i].c_str());
        jpegDraw(img_list[i].c_str(), jpegDrawCallback, true /* useBigEndian */,
                 0 /* x */, 0 /* y */, SCREEN_W /* widthLimit */, SCREEN_H /* heightLimit */);

        delay(1000);
    }
}
