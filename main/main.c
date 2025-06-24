#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dht.h"   // sensores/dht22/dht.h
#include "ldr.h"   // sensores/ldr/ldr.h

#define DHT_GPIO GPIO_NUM_5  // Pino onde o DHT22 está conectado

void app_main(void) {
    float temperatura = 0.0;
    float umidade = 0.0;

    // Inicializa os sensores
    dht_init(DHT_GPIO);
    // ldr_init();

    while (1) {
        // Leitura do DHT22
        if (dht_ler(&temperatura, &umidade)) {
            printf("📡 DHT22 - Temperatura: %.1f°C | Umidade: %.1f%%\n", temperatura, umidade);
        } else {
            printf("❌ Erro ao ler DHT22\n");
        }

        // Leitura do LDR
        // int lum = ldr_get_luminosidade();
        // if (lum < 0 || lum > 100) {
        //     printf("⚠️ LDR desconectado ou valor inválido\n");
        // } else {
        //     printf("💡 Luminosidade: %d%%\n", lum);
        // }

        // Aguarda 2 segundos
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
