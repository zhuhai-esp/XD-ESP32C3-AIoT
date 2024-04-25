#include "lgfx.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <lvgl.h>

LGFX tft;

#define LV_DISP_HOR_RES 320
#define LV_DISP_VER_RES 240

static const uint32_t buf_size = LV_DISP_HOR_RES * 20;
static lv_color_t dis_buf1[buf_size];
static lv_color_t dis_buf2[buf_size];

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;

void inline lv_drivers_init() {
  lv_disp_draw_buf_init(&draw_buf, dis_buf1, dis_buf2, buf_size);
  lv_disp_drv_init(&disp_drv);

  auto f_disp = [](lv_disp_drv_t *disp, const lv_area_t *area,
                   lv_color_t *color_p) {
    if (tft.getStartCount() == 0) {
      tft.endWrite();
    }
    tft.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1,
                     area->y2 - area->y1 + 1, (lgfx::rgb565_t *)&color_p->full);
    lv_disp_flush_ready(disp);
  };
  disp_drv.hor_res = LV_DISP_HOR_RES;
  disp_drv.ver_res = LV_DISP_VER_RES;
  disp_drv.flush_cb = f_disp;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
}

long check1s = 0;
long check5ms = 0;
char buf[48] = {0};

lv_obj_t *label_date;
lv_obj_t *label_ip;
lv_obj_t *label_time;
lv_obj_t *img_second;
lv_obj_t *img_minute;
lv_obj_t *img_hour;

LV_IMG_DECLARE(watch_bg);
LV_IMG_DECLARE(hour);
LV_IMG_DECLARE(minute);
LV_IMG_DECLARE(second);

void ui_init(void) {
  auto *parent = lv_scr_act();

  label_date = lv_label_create(parent);
  lv_obj_align(label_date, LV_ALIGN_TOP_RIGHT, -4, 4);
  lv_label_set_text(label_date, "2000-01-01");

  label_ip = lv_label_create(parent);
  lv_obj_align(label_ip, LV_ALIGN_TOP_LEFT, 4, 4);
  lv_obj_set_style_text_font(label_ip, &lv_font_montserrat_12, LV_PART_MAIN);
  lv_label_set_text(label_ip, "IP: 0.0.0.0");

  label_time = lv_label_create(parent);
  lv_obj_align(label_time, LV_ALIGN_BOTTOM_LEFT, 4, -4);
  lv_label_set_text(label_time, "12:33:45");

  auto *panel = lv_img_create(parent);
  lv_img_set_src(panel, &watch_bg);
  lv_obj_set_align(panel, LV_ALIGN_CENTER);

  img_hour = lv_img_create(parent);
  lv_img_set_src(img_hour, &hour);
  lv_obj_set_align(img_hour, LV_ALIGN_CENTER);

  img_minute = lv_img_create(parent);
  lv_img_set_src(img_minute, &minute);
  lv_obj_set_align(img_minute, LV_ALIGN_CENTER);

  img_second = lv_img_create(parent);
  lv_img_set_src(img_second, &second);
  lv_obj_set_align(img_second, LV_ALIGN_CENTER);
}

inline void showClientIP() {
  sprintf(buf, "IP: %s", WiFi.localIP().toString().c_str());
  lv_label_set_text(label_ip, buf);
}

inline void showCurrentTime() {
  struct tm info;
  getLocalTime(&info);
  strftime(buf, 32, "%F", &info);
  lv_label_set_text(label_date, buf);
  strftime(buf, 32, "%T", &info);
  lv_label_set_text(label_time, buf);

  lv_img_set_angle(img_hour, info.tm_hour * 300 + info.tm_min / 12 % 12 * 60);
  lv_img_set_angle(img_minute, info.tm_min * 6 * 10);
  lv_img_set_angle(img_second, info.tm_sec * 6 * 10);
}

void inline autoConfigWifi() {
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin();
  for (int i = 0; WiFi.status() != WL_CONNECTED && i < 100; i++) {
    delay(100);
  }
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.beginSmartConfig();
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
    }
    WiFi.stopSmartConfig();
    WiFi.mode(WIFI_MODE_STA);
  }
}

inline void show_center_msg(const char *msg) {
  tft.drawCenterString(msg, LV_DISP_HOR_RES / 2, LV_DISP_VER_RES / 2,
                       &fonts::AsciiFont8x16);
}

void inline setupOTAConfig() {
  ArduinoOTA.onStart([] { show_center_msg("OTA Update"); });
  ArduinoOTA.onProgress([](u32_t pro, u32_t total) {
    sprintf(buf, "OTA Updating: %d / %d", pro, total);
    show_center_msg(buf);
  });
  ArduinoOTA.onEnd([] { show_center_msg("OTA Sccess, Restarting..."); });
  ArduinoOTA.onError([](ota_error_t err) {
    sprintf(buf, "OTA Error [%d]!!", err);
    show_center_msg(buf);
  });
  ArduinoOTA.begin();
}

void inline startConfigTime() {
  const int timeZone = 8 * 3600;
  configTime(timeZone, 0, "ntp6.aliyun.com", "cn.ntp.org.cn", "ntp.ntsc.ac.cn");
  while (time(nullptr) < 8 * 3600 * 2) {
    delay(500);
  }
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setBrightness(5);
  tft.fillScreen(TFT_BLACK);
  lv_init();
  lv_drivers_init();

  ui_init();
  show_center_msg("Please Config WiFi With ESP-Touch!");
  autoConfigWifi();
  startConfigTime();
  setupOTAConfig();
  showClientIP();
}

void loop() {
  auto ms = millis();
  if (ms - check1s > 1000) {
    check1s = ms;
    ArduinoOTA.handle();
    showCurrentTime();
  }
  if (ms - check5ms > 5) {
    check5ms = ms;
    lv_timer_handler();
  }
}