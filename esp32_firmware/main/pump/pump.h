#pragma once

#include <stdbool.h>
#include "app_config.h"
#include "driver/gpio.h"
#include "esp_log.h"


void pump_init(void);
void pump_set(bool on);
