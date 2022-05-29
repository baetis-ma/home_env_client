#include "esp_timer.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static gpio_num_t dht_gpio;
int highw = 31;

static int _waitOrTimeout(uint16_t microSeconds, int level) {
    int micros_ticks = 0;
    while(gpio_get_level(dht_gpio) == level) { 
        if(micros_ticks++ > microSeconds) return 0;
        ets_delay_us(1);
    }
    return micros_ticks;
}

static void _sendStartSignal() {
    gpio_set_direction(2, GPIO_MODE_OUTPUT);
    gpio_set_direction(dht_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(dht_gpio, 0);
    //ets_delay_us(50 * 1000);
    vTaskDelay(25 / portTICK_PERIOD_MS); //18msec min
    gpio_set_level(dht_gpio, 1);
    //if(sensorType == 0) ets_delay_us(80);
    //     else ets_delay_us(40);
    ets_delay_us(40);
    gpio_pullup_en(dht_gpio);
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT);
}

static void _checkResponse() {
    /* Wait for next step ~80us*/
    if(_waitOrTimeout(180, 0) < 1000) { }
    /* Wait for next step ~80us*/
    if(_waitOrTimeout(180, 1) < 1000) { }
}

void DHT11_init(gpio_num_t gpio_num) {
    /* Wait 1 seconds to make the device pass its initial unstable status */
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    dht_gpio = gpio_num;
}

void DHT11_read() {
    uint8_t data[6] = {0,0,0,0,0,0};

    _sendStartSignal();
    _checkResponse();

    for(int i = 0; i < 48; i++) {
        if(_waitOrTimeout(350, 0) < 1000) { } 
        if(_waitOrTimeout(370, 1) >  highw) {
            data[i/8] |= (1 << (7-(i%8)));
            gpio_set_level(2, 1); 
        } 
        else gpio_set_level(2, 0);
    }
    printf("0x%02x%02x%02x%02x\n", data[0], data[1], data[2], data[3]);
    if((data[0]+ data[1]+ data[2]+ data[3])%256 == data[4]) { 
       if(sensorType == 0) {
          humidity = 10*data[0]+data[1];
          temperature = 10*data[2]+data[3]; 
       }
       else {
          humidity = 256*data[0]+data[1];
          temperature = 256*data[2]+data[3]; 
       }
    }
}
