#pragma once

#include <math.h>
#include <time.h>
#include "ia_params.h"  // FORECAST_* vêm do header gerado

float forecast_chuva24h_now(void);  // retorna mm nas próximas 24h, do header embutido
