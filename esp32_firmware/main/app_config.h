#pragma once

// Janela quente (hora local)
#define HORA_QUENTE_INICIO   10   // 10:00
#define HORA_QUENTE_FIM      17   // 17:00

// Regras de solo e chuva
#define SOLO_CRITICO_PCT     15.0f   // override: pode irrigar mesmo 10–17h
#define SOLO_OK_PCT          25.0f   // se chover e solo ≥ OK → adiar
#define CHUVA_BLOQUEIO_MM24  8.0f

// Histerese da IA
#define IA_LOW_TH            0.45f
#define IA_HIGH_TH           0.55f

// Anti-chattering (mínimos entre trocas)
#define MIN_ON_MS            30000u   // 30 s
#define MIN_OFF_MS           120000u  // 120 s

// (Opcional) Gate por LDR
#define LDR_ESCURO_MV        400.0f

// --- Gate por LDR (habilita/desabilita a regra opcional) ---
#ifndef APP_USE_LDR_GATE
    #define APP_USE_LDR_GATE    0    // 0 = desligado, 1 = ligado
#endif

// --- Bomba (relé) ---
#ifndef PUMP_GPIO
    #define PUMP_GPIO           12   // ajuste pro seu hardware
#endif

#ifndef PUMP_ACTIVE_HIGH
    #define PUMP_ACTIVE_HIGH     1   // 1: nível alto liga; 0: nível baixo liga
#endif

// --- Taxa do loop de decisão (opcional) ---
#ifndef DECIDER_TICK_MS
    #define DECIDER_TICK_MS   10000  // 10 s
#endif

// --- Mapeamento das leituras (ajuste nomes se necessário) ---
#ifndef READ_SOIL_PCT
    #define READ_SOIL_PCT()        moisture_read_percent()  // 0–100
#endif

#ifndef READ_TEMP_C
    #define READ_TEMP_C()          dht22_read_temp_c()      // °C
#endif

#ifndef READ_RH_PCT
    #define READ_RH_PCT()          dht22_read_rh_pct()      // 0–100
#endif

#ifndef READ_LDR_MV
    #define READ_LDR_MV()          ldr_read_mv()            // mV (se usar LDR)
#endif

