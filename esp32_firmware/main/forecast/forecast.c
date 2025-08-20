#include "forecast.h"

static inline int clampi(int v, int lo, int hi) { return v<lo?lo : v>hi?hi : v; }

float forecast_chuva24h_now(void) {
    #ifdef FORECAST_LEN
        time_t now = time(NULL);                // epoch (s)
        double step_s = (double)FORECAST_STEP_MIN * 60.0;
        double idxf = ((double)now - (double)FORECAST_T0_EPOCH) / step_s;
        int idx = (int)lrint(idxf);             // arredonda pro inteiro mais próximo
        idx = clampi(idx, 0, (int)FORECAST_LEN - 1);
        return FORECAST_CHUVA_MM24[idx];
    #else
        return 0.0f; // se não existir previsão embutida no header
    #endif
}
