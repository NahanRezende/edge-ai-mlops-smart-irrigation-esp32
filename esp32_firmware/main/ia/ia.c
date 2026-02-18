#include <math.h>
#include "ia_infer.h"
#include "ia_params.h"
#include "app_config.h"

#include "esp_log.h"
static const char *IATAG = "IA";

static inline float safe_div(float num, float den) {
    return den == 0.0f ? 0.0f : num / den;
}

static inline float clampf(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static inline float sigmoidf(float x) {
    if (x > 16.0f)  x = 16.0f;
    if (x < -16.0f) x = -16.0f;
    return 1.0f / (1.0f + expf(-x));
}

enum { N = N_FEATS };
_Static_assert(sizeof(SCALER_MEAN) / sizeof(float) == N, "SCALER_MEAN tamanho incorreto");
_Static_assert(sizeof(SCALER_SCALE) / sizeof(float) == N, "SCALER_SCALE tamanho incorreto");
_Static_assert(sizeof(W) / sizeof(float) == N, "W tamanho incorreto");
_Static_assert(sizeof(B) / sizeof(float) == 1, "B deve ter 1 elemento");

static inline float dotN(const float *a, const float *b) {
    float s = 0.0f;
    for (int i = 0; i < N; ++i) s += a[i] * b[i];
    return s;
}

static inline float soil_wet_01(float soil_pct) {
    float denom = (100.0f - SOIL_REF_UMIDO_PCT);
    float wet = (denom > 0.0f) ? ((soil_pct - SOIL_REF_UMIDO_PCT) / denom) : 0.0f;
    return clampf(wet, 0.0f, 1.0f);
}

float ia_predict_proba(const float in[N]) {
    float x_norm[N];
    for (int i = 0; i < N; ++i) {
        x_norm[i] = safe_div(in[i] - SCALER_MEAN[i], SCALER_SCALE[i]);
    }

    float soil = in[0];

    float logit_before = dotN(W, x_norm) + B[0];

    float wet = soil_wet_01(soil);
    float off = -IA_SOIL_LOGIT_PENALTY * wet;

    float logit_after = logit_before + off;

    ESP_LOGI(IATAG,
             "soil=%.1f wet=%.2f off=%.2f logit_before=%.2f logit_after=%.2f",
             soil, wet, off,
             (double)logit_before, (double)logit_after);

    return sigmoidf(logit_after);
}

bool ia_should_irrigate(const float in[N], float threshold) {
    return ia_predict_proba(in) > threshold;
}

bool ia_should_irrigate_hysteresis(const float in[N], float low_th, float high_th, bool previous_state) {
    float p = ia_predict_proba(in);
    if (previous_state) return p > low_th;
    else                return p > high_th;
}
