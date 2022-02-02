#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_heap_trace.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "driver/gpio.h"

#include "lora.h"

static xQueueHandle evt_queue = NULL;
TimerHandle_t timer;

void task_tx(void *p,int txlen)
{
      //vTaskDelay(pdMS_TO_TICKS(5000));
      printf("Send packet...\n");
      lora_send_packet((uint8_t *)p,txlen);
}

int do_rx(char *p,int maxlen)
{
   int x;
   lora_receive();    // put into receive mode
   if(lora_received()) {
    x = lora_receive_packet((uint8_t *)p, maxlen-1);
    p[x] = 0;
    printf("Received: %s\n", p);
    return (x);
   }
   return(0);
}

#define EVENT_LORA_RX 0
#define EVENT_TIMER 1
static void IRAM_ATTR lora_isr_handler(void* arg) {
    printf("LoRa ISR\r\n");

    uint32_t item = EVENT_LORA_RX;
    xQueueSendFromISR(evt_queue, &item, NULL);
}

void LoRaTimer(TimerHandle_t xTimer) {
     printf("LoRa Timer\r\n");
    uint32_t item = EVENT_TIMER;
    xQueueSendFromISR(evt_queue, &item, NULL);
}

static void main_task(void* arg)
{
    char rxbuf[32];
    uint32_t evtNo;
    for(;;) {
        if(xQueueReceive(evt_queue, &evtNo, portMAX_DELAY)) {
		printf("Got Event Number %d\n",evtNo);
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

   gpio_install_isr_service(0);
   // Set Button handler 
   gpio_config_t io_conf;
   io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
   io_conf.pin_bit_mask = (1<< CONFIG_IRQ_GPIO);
   io_conf.mode = GPIO_MODE_INPUT;
   io_conf.pull_up_en = 1;
   gpio_config(&io_conf);
   gpio_isr_handler_add(CONFIG_IRQ_GPIO, lora_isr_handler, (void*) 0);


   printf("LoRa Init...\n");
   lora_init();
   printf("LoRa SetFreq...\n");
   lora_set_frequency(433e6);
   lora_set_spreading_factor(6);
   lora_set_tx_power(17);
   lora_set_bandwidth(7.8e3);
   //printf("LoRa Enable CRC...\n");
   //lora_enable_crc();
   printf("LoRa Start TX Task...\n");
   xTaskCreate(&main_task, "task_tx", 2048, NULL, 5, NULL);
   timer = xTimerCreate("Timer",10000,pdFALSE,(void *) 0,LoRaTimer);
}
