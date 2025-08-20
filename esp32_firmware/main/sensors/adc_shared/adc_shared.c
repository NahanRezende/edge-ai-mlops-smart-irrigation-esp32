#include "adc_shared.h"
#include "esp_check.h"

static adc_oneshot_unit_handle_t s_adc1 = NULL;
// máscara de canais configurados (ADC1 tem CH0..CH9 no S3)
static uint16_t s_cfg_mask = 0;

adc_oneshot_unit_handle_t adc1_get_handle(void) {
    if (!s_adc1) {
        adc_oneshot_unit_init_cfg_t cfg = { .unit_id = ADC_UNIT_1 };
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&cfg, &s_adc1));
    }
    return s_adc1;
}

esp_err_t adc1_config_channel_once(adc_channel_t channel,
                                   const adc_oneshot_chan_cfg_t *cfg) {
    uint16_t bit = (1u << channel);
    if (s_cfg_mask & bit) return ESP_OK;
    adc_oneshot_unit_handle_t h = adc1_get_handle();
    esp_err_t err = adc_oneshot_config_channel(h, channel, cfg);
    if (err == ESP_OK) s_cfg_mask |= bit;
    return err;
}

esp_err_t adc1_read_raw(adc_channel_t channel, int *out_raw) {
    adc_oneshot_unit_handle_t h = adc1_get_handle();
    return adc_oneshot_read(h, channel, out_raw);
}
