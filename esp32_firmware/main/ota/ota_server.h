#pragma once

// ESP-IDF
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sdkconfig.h"

// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// C stdlib
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// inicia o servidor HTTP OTA
esp_err_t ota_server_start(void);

#ifdef __cplusplus
}
#endif
