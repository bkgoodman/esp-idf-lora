# This is an Espressif ESP-IDF project for the Lilygo TTGO LoRa board

It is based off the work of the ESP-IDF LoRa libraries from https://github.com/Inteform/esp32-lora-library and the SSD1309 OLED library from https://github.com/nopnop2002/esp-idf-ssd1306

You should only have to run `idf.py menuconfig` and set your pin numbers and callsigns in the "LoRa Configuration" top menu.

It does basic packet send and receives

![image](https://user-images.githubusercontent.com/473399/152606080-4939fc39-d760-4324-9357-24fb104d1400.png)

# Settings for Lilygo board

```
CONFIG_CS_GPIO=18
CONFIG_RST_GPIO=14
CONFIG_MISO_GPIO=19
CONFIG_MOSI_GPIO=27
CONFIG_IRQ_GPIO=26
CONFIG_SCK_GPIO=5
CONFIG_CALLSIGN="K1BKG"
CONFIG_OLED_I2C_MASTER_SCL=21
CONFIG_OLED_I2C_MASTER_SDA=22
CONFIG_OLED_I2C_MASTER_PORT_NUM=1
```
