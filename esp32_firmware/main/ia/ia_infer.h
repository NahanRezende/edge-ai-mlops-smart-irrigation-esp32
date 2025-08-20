#pragma once
#include <stdbool.h>

// Este header expõe a API de inferência.
// A ORDEM/UNIDADES das features vem do ia_params.h (comentário FEATURE_ORDER).

#ifdef __cplusplus
extern "C" {
#endif

// Puxe N_FEATS do header gerado pelo treino
#include "ia_params.h"  // define N_FEATS, SCALER_*, W, B, e (opcional) FEATURE_ORDER

// Probabilidade (sigmóide do logit). 'in' deve ter N_FEATS elementos.
float ia_predict_proba(const float in[N_FEATS]);

// Decisão com limiar simples (ex.: 0.5f)
bool ia_should_irrigate(const float in[N_FEATS], float threshold);

// Decisão com histerese (evita liga/desliga rápido)
bool ia_should_irrigate_hysteresis(const float in[N_FEATS],
                                   float low_th, float high_th,
                                   bool previous_state);

#ifdef __cplusplus
}
#endif
