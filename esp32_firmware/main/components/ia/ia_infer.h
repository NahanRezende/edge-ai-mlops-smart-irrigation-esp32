#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Entradas: {temp_max, temp_min, chuva_mm} (valores BRUTOS)
float ia_predict_proba(const float in[3]);

// Decide irrigação com limiar (ex.: 0.5f)
bool ia_should_irrigate(const float in[3], float threshold);

// Histerese para evitar liga/desliga
bool ia_should_irrigate_hysteresis(const float in[3], float low_th, float high_th, bool previous_state);

#ifdef __cplusplus
}
#endif
