#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_heap_trace.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "driver/gpio.h"

#include "ssd1306.h"
#include "font8x8_basic.h"

#include "lora.h"

#if 0
#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)
#define OLED_I2C_MASTER_SCL_IO (CONFIG_OLED_I2C_MASTER_SCL)               /*!< gpio number for I2C master clock */
#define OLED_I2C_MASTER_SDA_IO (CONFIG_OLED_I2C_MASTER_SDA)               /*!< gpio number for I2C master data  */
#define OLED_I2C_MASTER_NUM (I2C_NUMBER(CONFIG_OLED_I2C_MASTER_PORT_NUM)) /*!< I2C port number for master dev */
#endif

#define TAG "BKGLoRa"
static xQueueHandle evt_queue = NULL;
TimerHandle_t timer;

void task_tx(void *p,int txlen)
{
      ESP_LOGI(TAG,"Send packet...");
      lora_send_packet((uint8_t *)p,txlen);
	lora_receive();    // put into receive mode
      ESP_LOGI(TAG,"Sent.");
}

int do_rx(char *p,int maxlen)
{
   int x;
   lora_receive();    // put into receive mode
   if(lora_received()) {
    x = lora_receive_packet((uint8_t *)p, maxlen-1);
    p[x] = 0;
    ESP_LOGI(TAG,"Received: %s", p);
    return (x);
   }
   return(0);
}

#define EVENT_LORA_RX 0
#define EVENT_TIMER 1
static void IRAM_ATTR lora_isr_handler(void* arg) {
    uint32_t item = EVENT_LORA_RX;
    xQueueSendFromISR(evt_queue, &item, NULL);
}

void LoRaTimer(TimerHandle_t xTimer) {
     ESP_LOGI(TAG,"LoRa Timer 0x%d\r",(uint32_t) xTimer);
    uint32_t item = EVENT_TIMER;
    xQueueSendFromISR(evt_queue, &item, NULL);
}

unsigned char buf[55];
void task_rx(void *p)
{
   int x;
   for(;;) {
      lora_receive();    // put into receive mode
      while(lora_received()) {
	      printf("RCVd...\n");
         x = lora_receive_packet(buf, sizeof(buf));
	 ESP_LOGW(TAG,"Post-Receive");
         buf[x] = 0;
         printf("Received: %s\n", buf);
         lora_receive();
      vTaskDelay(100 / portTICK_PERIOD_MS);
      }
      vTaskDelay(100 / portTICK_PERIOD_MS);
   }
}


static void main_task(void* arg)
{
    char rxbuf[32];
    uint32_t evtNo;
    ESP_LOGI(TAG,"Main Task Started");
    for(;;) {
        if(xQueueReceive(evt_queue, &evtNo, portMAX_DELAY)) {
		ESP_LOGI(TAG,"Got Event Number %d",evtNo);
		switch (evtNo) {
			case EVENT_LORA_RX:
				do_rx(rxbuf,sizeof(rxbuf));
				break;
			case EVENT_TIMER:
   ESP_LOGI(TAG,"Start Rx");
				do_rx(rxbuf,sizeof(rxbuf));
	   ESP_LOGI(TAG,"Ended Received");
				snprintf(rxbuf,sizeof(rxbuf),"%s is on the air",CONFIG_CALLSIGN);
				task_tx(rxbuf,strlen(rxbuf));
				break;
		}
	}
    }
}

void app_main()
{
	SSD1306_t dev;
	int center, top, bottom;
	char lineChar[20];
	ESP_LOGI(TAG,"OLED SDA %d OLED SCL %d MASTER_NUM %d",CONFIG_OLED_I2C_MASTER_SDA,CONFIG_OLED_I2C_MASTER_SCL,CONFIG_OLED_I2C_MASTER_PORT_NUM);
	//i2c_master_init(&dev, CONFIG_OLED_I2C_MASTER_SDA, CONFIG_OLED_I2C_MASTER_SCL, -1);
	i2c_master_init(&dev, 21, 22, -1);
 	ESP_LOGI(TAG, "Panel is 128x64");
	ssd1306_init(&dev, 128, 64);
	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
	top = 2;
	center = 3;
	bottom = 8;
	ssd1306_display_text(&dev, 0, "SSD1306 128x64", 14, false);
	ssd1306_display_text(&dev, 1, "ABCDEFGHIJKLMNOP", 16, false);
	ssd1306_display_text(&dev, 2, "abcdefghijklmnop",16, false);
	ssd1306_display_text(&dev, 3, "Hello World!!", 13, false);
	ssd1306_clear_line(&dev, 4, true);
	ssd1306_clear_line(&dev, 5, true);
	ssd1306_clear_line(&dev, 6, true);
	ssd1306_clear_line(&dev, 7, true);
	ssd1306_display_text(&dev, 4, "SSD1306 128x64", 14, true);
	ssd1306_display_text(&dev, 5, "ABCDEFGHIJKLMNOP", 16, true);
	ssd1306_display_text(&dev, 6, "abcdefghijklmnop",16, true);
	ssd1306_display_text(&dev, 7, "Hello World!!", 13, true);
   evt_queue = xQueueCreate(10, sizeof(uint32_t));

   gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
   // Set Button handler 
   gpio_config_t io_conf;
   io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
   io_conf.pin_bit_mask = (1<< CONFIG_IRQ_GPIO);
   io_conf.mode = GPIO_MODE_INPUT;
   io_conf.pull_up_en = 1;
   gpio_config(&io_conf);
   gpio_isr_handler_add(CONFIG_IRQ_GPIO, lora_isr_handler, (void*) 0);

   int err = lora_init();
   ESP_LOGI(TAG,"LoRa Init: %d",err);

   ESP_LOGI(TAG,"LoRa Check...");
   lora_initialized();
   ESP_LOGI(TAG,"LoRa SetFreq...");
   lora_set_frequency(433775000);
   lora_set_spreading_factor(12);
   lora_set_tx_power(17);
   lora_set_bandwidth(125000);
   lora_set_coding_rate(8);
   //ESP_LOGI(TAG,"LoRa Enable CRC...");
   //lora_enable_crc();
   ESP_LOGI(TAG,"LoRa Start TX Task...");
   //xTaskCreate(&main_task, "task_tx", 8192, NULL, 5, NULL);
   //ESP_LOGI(TAG,"Main Created...");
   xTaskCreate(&task_rx, "rx_tx", 8192, NULL, 5, NULL);
   ESP_LOGI(TAG,"Rx Running...");
  

   //ESP_LOGI(TAG,"OLED SDA %d OLED SCL %d MASTER_NUM %d",OLED_I2C_MASTER_SDA_IO,OLED_I2C_MASTER_SCL_IO,OLED_I2C_MASTER_NUM);

   //task_tx("Testing",7);
   //timer = xTimerCreate("Timer",(10000 / portTICK_PERIOD_MS ),pdTRUE,(void *) 0,LoRaTimer);
   //ESP_LOGI(TAG,"Timer Created");
   //xTimerStart(timer,0);
   //ESP_LOGI(TAG,"Timer Started");

}
