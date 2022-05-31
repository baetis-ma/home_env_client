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
//supports three type of humidity/temperature sensors
//also for esp01 gpio has to be changed down in app_main init area
#define sensorType   0  //0 for dht11, 1 for dht22 or 2 for aht10
 //and calibration 
int humoff = 10;
int tempoff = -30;

//base station ip address
char *host_ip_addr = "192.168.0.106";
int temperature = 0;
int humidity = 0;

char payload[256] = "GET /index.html HTTP/1.1\nHost: 192.168.0.122\nUser-Agent: curl/7.68.0\n\0";
#include "./components/tcpclient.c"

#include "./components/dht.c"
#define I2C_SCL_IO           5 
#define I2C_SDA_IO           4 
#include "./components/i2c.c"
#include "./components/ath10.c"

int rate, regnum=0;
char name[32];
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
   printf("read regnum= %d name=%s rate=%d\n", regnum, name, rate);
}

void app_main()
{
    nvs_flash_init();
    initialize_wifi();
    wait_for_ip();
    rate = 10;
    strcpy(name, "anonymous");
    if (sensorType == 2) {   //0 for dht11, 1 for dht22 or 2 for aht10
       i2c_init();
       i2c_detect();
    }
    else {
       //keep the dht for dht22
       //DHT11_init(GPIO_NUM_4); //for d1 mini and wroom boards
       DHT11_init(GPIO_NUM_0); //for esp01 boards
    }
 
    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
    
    int cnt = 0;
    while(1) {
       if (sensorType == 2) {   //0 for dht11, 1 for dht22 or 2 for aht10
          aht10_read(); }
       else DHT11_read();

       printf("hum = %02d.%d   temp = %02d.%d\n",
            (humidity+humoff)/10, (humidity+humoff)%10, (temperature+tempoff)/10, (temperature+tempoff)%10);

       sprintf(payload, "GET /client?%d,%s,%s,%d,%d HTTP/1.1\nHost: %s\nUser-Agent: curl/7.68.0\n",
               regnum, glob_ipadr, name, humidity+humoff, temperature+tempoff, host_ip_addr);

       printf("payload --> %s", payload);
       cnt++;
       vTaskDelay(rate*1000/portTICK_RATE_MS);
    }
}
