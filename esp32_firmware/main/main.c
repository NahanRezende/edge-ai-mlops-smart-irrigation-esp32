#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_config.h" //configs gerais
#include "time_utils.h" //util para horario de irrigação (sim ou não)
#include "forecast.h" //acesso rapido para ler o ia_params.h
#include "ia/ia_infer.h"
#include "drivers/pump.h"
#include "control/decider.h"

#include "dht.h"      // sensores/dht22/dht.h
#include "ldr.h"      // sensores/ldr/ldr.h (comentado)
#include "moisture.h" // sensores/moisture/moisture.h

#define DHT_GPIO GPIO_NUM_17  // Pino onde o DHT22 está conectado

void app_main(void) {
    // Sensores
    dht_init(DHT_GPIO, DHT_TYPE_DHT22);
    moisture_init();
    ldr_init();

    // Atuador + decisor
    pump_init();
    decider_init();

    while (true) {
        bool on = false;
        if (decider_tick(&on)) {
            pump_set(on);
        }
        vTaskDelay(pdMS_TO_TICKS(DECIDER_TICK_MS)); // 10 s por default
    }
}


// void app_main(void) {
//     float temperatura = 0.0;
//     float umidade = 0.0;

//     // Inicializa os sensores
//     // ldr_init();
//     dht_init(DHT_GPIO, DHT_TYPE_DHT22);
//     // moisture_init();

//     while (1) {
//         // Leitura do DHT22
//         if (dht_read(DHT_GPIO, &temperatura, &umidade)) {
//             printf("✅ Temperatura: %.1f°C | Umidade: %.1f%%\n", temperatura, umidade);
//         } else {
//             printf("❌ Falha na leitura ou checksum inválido\n");
//         }

//         // Leitura do LDR
//         // int lum = ldr_get_luminosidade();
//         // if (lum < 0 || lum > 100) {
//         //     printf("⚠️ LDR desconectado ou valor inválido\n");
//         // } else {
//         //     printf("💡 Luminosidade: %d%%\n", lum);
//         // }

//         // Leitura do sensor de umidade do solo
//         // int solo = moisture_get_umidade();
//         // if (solo < 0 || solo > 100) {
//         //     printf("⚠️ Sensor de umidade do solo desconectado ou valor inválido\n");
//         // } else {
//         //     printf("🌱 Umidade do solo: %d%%\n", solo);
//         // }

//         // Aguarda 2 segundos
//         vTaskDelay(pdMS_TO_TICKS(2000));
//     }
// }
