#ifndef DHT_H
#define DHT_H

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


typedef enum {
    DHT_TYPE_DHT11 = 0,
    DHT_TYPE_DHT22,
    DHT_TYPE_AM2301,
    DHT_TYPE_SI7021
} dht_sensor_type_t;

void dht_init(gpio_num_t gpio, dht_sensor_type_t sensor_type);
bool dht_read(gpio_num_t pin, float *temperature, float *humidity);

#endif // DHT_H
