#include <Arduino_GFX_Library.h>
#include <SD_MMC.h>
#include <TAMC_GT911.h>
#include <Wire.h>
#include <Audio.h>
#include "JpegFunc.h"

#define SCREEN_HD
// #define SCREEN_NORMAL

#define TFT_BL 10

#define I2S_DOUT 19
#define I2S_BCLK 20
#define I2S_LRC 2

// microSD card
#define PIN_SD_CMD 11
#define PIN_SD_CLK 12
#define PIN_SD_D0 13

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


struct Music_info
{
    String name;
    int length;
    int runtime;
    int volume;
    int status;
    int mute_volume;
} music_info = {"", 0, 0, 0, 0, 0};

Audio audio;

String image_list[3] = {

    "/image01.jpg",
    "/image02.jpg",
    "/image03.jpg"

};

String music_list[20];
int music_num = 0;
int music_index = 0;

//---- Main --------------------------------------------------

void setup()
{
    Serial.begin(115200);

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, LOW);

    gfx->begin();
    gfx->fillScreen(YELLOW);

    sd_init();
    audio_init();

    open_new_song(music_list[music_index]);

    xTaskCreatePinnedToCore(Task_TFT, "Task_TFT", 20480, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
    xTaskCreatePinnedToCore(Task_Audio, "Task_Audio", 10240, NULL, 3, NULL, ARDUINO_RUNNING_CORE);
}

void loop()
{
}

void Task_TFT(void *pvParameters) // This is a task.
{
    while (1) // A Task shall never return or exit.
    {
        jpegDraw(image_list[0].c_str(), jpegDrawCallback, true, 0, 0, gfx->width(), gfx->height());
        vTaskDelay(2000);

        jpegDraw(image_list[1].c_str(), jpegDrawCallback, true, 0, 0, gfx->width(), gfx->height());
        vTaskDelay(2000);

        jpegDraw(image_list[2].c_str(), jpegDrawCallback, true, 0, 0, gfx->width(), gfx->height());
        vTaskDelay(2000);
    }
}

void Task_Audio(void *pvParameters) // This is a task.
{
    while (1)
        audio.loop();
}

static int jpegDrawCallback(JPEGDRAW *pDraw)
{
    // Serial.printf("Draw pos = %d,%d. size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
    gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
    return 1;
}

//---- Device init --------------------------------------------------

void sd_init()
{
    SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
    if (!SD_MMC.begin("/sdcard", true, true))
    {
        USBSerial.println("Card Mount Failed");
        return;
    }
}

void audio_init()
{
    // Read SD
    music_num = get_music_list(SD_MMC, "/", 0, music_list);
    Serial.print("Music file count:");
    Serial.println(music_num);
    Serial.println("All music:");
    for (int i = 0; i < music_num; i++)
    {
        Serial.println(music_list[i]);
    }

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // 0...21
}

//----- Audio Function --------------------------------------------------

int get_music_list(fs::FS &fs, const char *dirname, uint8_t levels, String wavlist[30])
{
    Serial.printf("Listing directory: %s\n", dirname);
    int i = 0;

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return i;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return i;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
        }
        else
        {
            String temp = file.name();
            if (temp.endsWith(".wav"))
            {
                wavlist[i] = temp;
                i++;
            }
            else if (temp.endsWith(".mp3"))
            {
                wavlist[i] = temp;
                i++;
            }
        }
        file = root.openNextFile();
    }
    return i;
}

void open_new_song(String filename)
{
    // 去掉文件名的根目录"/"和文件后缀".mp3",".wav"
    music_info.name = filename.substring(1, filename.indexOf("."));
    audio.connecttoFS(SD_MMC, filename.c_str());
    music_info.runtime = audio.getAudioCurrentTime();
    music_info.length = audio.getAudioFileDuration();
    music_info.volume = audio.getVolume();
    music_info.status = 1;
    Serial.println("**********start a new sound************");
}

void audio_eof_mp3(const char *info)
{ // end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);

    music_index++;
    if (music_index >= music_num)
    {
        music_index = 0;
    }
    open_new_song(music_list[music_index]);
}