#include "dht.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "DHT22"
#define MAX_TIMINGS 90  // Reduzido para evitar ruído extra

static gpio_num_t dht_gpio;
static dht_sensor_type_t dht_type;

void dht_init(gpio_num_t gpio, dht_sensor_type_t sensor_type) {
    dht_gpio = gpio;
    dht_type = sensor_type;
    gpio_reset_pin(dht_gpio);
    gpio_pullup_en(dht_gpio);  // Ativa pull-up interno
}

bool dht_read(gpio_num_t pin, float *temperature, float *humidity) {
    int data[5] = {0, 0, 0, 0, 0};
    int laststate = 1;
    int counter = 0;
    int j = 0;

    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
    vTaskDelay(pdMS_TO_TICKS(20));

    gpio_set_level(pin, 1);
    esp_rom_delay_us(80);  // Pulso de start
    gpio_set_direction(pin, GPIO_MODE_INPUT);
    // esp_rom_delay_us(20);  // Delay adicional para estabilização

    for (int i = 0; i < MAX_TIMINGS; i++) {
        counter = 0;
        while (gpio_get_level(pin) == laststate) {
            counter++;
            esp_rom_delay_us(1);
            if (counter >= 255) break;  // Timeout mais seguro
        }
        laststate = gpio_get_level(pin);
        if (counter >= 255) break;

        // Apenas a partir do 4º pulso e nos pares (representam bits)
        if ((i >= 4) && (i % 2 == 0)) {
            data[j / 8] <<= 1;
            if (counter > 40)  // Limiar ajustado para reconhecer bit "1"
                data[j / 8] |= 1;
            j++;
        }
    }

    ESP_LOGI(TAG, "Bits lidos: %d", j);
    ESP_LOGI(TAG, "Dados brutos: [%d, %d, %d, %d, %d]", data[0], data[1], data[2], data[3], data[4]);

    if (j < 40) {
        ESP_LOGE(TAG, "Dados insuficientes (%d bits)", j);
        return false;
    }

    int checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
    if (data[4] != checksum) {
        ESP_LOGE(TAG, "Checksum incorreto: recebido=%d, calculado=%d", data[4], checksum);
        return false;
    }

    *humidity = ((data[0] << 8) + data[1]) * 0.1;
    *temperature = (((data[2] & 0x7F) << 8) + data[3]) * 0.1;
    if (data[2] & 0x80) *temperature *= -1;

    return true;
}
