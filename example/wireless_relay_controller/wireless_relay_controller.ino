#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ui.h>

#define SSID "Makerfabs"
#define PWD "20160704"

#define TFT_BL 10

#define SCREEN_HD
#define SCREEN_W 1024
#define SCREEN_H 600

#include "touch.h"



/*Don't forget to set Sketchbook location in File/Preferencesto the path of your UI project (the parent foder of this INO file)*/

/*Change to your screen resolution*/
static const uint16_t screenWidth = 1024;
static const uint16_t screenHeight = 600;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];

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
    SCREEN_H /* height */, 1 /* vsync_polarity */, 13 /* vsync_front_porch */, 3 /* vsync_pulse_width */, 45 /* vsync_back_porch */,
    1 /* pclk_active_neg */, 16000000 /* prefer_speed */, true /* auto_flush */);

int net_flag = 0;
int index1;
int http_flag = 0;

void setup()
{
    USBSerial.begin(115200); /* prepare for possible serial debug */

    touch_init(gfx->width(), gfx->height());

    lv_init();

    // Init Display
    gfx->begin();
    gfx->fillScreen(BLACK);

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    ui_init();

    USBSerial.println("Setup done");

    xTaskCreatePinnedToCore(Task_TFT, "Task_TFT", 40960, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(Task_my, "Task_my", 20000, NULL, 1, NULL, 1);
}

void Task_TFT(void *pvParameters)
{
    while (1)
    {
        lv_timer_handler();
        vTaskDelay(10);
    }
}

long task_runtime_1 = 0;
void Task_my(void *pvParameters)
{
    while (1)
    {

        if (net_flag != 1)
            if ((millis() - task_runtime_1) > 1000)
            {
                wifi_init();
                task_runtime_1 = millis();
            }

        if (http_flag = 1)
        {
          
          http_flag = 0;
          http_func(index1);
          index1 = 4;
        }

        vTaskDelay(100);
    }

    
    
}

void loop()
{
}

//------------------------------------------------------------------------------------

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

    lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    if (touch_has_signal())
    {
        if (touch_touched())
        {
            data->state = LV_INDEV_STATE_PR;

            /*Set the coordinates*/
            data->point.x = touch_last_x;
            data->point.y = touch_last_y;
        }
        else if (touch_released())
        {
            data->state = LV_INDEV_STATE_REL;
        }
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

void wifi_init()
{
    String temp = "WiFi Connecting";
    lv_label_set_text(ui_Label5,temp.c_str());

    WiFi.begin(SSID, PWD);

    int connect_count = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(500);
        USBSerial.print(".");
        temp = temp + ".";
        lv_label_set_text(ui_Label5,temp.c_str());
        connect_count++;
    }

    lv_label_set_text(ui_Label5,"WiFi Connect Over");
    net_flag = 1;
}

// invert
// http://homeassistant.local:8123/api/webhook/-uWCIeRoUjy-lzQ7vbxnFQI3i

// all close
// http://homeassistant.local:8123/api/webhook/-LYt299nHgzxcCD95yct-G2jJ

// all open
// http://homeassistant.local:8123/api/webhook/-BhCrGgyQALW6F7b58N2PtoE7
void http_func(int index)
{
    HTTPClient http;

    USBSerial.print("[HTTP] begin...\n");

    String url = "";

    switch (index)
    {
    case 0:
        url = "http://homeassistant.local:8123/api/webhook/-LYt299nHgzxcCD95yct-G2jJ";
        break;
    case 1:
        url = "http://homeassistant.local:8123/api/webhook/-BhCrGgyQALW6F7b58N2PtoE7";
        break;
    case 2:
        url = "http://homeassistant.local:8123/api/webhook/-uWCIeRoUjy-lzQ7vbxnFQI3i";
        break;
    case 3:
        url = "http://homeassistant.local:8123/api/webhook/-0LPYwfu5Rr-1XTOGTnCDRmEU";
        break;

    default:
        USBSerial.println("Input error");
        return;
    }

    http.begin(url);

    USBSerial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
        // HTTP header has been send and Server response header has been handled
        USBSerial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            USBSerial.println(payload);
        }
    }
    else
    {
        USBSerial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}
