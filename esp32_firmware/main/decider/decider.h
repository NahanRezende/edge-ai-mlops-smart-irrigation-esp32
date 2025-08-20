#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include "esp_timer.h"

#include "app_config.h"
#include "time_utils.h"
#include "forecast.h"
#include "ia_infer.h"

void decider_init(void);
bool decider_tick(bool *out_on);  // true: decisão válida; *out_on = liga/desliga
