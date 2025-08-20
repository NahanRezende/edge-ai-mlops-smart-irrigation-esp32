#pragma once
#include "esp_err.h"
#include <stdbool.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "sdkconfig.h"

esp_err_t wifi_init_sta(void);
bool wifi_wait_connected(int timeout_ms);
