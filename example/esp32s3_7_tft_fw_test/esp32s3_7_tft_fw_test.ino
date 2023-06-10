#include <Arduino_GFX_Library.h>
#include <SD_MMC.h>
#include <TAMC_GT911.h>
#include <Wire.h>
#include <Audio.h>
#include "JpegFunc.h"

#define JPEG_FILENAME_LOGO "/logo.jpg"
#define JPEG_FILENAME_COVER "/cover.jpg"
#define JPEG_FILENAME_COVER_01 "/cover01.jpg"

#define I2S_DOUT 19
#define I2S_BCLK 20
#define I2S_LRC 2

// microSD card
#define PIN_SD_CMD 11
#define PIN_SD_CLK 12
#define PIN_SD_D0 13

#define I2C_SDA_PIN 17
#define I2C_SCL_PIN 18
#define TOUCH_INT -1
#define TOUCH_RST 38

#define TOUCH_ROTATION ROTATION_NORMAL

#define SCREEN_W 1024
#define SCREEN_H 600

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
    600 /* height */, 1 /* vsync_polarity */, 13 /* vsync_front_porch */, 3 /* vsync_pulse_width */, 45 /* vsync_back_porch */);

TAMC_GT911 ts = TAMC_GT911(I2C_SDA_PIN, I2C_SCL_PIN, TOUCH_INT, TOUCH_RST, SCREEN_W, SCREEN_H);
Audio audio;

String img_list[5] =
    {
        "/image01.jpg",
        "/image02.jpg",
        "/image03.jpg",
        "/image04.jpg",
        "/image05.jpg"};

struct Music_info
{
  String name;
  int length;
  int runtime;
  int volume;
  int status;
  int mute_volume;
} music_info = {"", 0, 0, 0, 0, 0};

String music_list[20];
int music_num = 0;
int music_index = 0;

int w = SCREEN_W;
int h = SCREEN_H;

int touch_last_x = 0;
int touch_last_y = 0;

int ColorArray[] = {BLACK, BLUE, GREEN, WHITE, RED, ORANGE};

void setup()
{

  Serial.begin(115200);
  Serial.println("ESP32S3 7inch LCD");

  pin_init();
  touch_init();

  // Init Display
  gfx->begin();

  for (int i = 0; i < 6; i++)
  {
    gfx->fillScreen(ColorArray[i]);
    delay(1000);
  }

  SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
  if (!SD_MMC.begin("/sdcard", true, true))
  {

    while (1)
    {
      Serial.println("Card Mount Failed");
      delay(1000);
    }
  }

  audio_init();

  jpegDraw(JPEG_FILENAME_LOGO, jpegDrawCallback, true /* useBigEndian */,
           0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);
  delay(1000);

  jpegDraw(JPEG_FILENAME_COVER_01, jpegDrawCallback, true /* useBigEndian */,
           0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);
  delay(500);

  while (1)
  {
    ts.read();
    if (ts.isTouched)
    {
      for (int i = 0; i < ts.touches; i++)
      {
        // gfx->fillScreen(WHITE);
        int temp = random(0, 6);
        gfx->fillScreen(ColorArray[temp]);

        gfx->setTextSize(4);

        if (temp == 4)
          gfx->setTextColor(WHITE);
        else
          gfx->setTextColor(RED);

        gfx->setCursor(w / 2, h / 2);
        gfx->print("X: ");
        gfx->println(String(ts.points[i].x));
        gfx->setCursor(w / 2, h / 2 + 100);
        gfx->print("Y: ");
        gfx->println(String(ts.points[i].y));

        // RTCShow(0);

        Serial.print("Touch ");
        Serial.print(i + 1);
        Serial.print(": ");
        ;
        Serial.print("  x: ");
        Serial.print(ts.points[i].x);
        Serial.print("  y: ");
        Serial.print(ts.points[i].y);
        Serial.print("  size: ");
        Serial.println(ts.points[i].size);
        Serial.println(' ');
      }

      ts.isTouched = false;
      if (ts.touches > 2)
        break;
    }
    delay(100);
  }

  open_new_song(music_list[0]);

  xTaskCreatePinnedToCore(Task_TFT, "Task_TFT", 20480, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(Task_Audio, "Task_Audio", 10240, NULL, 3, NULL, ARDUINO_RUNNING_CORE);
}

void Task_TFT(void *pvParameters) // This is a task.
{
  while (1)
    for (int i = 0; i < 5; i++)
    {
      Serial.println(img_list[i].c_str());
      jpegDraw(img_list[i].c_str(), jpegDrawCallback, true /* useBigEndian */,
               0 /* x */, 0 /* y */, w /* widthLimit */, h /* heightLimit */);

      vTaskDelay(1000);
    }
}

void Task_Audio(void *pvParameters) // This is a task.
{
  while (1)
    audio.loop();
}

void loop()
{
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

// pixel drawing callback
static int jpegDrawCallback(JPEGDRAW *pDraw)
{
  // Serial.printf("Draw pos = %d,%d. size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  return 1;
}

void touch_init(void)
{
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  ts.begin();
  ts.setRotation(TOUCH_ROTATION);
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