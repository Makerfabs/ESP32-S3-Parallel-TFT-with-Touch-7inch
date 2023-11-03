#include <Wire.h>
#include "TAMC_GT911.h"

#define I2C_SDA_PIN 17
#define I2C_SCL_PIN 18
#define TOUCH_INT -1
#define TOUCH_RST 38

int16_t touch_last_x = 0, touch_last_y = 0;

TAMC_GT911 ts = TAMC_GT911(I2C_SDA_PIN, I2C_SCL_PIN, TOUCH_INT, TOUCH_RST, 1024, 600);

void touch_init(int max_x, int max_y)
{
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  ts.begin();

#ifdef SCREEN_HD
  ts.setRotation(ROTATION_INVERTED);

#endif
#ifdef SCREEN_NORMAL
  ts.setRotation(ROTATION_NORMAL);

#endif
}

bool touch_has_signal()
{

  return true;
}

bool touch_touched()
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

    return true;
  }
  return false;
}

bool touch_released()
{

  return true;
}
