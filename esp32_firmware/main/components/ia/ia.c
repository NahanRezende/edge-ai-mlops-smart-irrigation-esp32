#include <math.h>
#include "ia/logreg_infer.h"
#include "ia/logreg_params.h"  // ← gerado pelo Python

static inline float safe_div(float num, float den) {
    return den == 0.0f ? 0.0f : num / den;
}

static inline float sigmoidf(float x) {
    if (x > 16.0f)  x = 16.0f;
    if (x < -16.0f) x = -16.0f;
    return 1.0f / (1.0f + expf(-x));
}

static inline float dot3(const float a[3], const float b[3]) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

float ia_predict_proba(const float in[3]) {
    float x_norm[3];
    for (int i = 0; i < 3; ++i) {
        x_norm[i] = safe_div(in[i] - SCALER_MEAN[i], SCALER_SCALE[i]);
    }
    float logit = dot3(W, x_norm) + B[0];
    return sigmoidf(logit);
}

bool ia_should_irrigate(const float in[3], float threshold) {
    return ia_predict_proba(in) > threshold;
}

bool ia_should_irrigate_hysteresis(const float in[3], float low_th, float high_th, bool previous_state) {
    float p = ia_predict_proba(in);
    if (previous_state) return p > low_th;   // desligar só se cair bem abaixo
    else                return p > high_th;  // ligar só se subir bem acima
}
