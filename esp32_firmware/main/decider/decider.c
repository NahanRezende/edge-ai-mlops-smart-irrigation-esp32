#include "decider.h"
#include "moisture.h"
#include "dht.h"
#include "ldr.h"
#include "esp_log.h"
#include "ia_params.h"   // só pra log de debug do modelo

#ifndef DECIDER_LOG_DATA
#define DECIDER_LOG_DATA 1
#endif

static const char *TAG = "DECIDER";

// Estado interno (histerese + proteção contra liga/desliga rápido)
static bool     s_prev = false;
static uint64_t s_last_switch_ms = 0;

static inline uint64_t now_ms(void) {
    return (uint64_t)(esp_timer_get_time() / 1000ULL);
}

static inline float clampf(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static void build_features(float in[]) {
    float soil = (float)READ_SOIL_PCT();
    float tc   = (float)READ_TEMP_C();
    float rh   = (float)READ_RH_PCT();
    float ch24 = forecast_chuva24h_now();

    #if N_FEATS == 3
        in[0] = soil; in[1] = tc; in[2] = rh;
    #elif N_FEATS == 4
        in[0] = soil; in[1] = tc; in[2] = rh; in[3] = ch24;
    #else
        # error "N_FEATS inesperado — ajuste build_features()"
    #endif
}

void decider_init(void) {
    s_prev = false;
    s_last_switch_ms = now_ms();

    #if DECIDER_LOG_DATA
    ESP_LOGI(TAG,
             "config: SOLO_CRIT=%.1f SOLO_OK=%.1f SOLO_REF=%.1f "
             "CHUVA_BLOQ=%.1f IA_TH=[%.2f..%.2f] ADJ_MAX=%.2f "
             "MIN_ON=%ums MIN_OFF=%ums TICK=%ums",
             (float)SOLO_CRITICO_PCT, (float)SOLO_OK_PCT, (float)SOIL_REF_UMIDO_PCT,
             (float)CHUVA_BLOQUEIO_MM24, (float)IA_LOW_TH, (float)IA_HIGH_TH, (float)IA_TH_ADJ_MAX,
             (unsigned)MIN_ON_MS, (unsigned)MIN_OFF_MS, (unsigned)DECIDER_TICK_MS);

    // Só pra conferir se o header da IA está batendo com o que foi gerado no Python
    ESP_LOGI(TAG,
             "model: mean=[%.1f %.1f %.1f %.1f] scale=[%.1f %.1f %.1f %.1f] "
             "W=[%.2f %.2f %.2f %.2f] B=%.2f",
             (float)SCALER_MEAN[0], (float)SCALER_MEAN[1], (float)SCALER_MEAN[2], (float)SCALER_MEAN[3],
             (float)SCALER_SCALE[0], (float)SCALER_SCALE[1], (float)SCALER_SCALE[2], (float)SCALER_SCALE[3],
             (float)W[0], (float)W[1], (float)W[2], (float)W[3],
             (float)B[0]);
    #endif
}

bool decider_tick(bool *out_on) {
    if (!out_on) return false;

    float soil = (float)READ_SOIL_PCT();
    float tc   = (float)READ_TEMP_C();
    float rh   = (float)READ_RH_PCT();
    float ch24 = forecast_chuva24h_now();

    // Garante faixa válida (se sensor der leitura doida não quebra a lógica)
    soil = clampf(soil, 0.0f, 100.0f);
    rh   = clampf(rh,   0.0f, 100.0f);

    // --- Regras fortes que já existiam ---

    // Evita irrigar no horário mais quente se o solo ainda estiver ok
    if (is_hot_window() && soil >= SOLO_CRITICO_PCT) {
        if (s_prev) {
            uint64_t n = now_ms();
            if (n - s_last_switch_ms < MIN_ON_MS) {
                *out_on = true;
                return true;
            }
            s_prev = false;
            s_last_switch_ms = n;
        }

        *out_on = false;

        #if DECIDER_LOG_DATA
        ESP_LOGI(TAG, "OFF (hot_window) SOIL=%.1f TEMP=%.1f RH=%.1f RAIN24=%.1f",
                 soil, tc, rh, ch24);
        #endif
        return true;
    }

    // Se vai chover bem e o solo ainda está ok → adia irrigação
    if ((ch24 >= CHUVA_BLOQUEIO_MM24) && (soil >= SOLO_OK_PCT)) {
        if (s_prev) {
            uint64_t n = now_ms();
            if (n - s_last_switch_ms < MIN_ON_MS) {
                *out_on = true;
                return true;
            }
            s_prev = false;
            s_last_switch_ms = n;
        }

        *out_on = false;

        #if DECIDER_LOG_DATA
        ESP_LOGI(TAG, "OFF (rain_block) SOIL=%.1f TEMP=%.1f RH=%.1f RAIN24=%.1f",
                 soil, tc, rh, ch24);
        #endif
        return true;
    }

    #if APP_USE_LDR_GATE
        float ldr_mv = (float)READ_LDR_MV();
        (void)ldr_mv;
    #endif

    // --- IA com threshold adaptativo pelo solo ---

    float in[N_FEATS];
    build_features(in);

    // urgência: 0 = solo úmido, 1 = solo crítico
    float denom = (SOIL_REF_UMIDO_PCT - SOLO_CRITICO_PCT);
    float urg = (denom > 0.0f) ? ((SOIL_REF_UMIDO_PCT - soil) / denom) : 1.0f;
    urg = clampf(urg, 0.0f, 1.0f);

    // Solo úmido sobe threshold, solo seco desce
    float adj = IA_TH_ADJ_MAX * (1.0f - 2.0f * urg);

    float low_th  = clampf(IA_LOW_TH  + adj, 0.05f, 0.95f);
    float high_th = clampf(IA_HIGH_TH + adj, 0.05f, 0.95f);

    // Garante que high > low (histerese não pode inverter)
    if (high_th < low_th) {
        float mid = 0.5f * (high_th + low_th);
        low_th  = clampf(mid - 0.02f, 0.05f, 0.95f);
        high_th = clampf(mid + 0.02f, 0.05f, 0.95f);
    }

    bool vote = ia_should_irrigate_hysteresis(in, low_th, high_th, s_prev);

    float p = ia_predict_proba(in);

    // Anti-chattering
    uint64_t n = now_ms();
    bool vote_before_hold = vote;

    if (vote && !s_prev) {
        if (n - s_last_switch_ms < MIN_OFF_MS) vote = false;
    }
    else if (!vote && s_prev) {
        if (n - s_last_switch_ms < MIN_ON_MS) vote = true;
    }

    bool changed = (vote != s_prev);

    if (changed) {
        s_prev = vote;
        s_last_switch_ms = n;
    }

    *out_on = s_prev;

    #if DECIDER_LOG_DATA
    float x0n = (SCALER_SCALE[0] != 0.0f) ? ((soil - SCALER_MEAN[0]) / SCALER_SCALE[0]) : 0.0f;

    ESP_LOGI(TAG,
             "tick SOIL=%.1f(xn=%.2f) TEMP=%.1f RH=%.1f RAIN24=%.1f "
             "p=%.3f th=[%.2f..%.2f] urg=%.2f vote=%d hold=%s -> %s",
             soil, x0n, tc, rh, ch24,
             p, low_th, high_th, urg,
             (int)vote_before_hold,
             (vote_before_hold != vote) ? "YES" : "NO",
             s_prev ? "ON" : "OFF");

    if (changed) {
        ESP_LOGW(TAG,
                 "SWITCH %s (p=%.3f th=[%.2f..%.2f] soil=%.1f rain24=%.1f)",
                 s_prev ? "OFF->ON" : "ON->OFF",
                 p, low_th, high_th, soil, ch24);
    }
    #endif

    return true;
}
