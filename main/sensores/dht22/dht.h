#ifndef __DHT_H__
#define __DHT_H__

#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DHT_TYPE_DHT11 = 0,
    DHT_TYPE_DHT22 = 1
} dht_sensor_type_t;

/**
 * @brief Inicializa o pino usado pelo sensor DHT.
 * 
 * @param pin GPIO que será usado para comunicação com o DHT
 * @param type Tipo de sensor (DHT11 ou DHT22)
 */
void dht_init(gpio_num_t pin, dht_sensor_type_t type);

/**
 * @brief Lê os dados do sensor DHT.
 *
 * @param type Tipo de sensor (DHT11 ou DHT22)
 * @param pin GPIO usado para comunicação com o DHT
 * @param humidity ponteiro para armazenar umidade (em float)
 * @param temperature ponteiro para armazenar temperatura (em float)
 * @return ESP_OK se leitura for bem-sucedida, ESP_FAIL caso contrário
 */
esp_err_t dht_read_float_data(dht_sensor_type_t type, gpio_num_t pin, float *humidity, float *temperature);

#ifdef __cplusplus
}
#endif

#endif // __DHT_H__
