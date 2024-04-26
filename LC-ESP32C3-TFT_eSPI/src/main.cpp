#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft;

void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  pinMode(TFT_BL, OUTPUT);
  analogWriteResolution(10);
  analogWrite(TFT_BL, 200);

  tft.setTextFont(4);
  tft.drawString("Hello TFT_eSPI!!", 0, 0);
  tft.drawRoundRect(20, 30, 200, 100, 12, TFT_YELLOW);
  tft.drawCircle(150, 150, 40, TFT_RED);
  tft.drawRect(60, 60, 30, 40, TFT_BLUE);
}

void loop() {}