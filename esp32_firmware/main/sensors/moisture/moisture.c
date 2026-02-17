#include "moisture.h"

static const adc_oneshot_chan_cfg_t chan_cfg = {
    .atten    = MOISTURE_ATTEN,
    .bitwidth = ADC_BITWIDTH_12
};

void moisture_init(void) {
    ESP_ERROR_CHECK(adc1_config_channel_once(MOISTURE_ADC_CHANNEL, &chan_cfg));
    ESP_LOGI(TAG_SOIL, "ADC1 CH=%d configurado", (int)MOISTURE_ADC_CHANNEL);
}

int moisture_read_raw(void) {
    int raw = 0;
    ESP_ERROR_CHECK(adc1_read_raw(MOISTURE_ADC_CHANNEL, &raw));
    return raw;
}

int moisture_get_umidade(void) {
    int raw = moisture_read_raw();

    ESP_LOGI(TAG_SOIL, "raw=%d", raw);

    // int umidade = 100 - ((raw * 100) / 4095); // simples: seco≈4095, molhado≈0

    int umidade = (SOIL_RAW_SECO - raw) * 100 / (SOIL_RAW_SECO - SOIL_RAW_MOLHADO);

    if (umidade < 0) umidade = 0;
    if (umidade > 100) umidade = 100;
    return umidade;
}
