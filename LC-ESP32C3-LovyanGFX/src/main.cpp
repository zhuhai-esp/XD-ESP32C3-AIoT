#include "lgfx.h"
#include <Arduino.h>

LGFX tft;

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setBrightness(10);
  tft.fillScreen(TFT_BLACK);

  tft.setTextFont(4);
  tft.drawString("Hello LovyanGFX!!", 0, 0);
  tft.drawRoundRect(20, 30, 200, 100, 12, TFT_YELLOW);
  tft.drawCircle(150, 150, 40, TFT_RED);
  tft.drawRect(60, 60, 30, 40, TFT_BLUE);
}

void loop() {
  static uint16_t touchX, touchY;
  bool touched = tft.getTouch(&touchX, &touchY);
  if (touched) {
    touchY = (touchY - 60) * 1.333;
    Serial.printf("Touch [%d][%d]\n", touchX, touchY);
    tft.fillCircle(touchX, touchY, 3);
  }
  delay(10);
}