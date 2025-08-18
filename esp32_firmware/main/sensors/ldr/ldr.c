#include "ldr.h"

static const adc_oneshot_chan_cfg_t chan_cfg = {
    .atten    = LDR_ATTEN,        // usei DB_12 pra evitar warning
    .bitwidth = ADC_BITWIDTH_12
};

void ldr_init(void) {
    esp_err_t err = adc1_config_channel_once(LDR_ADC_CHANNEL, &chan_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao configurar canal do ADC: %s", esp_err_to_name(err));
    }
}

int ldr_get_luminosidade(void) {
    int raw = 0;
    esp_err_t err = adc1_read_raw(LDR_ADC_CHANNEL, &raw);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao ler LDR: %s", esp_err_to_name(err));
        return -1;
    }
    int lum = (raw * 100) / 4095;
    if (lum < 0) lum = 0;
    if (lum > 100) lum = 100;
    return lum;
}
