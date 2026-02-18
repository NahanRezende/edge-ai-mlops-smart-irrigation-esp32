#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_config.h"   // onde eu mudo pinos e thresholds
#include "time_utils.h"   // helpers de hora local, janela quente e anti-chattering
#include "forecast.h"     // lê dados/curvas do ia_params.h e chuva prevista
#include "ia_infer.h"     // roda o modelo e devolve probabilidade/score
#include "pump.h"         // abstração do relé (pump_init, pump_set)
#include "decider.h"      // regra final de irrigação (on/off)
#include "wifi.h"         // chamando o wifi para conectar
#include "ota_server.h"

#include "dht.h"      // sensores/dht22/dht.h
#include "ldr.h"      // sensores/ldr/ldr.h (comentado)
#include "moisture.h" // sensores/moisture/moisture.h


void app_main(void) {
    esp_log_level_set("DECIDER", ESP_LOG_INFO);
    esp_log_level_set("SOIL", ESP_LOG_INFO);
    // esp_log_level_set("DHT22", ESP_LOG_INFO);
    
    // Wifi
    ESP_LOGI("app","init Wi-Fi");
    wifi_init_sta();
    esp_wifi_set_ps(WIFI_PS_NONE);
    if(wifi_wait_connected(15000)){
        ESP_LOGI("app","Wi-Fi OK (IP obtido)");
    } else {
        ESP_LOGW("app","sem IP após 15s; seguindo mesmo assim");
    }

    // Start OTA server
    ESP_LOGI("app","iniciando OTA server");
    ota_server_start();

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
//         int lum = ldr_get_luminosidade();
//         if (lum < 0 || lum > 100) {
//             printf("⚠️ LDR desconectado ou valor inválido\n");
//         } else {
//             printf("💡 Luminosidade: %d%%\n", lum);
//         }

//         // Leitura do sensor de umidade do solo
//         int solo = moisture_get_umidade();
//         if (solo < 0 || solo > 100) {
//             printf("⚠️ Sensor de umidade do solo desconectado ou valor inválido\n");
//         } else {
//             printf("🌱 Umidade do solo: %d%%\n", solo);
//         }

//         // Aguarda 2 segundos
//         vTaskDelay(pdMS_TO_TICKS(2000));
//     }
// }
