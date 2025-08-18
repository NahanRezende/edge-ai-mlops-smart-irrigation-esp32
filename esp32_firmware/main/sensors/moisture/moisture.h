#ifndef MOISTURE_H
#define MOISTURE_H

#include "driver/adc.h"

void moisture_init(void);
int moisture_get_umidade(void);  // Retorna umidade em porcentagem (0-100)

#endif
