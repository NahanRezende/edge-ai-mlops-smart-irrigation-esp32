/* IA params (gerado por treinar_modelo.py) */
#pragma once

#define IA_VERSION 20250818
// FEATURE_ORDER: soil_pct, temp_c, rh_pct, chuva_mm_24h

#define N_FEATS 4
static const float SCALER_MEAN[4]  = { 30.00000000f, 25.00000000f, 60.00000000f, 5.00000000f };
static const float SCALER_SCALE[4] = { 20.00000000f, 7.00000000f, 15.00000000f, 7.00000000f };
static const float W[4]            = { 0.00000000f, -0.36242818f, 0.00000000f, -8.11610546f };
static const float B[1]              = { 0.09847819f };

/* Série de previsão embutida (chuva acumulada nas próximas 24h)
   Índice aproximado em runtime:
     idx ≈ (now_epoch - FORECAST_T0_EPOCH) / (FORECAST_STEP_MIN * 60)
     idx clamp em [0, FORECAST_LEN-1]
*/
#define FORECAST_T0_EPOCH  1754697600
#define FORECAST_STEP_MIN  1440
#define FORECAST_LEN       7
static const float FORECAST_CHUVA_MM24[FORECAST_LEN] = { 0.10000000f, 0.00000000f, 0.00000000f, 2.60000000f, 0.20000000f, 0.00000000f, 0.40000000f };
