#include "moisture.h"

#define MOISTURE_ADC_CHANNEL ADC1_CHANNEL_3

void moisture_init(void) {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(MOISTURE_ADC_CHANNEL, ADC_ATTEN_DB_11);
}

int moisture_get_umidade(void) {
    int raw = adc1_get_raw(MOISTURE_ADC_CHANNEL);
    int umidade = 100 - ((raw * 100) / 4095);

    if (umidade < 0) umidade = 0;
    if (umidade > 100) umidade = 100;

    return umidade;
}
