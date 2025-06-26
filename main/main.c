#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dht.h"      // sensores/dht22/dht.h
#include "ldr.h"      // sensores/ldr/ldr.h (comentado)
#include "moisture.h" // sensores/moisture/moisture.h

#define DHT_GPIO GPIO_NUM_21  // Pino onde o DHT22 está conectado

void app_main(void) {
    float temperatura = 0.0;
    float umidade = 0.0;

    // Inicializa os sensores
    dht_init(DHT_GPIO, DHT_TYPE_DHT22);
    // ldr_init();
    // moisture_init();

    while (1) {
        // Leitura do DHT22
        float temp, hum;
        if (dht_read_float_data(DHT_TYPE_DHT22, GPIO_NUM_21, &hum, &temp) == ESP_OK) {
            printf("Temp: %.1f°C, Hum: %.1f%%\n", temp, hum);
        } else {
            printf("Erro na leitura do DHT22\n");
        }

        // Leitura do LDR
        // int lum = ldr_get_luminosidade();
        // if (lum < 0 || lum > 100) {
        //     printf("⚠️ LDR desconectado ou valor inválido\n");
        // } else {
        //     printf("💡 Luminosidade: %d%%\n", lum);
        // }

        // Leitura do sensor de umidade do solo
        // int solo = moisture_get_umidade();
        // if (solo < 0 || solo > 100) {
        //     printf("⚠️ Sensor de umidade do solo desconectado ou valor inválido\n");
        // } else {
        //     printf("🌱 Umidade do solo: %d%%\n", solo);
        // }

        // Aguarda 2 segundos
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
