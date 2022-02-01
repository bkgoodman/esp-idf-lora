#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lora.h"

void task_tx(void *p)
{
   for(;;) {
      printf("Sleep...\n");
      vTaskDelay(pdMS_TO_TICKS(5000));
      printf("Send packet...\n");
      lora_send_packet((uint8_t*)"K1BKG Is On the air", 19);
      printf("packet sent...\n");
   }
}

void task_rx(void *p)
{
   int x;
   unsigned char buf[32];
   for(;;) {
      lora_receive();    // put into receive mode
      while(lora_received()) {
         x = lora_receive_packet(buf, sizeof(buf));
         buf[x] = 0;
         printf("Received: %s\n", buf);
         lora_receive();
      }
      vTaskDelay(1);
   }
}

void app_main()
{
   printf("LoRa Init...\n");
   lora_init();
   printf("LoRa SetFreq...\n");
   lora_set_frequency(433e6);
   printf("LoRa Enable CRC...\n");
   lora_enable_crc();
   printf("LoRa Start TX Task...\n");
   xTaskCreate(&task_tx, "task_tx", 2048, NULL, 5, NULL);
}
