#ifndef LDR_H
#define LDR_H

#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#include "sensors/adc_shared/adc_shared.h"

#define LDR_ADC_UNIT       ADC_UNIT_1
#define LDR_ADC_CHANNEL    ADC_CHANNEL_5   // GPIO6 = ADC1_CHANNEL_5
// #define LDR_ADC_CHANNEL ADC_CHANNEL_3   // GPIO4 = ADC1_CH3 (comum no S3)
#define LDR_ATTEN          ADC_ATTEN_DB_12

void ldr_init(void);
int ldr_get_luminosidade(void);  // Retorna valor de 0 a 100

#endif
