#ifndef DHT_H
#define DHT_H

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app_config.h"


typedef enum {
    DHT_TYPE_DHT11 = 0,
    DHT_TYPE_DHT22,
    DHT_TYPE_AM2301,
    DHT_TYPE_SI7021
} dht_sensor_type_t;

void dht_init(gpio_num_t gpio, dht_sensor_type_t sensor_type);
bool dht_read(gpio_num_t pin, float *temperature, float *humidity);

float dht22_read_temp_c(void);
float dht22_read_rh_pct(void);

#endif // DHT_H
