#include "dht.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "DHT22"
#define MAX_TIMINGS 85

static gpio_num_t dht_gpio;
static dht_sensor_type_t dht_type;

void dht_init(gpio_num_t gpio, dht_sensor_type_t sensor_type) {
    dht_gpio = gpio;
    dht_type = sensor_type;
    gpio_reset_pin(dht_gpio);
}

esp_err_t dht_read_data(dht_sensor_type_t type, gpio_num_t pin, int16_t *humidity, int16_t *temperature) {
    int data[5] = {0, 0, 0, 0, 0};
    int laststate = 1;
    int counter = 0;
    int j = 0;

    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
    vTaskDelay(pdMS_TO_TICKS(20));

    gpio_set_level(pin, 1);
    esp_rom_delay_us(40);
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    for (int i = 0; i < MAX_TIMINGS; i++) {
        counter = 0;
        while (gpio_get_level(pin) == laststate) {
            counter++;
            esp_rom_delay_us(1);
            if (counter == 255) break;
        }
        laststate = gpio_get_level(pin);
        if (counter == 255) break;

        if ((i >= 4) && (i % 2 == 0)) {
            data[j / 8] <<= 1;
            if (counter > 50) data[j / 8] |= 1;
            j++;
        }
    }

    if (j < 40) {
        ESP_LOGE(TAG, "Dados insuficientes (%d bits)", j);
        return ESP_FAIL;
    }

    int checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
    if (data[4] != checksum) {
        ESP_LOGE(TAG, "Checksum incorreto: recebido=%d, calculado=%d", data[4], checksum);
        return ESP_FAIL;
    }

    if (type == DHT_TYPE_DHT22) {
        *humidity = (data[0] << 8) + data[1];
        *temperature = ((data[2] & 0x7F) << 8) + data[3];
        if (data[2] & 0x80) *temperature = -*temperature;
    } else {
        // DHT11 fallback
        *humidity = data[0] * 10;
        *temperature = data[2] * 10;
    }

    return ESP_OK;
}

esp_err_t dht_read_float_data(dht_sensor_type_t sensor_type, gpio_num_t pin,
                              float *humidity, float *temperature) {
    if (!humidity || !temperature) return ESP_ERR_INVALID_ARG;

    int16_t h, t;
    esp_err_t res = dht_read_data(sensor_type, pin, &h, &t);
    if (res != ESP_OK) return res;

    *humidity = h / 10.0;
    *temperature = t / 10.0;

    return ESP_OK;
}
