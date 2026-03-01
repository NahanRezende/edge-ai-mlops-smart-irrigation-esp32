#pragma once
#include <stdbool.h>

// API de inferência. Ordem/unidade das features: ia_params.h (FEATURE_ORDER).

#ifdef __cplusplus
extern "C" {
#endif

// vem do header gerado no treino
#include "ia_params.h"  // define N_FEATS, SCALER_*, W, B, e (opcional) FEATURE_ORDER

// probabilidade (sigmóide do logit). 'in' tem N_FEATS elementos.
float ia_predict_proba(const float in[N_FEATS]);

// decisão com limiar simples (ex.: 0.5f)
bool ia_should_irrigate(const float in[N_FEATS], float threshold);

// decisão com histerese (evita liga/desliga rápido)
bool ia_should_irrigate_hysteresis(const float in[N_FEATS],
                                   float low_th, float high_th,
                                   bool previous_state);

#ifdef __cplusplus
}
#endif
