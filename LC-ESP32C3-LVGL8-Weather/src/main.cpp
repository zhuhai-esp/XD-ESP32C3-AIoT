#include "lgfx.h"
#include "web_api.h"
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

using namespace std;

LV_FONT_DECLARE(simsun_16)
LV_FONT_DECLARE(led_48)

static const char *WEEK_DAYS[] = {"日", "一", "二", "三", "四", "五", "六"};

long check1s = 0;
long check5ms = 0;
long check5min = 0;

char buf[48] = {0};

lv_obj_t *label_date;
lv_obj_t *label_time;
lv_obj_t *label_loc;
lv_obj_t *label_wind;
lv_obj_t *label_ip;

static json city_info;
static json weather;

void start_lv_app() {
  lv_obj_t *parent = lv_scr_act();
  label_date = lv_label_create(parent);
  lv_label_set_text(label_date, "2000年01月01日 星期一");
  lv_obj_set_style_text_font(label_date, &simsun_16, LV_PART_MAIN);
  lv_obj_align(label_date, LV_ALIGN_TOP_MID, 0, 10);

  label_time = lv_label_create(parent);
  lv_label_set_text(label_time, "08:00:00");
  lv_obj_set_style_text_font(label_time, &led_48, LV_PART_MAIN);
  lv_obj_align(label_time, LV_ALIGN_TOP_MID, 0, 45);

  label_loc = lv_label_create(parent);
  lv_label_set_text(label_loc, "");
  lv_obj_set_style_text_font(label_loc, &simsun_16, LV_PART_MAIN);
  lv_obj_align(label_loc, LV_ALIGN_TOP_MID, 0, 100);

  label_wind = lv_label_create(parent);
  lv_label_set_text(label_wind, "");
  lv_obj_set_style_text_font(label_wind, &simsun_16, LV_PART_MAIN);
  lv_obj_align(label_wind, LV_ALIGN_TOP_MID, 0, 130);

  label_ip = lv_label_create(parent);
  lv_label_set_text(label_ip, "IP: 127.0.0.1");
  lv_obj_align(label_ip, LV_ALIGN_BOTTOM_MID, 0, -10);
}

inline void show_center_msg(const char *msg) {
  tft.drawCenterString(msg, LV_DISP_HOR_RES / 2, LV_DISP_VER_RES / 2,
                       &fonts::AsciiFont8x16);
}

inline void showCurrentTime() {
  struct tm info;
  getLocalTime(&info);
  strftime(buf, 32, "%T", &info);
  lv_label_set_text(label_time, buf);
  sprintf(buf, "%d年%d月%d日 星期%s", 1900 + info.tm_year, info.tm_mon + 1,
          info.tm_mday, WEEK_DAYS[info.tm_wday]);
  lv_label_set_text(label_date, buf);
}

inline void showClientIP() {
  sprintf(buf, "IP: %s", WiFi.localIP().toString().c_str());
  lv_label_set_text(label_ip, buf);
}

inline void updateWeatherInfo() {
  weather = get_now_weather_info(city_info["cityId"]);
  json ip_info = city_info["ip"];
  string province = ip_info["province"], city = ip_info["city"],
         district = ip_info["district"], condition = weather["CONDITIONSTEXT"],
         wind = weather["WIND"], aiq_str = weather["levelIndex"];
  int aqi = weather["aqi"];
  int temp = weather["TEMP"], winp = weather["WINP"], humi = weather["HUMI"];
  sprintf(buf, "%s %s %s %s %d℃", province.c_str(), city.c_str(),
          district.c_str(), condition.c_str(), temp);
  lv_label_set_text(label_loc, buf);
  sprintf(buf, "%s %d级 | 湿度 %d%% | 空气 %d %s", wind.c_str(), winp, humi,
          aqi, aiq_str.c_str());
  lv_label_set_text(label_wind, buf);
}

void inline autoConfigWifi() {
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin();
  show_center_msg("Start Connect WiFi!");
  for (int i = 0; WiFi.status() != WL_CONNECTED && i < 100; i++) {
    delay(100);
  }
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_MODE_APSTA);
    show_center_msg("Please Config WiFi With ESP-Touch!");
    WiFi.beginSmartConfig();
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
    }
    WiFi.stopSmartConfig();
    WiFi.mode(WIFI_MODE_STA);
  }
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

  start_lv_app();
  autoConfigWifi();
  startConfigTime();
  setupOTAConfig();
  showClientIP();
  city_info = get_cur_city_info();
  updateWeatherInfo();
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
  if (ms - check5min > 5 * 60 * 1000) {
    check5min = ms;
    updateWeatherInfo();
  }
}