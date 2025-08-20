#include "pump.h"

#define TAG "PUMP"
static bool s_on = false;

void pump_init(void) {
    #if PUMP_DRY_RUN
        ESP_LOGW(TAG, "DRY-RUN ATIVO: não configurando GPIO %d; apenas simulando.", PUMP_GPIO);
    #else
        gpio_config_t io = {
            .pin_bit_mask = 1ULL << PUMP_GPIO,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        gpio_config(&io);
        int off_level = PUMP_ACTIVE_HIGH ? 0 : 1;
        gpio_set_level(PUMP_GPIO, off_level);
        ESP_LOGI(TAG, "GPIO %d configurado (active_%s).", PUMP_GPIO, PUMP_ACTIVE_HIGH ? "HIGH" : "LOW");
    #endif
}

void pump_set(bool on) {
    s_on = on;
    #if PUMP_DRY_RUN
        ESP_LOGI(TAG, "[SIM] pump_set(%s) -> GPIO %d %s",
                on ? "ON" : "OFF",
                PUMP_GPIO,
                on ? (PUMP_ACTIVE_HIGH ? "HIGH" : "LOW") : (PUMP_ACTIVE_HIGH ? "LOW" : "HIGH"));
    #else
        int level = on ? (PUMP_ACTIVE_HIGH ? 1 : 0) : (PUMP_ACTIVE_HIGH ? 0 : 1);
        gpio_set_level(PUMP_GPIO, level);
        ESP_LOGI(TAG, "pump_set(%s) -> GPIO %d level=%d", on ? "ON" : "OFF", PUMP_GPIO, level);
    #endif
}
