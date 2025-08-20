#ifndef MOISTURE_H
#define MOISTURE_H

#include "esp_adc/adc_oneshot.h"
#include "sensors/adc_shared/adc_shared.h"
#include "esp_log.h"

#define TAG_SOIL "SOIL"
#define MOISTURE_ADC_CHANNEL ADC_CHANNEL_6
#define MOISTURE_ATTEN        ADC_ATTEN_DB_12

void moisture_init(void);
int moisture_get_umidade(void);  // Retorna umidade em porcentagem (0-100)

#endif
