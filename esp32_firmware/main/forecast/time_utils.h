#pragma once
#include <stdbool.h>
#include <time.h>
#include "app_config.h"

static inline bool is_hot_window(void) {
    time_t now = time(NULL);
    struct tm lt;
    localtime_r(&now, &lt); // usa TZ configurado no sistema
    return (lt.tm_hour >= HORA_QUENTE_INICIO) && (lt.tm_hour < HORA_QUENTE_FIM);
}
