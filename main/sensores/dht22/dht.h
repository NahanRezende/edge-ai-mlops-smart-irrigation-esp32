#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include "driver/gpio.h"
#include <stdbool.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_rom_sys.h"

#define MAX_TIMINGS 85


void dht_init(gpio_num_t gpio);
bool dht_ler(float *temperatura, float *umidade);

#endif
