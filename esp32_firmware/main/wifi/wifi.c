#include "wifi.h"

static const char *TAG = "wifi";
static EventGroupHandle_t s_evt;
#define WIFI_OK BIT0

static void eh(void *arg, esp_event_base_t base, int32_t id, void *data){
  if(base==WIFI_EVENT && id==WIFI_EVENT_STA_START){ esp_wifi_connect(); }
  else if(base==WIFI_EVENT && id==WIFI_EVENT_STA_DISCONNECTED){
    ESP_LOGW(TAG,"disconnected; reconnecting");
    esp_wifi_connect();
  } else if(base==IP_EVENT && id==IP_EVENT_STA_GOT_IP){
    xEventGroupSetBits(s_evt, WIFI_OK);
  }
}

esp_err_t wifi_init_sta(void){
  esp_err_t r = nvs_flash_init();
  if(r==ESP_ERR_NVS_NO_FREE_PAGES || r==ESP_ERR_NVS_NEW_VERSION_FOUND){
    ESP_ERROR_CHECK(nvs_flash_erase());
    ESP_ERROR_CHECK(nvs_flash_init());
  }
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  s_evt = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &eh, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &eh, NULL, NULL));

  wifi_config_t wc = {0};
  snprintf((char*)wc.sta.ssid, sizeof(wc.sta.ssid), "%s", CONFIG_WIFI_SSID);
  snprintf((char*)wc.sta.password, sizeof(wc.sta.password), "%s", CONFIG_WIFI_PASS);
  wc.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
  wc.sta.pmf_cfg.capable = true; wc.sta.pmf_cfg.required = false;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wc));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "conectando no SSID: %s ...", CONFIG_WIFI_SSID);
  return ESP_OK;
}

bool wifi_wait_connected(int timeout_ms){
  EventBits_t b = xEventGroupWaitBits(s_evt, WIFI_OK, pdFALSE, pdTRUE, pdMS_TO_TICKS(timeout_ms));
  return (b & WIFI_OK) != 0;
}
