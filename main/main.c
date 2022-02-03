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

#include "lora.h"

#define TAG "BKGLoRa"
static xQueueHandle evt_queue = NULL;
TimerHandle_t timer;

void task_tx(void *p,int txlen)
{
      ESP_LOGI(TAG,"Send packet...");
      lora_send_packet((uint8_t *)p,txlen);
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
    ESP_LOGI(TAG,"LoRa ISR\r");

    uint32_t item = EVENT_LORA_RX;
    xQueueSendFromISR(evt_queue, &item, NULL);
}

void LoRaTimer(TimerHandle_t xTimer) {
     ESP_LOGI(TAG,"LoRa Timer 0x%d\r",(uint32_t) xTimer);
    uint32_t item = EVENT_TIMER;
    xQueueSendFromISR(evt_queue, &item, NULL);
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
				snprintf(rxbuf,sizeof(rxbuf),"%s is on the air",CONFIG_CALLSIGN);
				task_tx(rxbuf,strlen(rxbuf));
				break;
		}
	}
    }
}

void app_main()
{
   evt_queue = xQueueCreate(10, sizeof(uint32_t));

   gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
   // Set Button handler 
   gpio_config_t io_conf;
   io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
   io_conf.pin_bit_mask = (1<< CONFIG_IRQ_GPIO);
   io_conf.mode = GPIO_MODE_INPUT;
   io_conf.pull_up_en = 1;
   gpio_config(&io_conf);
   gpio_isr_handler_add(CONFIG_IRQ_GPIO, lora_isr_handler, (void*) 0);

   ESP_LOGI(TAG,"LoRa Init...");
   lora_init();
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
   xTaskCreate(&main_task, "task_tx", 8192, NULL, 5, NULL);
   ESP_LOGI(TAG,"Main Created...");
   //task_tx("Testing",7);
   timer = xTimerCreate("Timer",(10000 / portTICK_PERIOD_MS ),pdTRUE,(void *) 0,LoRaTimer);
   ESP_LOGI(TAG,"Timer Created");
   xTimerStart(timer,0);
   ESP_LOGI(TAG,"Timer Started");
}
