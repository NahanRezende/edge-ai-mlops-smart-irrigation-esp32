#include <math.h>
#include "ia_infer.h"
#include "ia_params.h"  // Gerado pelo Python

static inline float safe_div(float num, float den) {
    return den == 0.0f ? 0.0f : num / den;
}

static inline float sigmoidf(float x) {
    if (x > 16.0f)  x = 16.0f;
    if (x < -16.0f) x = -16.0f;
    return 1.0f / (1.0f + expf(-x));
}

/* ---- Garantias de compatibilidade com o header ---- */
enum { N = N_FEATS };
_Static_assert(sizeof(SCALER_MEAN)/sizeof(float) == N, "SCALER_MEAN tamanho incorreto");
_Static_assert(sizeof(SCALER_SCALE)/sizeof(float) == N, "SCALER_SCALE tamanho incorreto");
_Static_assert(sizeof(W)/sizeof(float)            == N, "W tamanho incorreto");
_Static_assert(sizeof(B)/sizeof(float)            == 1, "B deve ter 1 elemento");

/* ---- Produto escalar genérico ---- */
static inline float dotN(const float *a, const float *b) {
    float s = 0.0f;
    for (int i = 0; i < N; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Inferência ---- */
float ia_predict_proba(const float in[N]) {
    float x_norm[N];
    for (int i = 0; i < N; ++i) {
        x_norm[i] = safe_div(in[i] - SCALER_MEAN[i], SCALER_SCALE[i]);
    }
    float logit = dotN(W, x_norm) + B[0];
    return sigmoidf(logit);
}

bool ia_should_irrigate(const float in[N], float threshold) {
    return ia_predict_proba(in) > threshold;
}

bool ia_should_irrigate_hysteresis(const float in[N], float low_th, float high_th, bool previous_state) {
    float p = ia_predict_proba(in);
    if (previous_state) return p > low_th;   // desligar só se cair bem abaixo
    else                return p > high_th;  // ligar só se subir bem acima
}
