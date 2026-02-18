#pragma once

// Defines para os GPIO
#define DHT_GPIO    17   // DHT22 
#define LDR_GPIO     6   // LDR
#define SOIL_GPIO    7   // Umidade do solo 
#define PUMP_GPIO   12   // Bomba/relé

// Trava de segurança para a bomba (1 = só simula; 0 = controla o GPIO de verdade)
#define PUMP_DRY_RUN  1

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

// --- Threshold adaptativo (solo regula urgência) ---
#define SOIL_REF_UMIDO_PCT      40.0f   // acima disso, solo “úmido” -> reduz chance de irrigar
#define IA_TH_ADJ_MAX           0.20f   // ajuste máximo nos thresholds (0.15~0.25 é um bom intervalo)

// Quanto o solo "puxa" o logit quando está úmido.
// 1.5 ~ 3.0 costuma ser um intervalo bom. Começa em 2.5.
#define IA_SOIL_LOGIT_PENALTY  5.5f

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
    #define READ_SOIL_PCT()        moisture_get_umidade()  // 0–100
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

