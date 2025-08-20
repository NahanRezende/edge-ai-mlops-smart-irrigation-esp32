#pragma once

// ===== ESP-IDF =====
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sdkconfig.h"

// ===== FreeRTOS =====
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ===== STD =====
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicia o servidor HTTP OTA
 *
 * @return ESP_OK em caso de sucesso, ou código de erro (esp_err_t)
 */
esp_err_t ota_server_start(void);

#ifdef __cplusplus
}
#endif
