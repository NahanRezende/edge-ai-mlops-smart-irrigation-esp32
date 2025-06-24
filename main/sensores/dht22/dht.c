#include "dht.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"

#define TAG "DHT22"
#define MAX_TIMINGS 85

static gpio_num_t dht_gpio;

void dht_init(gpio_num_t gpio) {
    dht_gpio = gpio;
    gpio_reset_pin(gpio);
}

bool dht_ler(float *temperatura, float *umidade) {
    int data[5] = {0, 0, 0, 0, 0};
    int laststate = 1;
    int j = 0, counter;

    gpio_set_direction(dht_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(dht_gpio, 0);
    vTaskDelay(pdMS_TO_TICKS(20));

    gpio_set_level(dht_gpio, 1);
    esp_rom_delay_us(40);
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT);

    for (int i = 0; i < MAX_TIMINGS; i++) {
        counter = 0;
        while (gpio_get_level(dht_gpio) == laststate) {
            counter++;
            esp_rom_delay_us(1);
            if (counter == 255) break;
        }

        laststate = gpio_get_level(dht_gpio);

        if (counter == 255) break;

        if ((i >= 4) && (i % 2 == 0)) {
            data[j / 8] <<= 1;
            if (counter > 50)
                data[j / 8] |= 1;
            j++;
        }
    }

    ESP_LOGI(TAG, "🧩 Bits lidos: %d", j);
    ESP_LOGI(TAG, "🧾 Dados brutos: [%d, %d, %d, %d, %d]", data[0], data[1], data[2], data[3], data[4]);
    ESP_LOGI(TAG, "🔢 Checksum recebido: %d | Calculado: %d", data[4], ((data[0] + data[1] + data[2] + data[3]) & 0xFF));

    if (j >= 40 && data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        *umidade = ((data[0] << 8) + data[1]) * 0.1;
        *temperatura = (((data[2] & 0x7F) << 8) + data[3]) * 0.1;
        if (data[2] & 0x80) *temperatura *= -1;
        return true;
    }

    ESP_LOGE(TAG, "❌ Falha no checksum ou dados inválidos");
    return false;
}
