# This is an Espressif ESP-IDF project for the Lilygo TTGO LoRa board

It is based off the work of the ESP-IDF LoRa libraries from https://github.com/Inteform/esp32-lora-library and the SSD1309 OLED library from https://github.com/nopnop2002/esp-idf-ssd1306

You should only have to run `idf.py menuconfig` and set your pin numbers and callsigns in the "LoRa Configuration" top menu.

It does basic packet send and receives

![image](https://user-images.githubusercontent.com/473399/152606080-4939fc39-d760-4324-9357-24fb104d1400.png)

In the display above, the white section is the data being transmitted, and the black is the data being recieved. In this example, I have another board about a mile way. Notice that it said on the "adr", where it should say on the "air". This was a reception error. There is no checksuming, so it will display raw data received, even if there is an error.

Newer versions of this project use pins 33 and 34 to transmit input data to the remote. 
![image](https://github.com/bkgoodman/esp-idf-lora/assets/473399/4306c887-06e2-47d7-858e-8b6114ebf2c2)


# LoRa Board
![image](https://github.com/bkgoodman/esp-idf-lora/assets/473399/4a2b0de5-a9b2-4cde-bebb-293fd4531687)

# Settings for Lilygo TTGO V2.0 board

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
