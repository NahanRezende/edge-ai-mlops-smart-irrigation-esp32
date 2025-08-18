#pragma once

#include <stdbool.h>
#include "app_config.h"
#include "gpio.h"


void pump_init(void);
void pump_set(bool on);
