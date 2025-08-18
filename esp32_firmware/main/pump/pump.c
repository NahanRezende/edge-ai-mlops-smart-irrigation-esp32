#include "pump.h"

void pump_init(void) {
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << PUMP_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io);
    pump_set(false);
}

void pump_set(bool on) {
#if PUMP_ACTIVE_HIGH
    gpio_set_level(PUMP_GPIO, on ? 1 : 0);
#else
    gpio_set_level(PUMP_GPIO, on ? 0 : 1);
#endif
}
