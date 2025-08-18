#ifndef LDR_H
#define LDR_H

#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#define TAG "LDR"

#define LDR_ADC_UNIT       ADC_UNIT_1
#define LDR_ADC_CHANNEL    ADC_CHANNEL_2   // GPIO3 = ADC1_CHANNEL_2
#define LDR_ATTEN          ADC_ATTEN_DB_11

void ldr_init(void);
int ldr_get_luminosidade(void);  // Retorna valor de 0 a 100

#endif
