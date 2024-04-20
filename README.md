# XD-ESP32C3-AIoT
立创·实战派ESP32-C3 开发板 学习实践

### 实战派基于PlatformIO(Arduino)开发

#### 配置说明
* 配置8M Flash(默认配置是4MB)
```
board_upload.flash_size=8MB
board_build.partitions = default_8MB.csv
```

* 配置串口波特率和下载速率
```
upload_speed = 921600
monitor_speed = 115200
```

* 引入及配置TFT_eSPI
```
lib_deps =
    bodmer/TFT_eSPI @ ^2.5.43

build_flags = 
    -DUSER_SETUP_LOADED=1
    -DST7789_DRIVER=1
    -DTFT_RGB_ORDER=TFT_BGR
    -DTFT_WIDTH=240
    -DTFT_HEIGHT=320
    -DTFT_DC=6
    -DTFT_MOSI=5
    -DTFT_CS=4
    -DTFT_SCLK=3
    -DTFT_BL=2
    -DLOAD_GLCD=1
    -DLOAD_FONT2=1
    -DLOAD_FONT4=1
    -DLOAD_FONT6=1
    -DLOAD_FONT7=1
    -DLOAD_FONT8=1
    -DLOAD_GFXFF=1
    -DSMOOTH_FONT=1
    -DSPI_FREQUENCY=27000000
```

* LGFX显示接入，触摸配置
```
cfg.i2c_port = 0;    // Select I2C to use (0 or 1)
cfg.i2c_addr = 0x38; // I2C device address number
cfg.pin_sda = 0;     // pin number where SDA is connected
cfg.pin_scl = 1;     // pin number to which SCL is connected
```

