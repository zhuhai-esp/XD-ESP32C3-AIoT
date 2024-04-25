#include "lgfx.h"
#include <Arduino.h>
#include <demos/lv_demos.h>
#include <lvgl.h>

LGFX tft;

#define LV_DISP_HOR_RES 320
#define LV_DISP_VER_RES 240

static const uint32_t buf_size = LV_DISP_HOR_RES * 20;
static lv_color_t dis_buf1[buf_size];
static lv_color_t dis_buf2[buf_size];

void inline lv_touch_init() {
  auto *indev_touchpad = lv_indev_create();
  lv_indev_set_type(indev_touchpad, LV_INDEV_TYPE_POINTER);
  auto f_touch = [](lv_indev_t *indev_drv, lv_indev_data_t *data) {
    uint16_t touchX, touchY;
    data->state = LV_INDEV_STATE_RELEASED;
    if (tft.getTouch(&touchX, &touchY)) {
      data->state = LV_INDEV_STATE_PRESSED;
      data->point.x = touchX;
      data->point.y = (touchY - 60) * 1.333;
    }
  };
  lv_indev_set_read_cb(indev_touchpad, f_touch);
}

void inline lv_disp_init() {
  auto *disp = lv_display_create(LV_DISP_HOR_RES, LV_DISP_VER_RES);
  auto f_disp = [](lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    if (tft.getStartCount() == 0) {
      tft.endWrite();
    }
    tft.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1,
                     area->y2 - area->y1 + 1, (lgfx::rgb565_t *)px_map);
    lv_display_flush_ready(disp);
  };
  lv_display_set_flush_cb(disp, f_disp);
  auto mode = LV_DISPLAY_RENDER_MODE_PARTIAL;
  lv_display_set_buffers(disp, dis_buf1, dis_buf2, buf_size, mode);
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setBrightness(5);
  tft.fillScreen(TFT_BLACK);
  lv_init();
  lv_tick_set_cb(millis);
  lv_disp_init();
  lv_touch_init();
  lv_demo_widgets();
}

void loop() { lv_timer_handler(); }