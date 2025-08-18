#pragma once
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
    #endif

    // Cria (se ainda não existir) e retorna o handle do ADC1 (oneshot)
    adc_oneshot_unit_handle_t adc1_get_handle(void);

    // Configura um canal do ADC1 apenas na primeira vez que for chamado
    esp_err_t adc1_config_channel_once(adc_channel_t channel,
                                    const adc_oneshot_chan_cfg_t *cfg);

    // Leitura RAW do ADC1 no canal especificado
    esp_err_t adc1_read_raw(adc_channel_t channel, int *out_raw);

    #ifdef __cplusplus
}
#endif
