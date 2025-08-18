#include "decider.h"

// Usamos apenas os macros de leitura definidos em app_config.h:
//   READ_SOIL_PCT(), READ_TEMP_C(), READ_RH_PCT(), (opcional) READ_LDR_MV()

// estado interno (histerese + anti-chattering)
static bool     s_prev = false;
static uint64_t s_last_switch_ms = 0;

static inline uint64_t now_ms(void) { return (uint64_t)(esp_timer_get_time() / 1000ULL); }

static void build_features(float in[]) {
    float soil = (float)READ_SOIL_PCT();
    float tc   = (float)READ_TEMP_C();
    float rh   = (float)READ_RH_PCT();
    float ch24 = forecast_chuva24h_now();

    #if N_FEATS == 3
        // FEATURE_ORDER (header): soil_pct, temp_c, rh_pct
        in[0] = soil; in[1] = tc; in[2] = rh;
    #elif N_FEATS == 4
        // FEATURE_ORDER (header): soil_pct, temp_c, rh_pct, chuva_mm_24h
        in[0] = soil; in[1] = tc; in[2] = rh; in[3] = ch24;
    #else
        # error "N_FEATS inesperado — ajuste build_features()"
    #endif
}

void decider_init(void) {
    s_prev = false;
    s_last_switch_ms = now_ms();
}

bool decider_tick(bool *out_on) {
    if (!out_on) return false;

    float soil = (float)READ_SOIL_PCT();
    float tc   = (float)READ_TEMP_C();
    float rh   = (float)READ_RH_PCT();
    float ch24 = forecast_chuva24h_now();

    // 1) Janela quente (bloqueia entre 10–17h exceto solo crítico)
    if (is_hot_window() && soil >= SOLO_CRITICO_PCT) {
        if (s_prev) {
            uint64_t n = now_ms();
            if (n - s_last_switch_ms < MIN_ON_MS) { *out_on = true; return true; }
            s_prev = false; s_last_switch_ms = n;
        }
        *out_on = false; return true;
    }

    // 2) Chuva prevista forte + solo ok -> adiar
    if ((ch24 >= CHUVA_BLOQUEIO_MM24) && (soil >= SOLO_OK_PCT)) {
        if (s_prev) {
            uint64_t n = now_ms();
            if (n - s_last_switch_ms < MIN_ON_MS) { *out_on = true; return true; }
            s_prev = false; s_last_switch_ms = n;
        }
        *out_on = false; return true;
    }

    #if APP_USE_LDR_GATE
        // 3) (Opcional) Gate por LDR — ajuste o comparador conforme sua calibração.
        float ldr_mv = (float)READ_LDR_MV();
        // Exemplo: bloquear se estiver claro (substitua a regra conforme seu circuito):
        // if (ldr_mv >= LDR_ESCURO_MV && soil >= SOLO_CRITICO_PCT) { ... }
        (void)ldr_mv; // evite warning se não usar
    #endif

        // 4) IA + histerese
        float in[N_FEATS];
        build_features(in);
        bool vote = ia_should_irrigate_hysteresis(in, IA_LOW_TH, IA_HIGH_TH, s_prev);

        // 5) Anti-chattering
        uint64_t n = now_ms();
        if (vote && !s_prev) {
            if (n - s_last_switch_ms < MIN_OFF_MS) vote = false;
        } else if (!vote && s_prev) {
            if (n - s_last_switch_ms < MIN_ON_MS)  vote = true;
        }

        if (vote != s_prev) {
            s_prev = vote;
            s_last_switch_ms = n;
        }

        // Debug:
        // printf("soil=%.1f temp=%.1f rh=%.1f chuva24=%.1f -> %s\n",
        //        soil, tc, rh, ch24, s_prev ? "ON" : "OFF");

        *out_on = s_prev;
        return true;
}
