#include <string.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp8266/spi_struct.h"
#include "esp8266/gpio_struct.h"

#define sensorType   2  //0 for dht11, 1 for dht22 or 2 for aht10

char *host_ip_addr = "192.168.0.106";
char payload[256] = "GET /index.html HTTP/1.1\nHost: 192.168.0.122\nUser-Agent: curl/7.68.0\n\0";
#include "./components/tcpclient.c"

int humidity, temperature;
#include "./components/dht.c"
#define I2C_SCL_IO           5 
#define I2C_SDA_IO           4 
#include "./components/i2c.c"
#include "./components/ath10.c"

int rate = 5, regnum = 0;;
char name[32] = "anonymous";
char *pch;
int strcnt = 0;
void hostreturn(char *rx_buffer) {
   printf("hostreturn prints --> %s\n", rx_buffer);
   pch = strtok (rx_buffer, ",");
   strcnt = 0;
   while (pch != NULL) {
      if(strcnt == 0)sscanf(pch, "%d", &regnum);
      if(strcnt == 1)strcpy(name, pch);
      if(strcnt == 2)sscanf(pch, "%d", &rate);
      printf("%d %s\n", strcnt++, pch);
      pch = strtok (NULL, ",");
   } 
   //sscanf(rx_buffer, "%s ,%d", name, &rate);
   printf("read regnum= %d name=%s rate=%d\n", regnum, name, rate);
}

void app_main()
{
    nvs_flash_init();
    initialize_wifi();
    wait_for_ip();
    //keep the i2c for aht10
    if (sensorType == 2) {   //0 for dht11, 1 for dht22 or 2 for aht10
       i2c_init();
       i2c_detect();
    }
    else {
       //keep the dht for dht22
       DHT11_init(GPIO_NUM_4); //for d1 mini boards
       //DHT11_init(GPIO_NUM_0); //for esp01 boards
    }
 
    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
    
    int cnt = 0;
    while(1) {
       //one of the other
       if (sensorType == 2) {   //0 for dht11, 1 for dht22 or 2 for aht10
          aht10_read(); }
          else DHT11_read();

       printf("hum = %02d.%d   temp = %02d.%d\n",
            humidity/10, humidity%10, temperature/10, temperature%10);

       sprintf(payload, "GET /client?%d,%s,%s,%d,%d HTTP/1.1\nHost: %s\nUser-Agent: curl/7.68.0\n",
               regnum, glob_ipadr, name, humidity, temperature, host_ip_addr);

       printf("payload --> %s", payload);
       //payload[strlen(payload)] = '\0';
       cnt++;
       vTaskDelay(5000/portTICK_RATE_MS);
    }
}
