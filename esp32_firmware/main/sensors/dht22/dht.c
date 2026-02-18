#include "dht.h"
#include "esp_timer.h"

#define TAG "DHT22"

static gpio_num_t dht_gpio;
static dht_sensor_type_t dht_type;

// Cache da última leitura válida
static float   s_last_t = -127.0f;
static float   s_last_h = -1.0f;
static int64_t s_last_ok_us = 0;
static bool    s_has_cache = false;

// Controle de tentativa (pra não martelar o sensor quando está falhando)
static int64_t s_last_attempt_us = 0;

// Protege cache/controle
static SemaphoreHandle_t s_mu = NULL;

// Protege só a janela curta de captura dos pulsos
static portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;

static inline int64_t now_us(void) { return esp_timer_get_time(); }

// Espera ficar em "level" ou timeout
static inline bool wait_level(gpio_num_t pin, int level, int timeout_us) {
    int64_t t0 = now_us();
    while (gpio_get_level(pin) != level) {
        if ((now_us() - t0) > timeout_us) return false;
        esp_rom_delay_us(1);
    }
    return true;
}

// Espera sair de "level" (borda) ou timeout
static inline bool wait_not_level(gpio_num_t pin, int level, int timeout_us) {
    int64_t t0 = now_us();
    while (gpio_get_level(pin) == level) {
        if ((now_us() - t0) > timeout_us) return false;
        esp_rom_delay_us(1);
    }
    return true;
}

// Captura handshake + 40 bits (TEM que ser rápido)
static bool dht_capture_bits(gpio_num_t pin, uint8_t data[5]) {
    // Resposta do sensor: LOW -> HIGH -> LOW
    if (!wait_level(pin, 0, DHT_EDGE_TIMEOUT_US)) return false;
    if (!wait_not_level(pin, 0, DHT_EDGE_TIMEOUT_US)) return false;
    if (!wait_not_level(pin, 1, DHT_EDGE_TIMEOUT_US)) return false;

    // 40 bits: LOW ~50us + HIGH (0 curto / 1 longo)
    for (int bit = 0; bit < 40; bit++) {
        if (!wait_level(pin, 0, DHT_EDGE_TIMEOUT_US)) return false;     // garante LOW
        if (!wait_not_level(pin, 0, DHT_EDGE_TIMEOUT_US)) return false; // sobe (início do HIGH)

        int64_t t_high_start = now_us();
        if (!wait_not_level(pin, 1, DHT_EDGE_TIMEOUT_US)) return false; // desce (fim do HIGH)
        int64_t high_us = now_us() - t_high_start;

        data[bit / 8] <<= 1;
        if (high_us > 50) data[bit / 8] |= 1; // corte típico
    }

    return true;
}

void dht_init(gpio_num_t gpio, dht_sensor_type_t sensor_type) {
    dht_gpio = gpio;
    dht_type = sensor_type;

    gpio_reset_pin(dht_gpio);
    gpio_pullup_en(dht_gpio);
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT);

    if (!s_mu) {
        s_mu = xSemaphoreCreateMutex();
        if (!s_mu) {
            ESP_LOGE(TAG, "Falha ao criar mutex do DHT");
        }
    }

    ESP_LOGI(TAG, "Init OK (GPIO=%d, type=%d)", (int)dht_gpio, (int)dht_type);
}

// Mantém assinatura do seu projeto
bool dht_read(gpio_num_t pin, float *temperature, float *humidity) {
    (void)dht_type;

    uint8_t data[5] = {0, 0, 0, 0, 0};

    // Start: LOW por ~20ms (NÃO pode estar em critical aqui)
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
    vTaskDelay(pdMS_TO_TICKS(DHT_START_LOW_MS));

    // Solta e vira input
    gpio_set_level(pin, 1);
    esp_rom_delay_us(DHT_START_HIGH_US);
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    // Agora sim: janela crítica só para capturar os pulsos
    bool ok = false;
    taskENTER_CRITICAL(&s_mux);
    ok = dht_capture_bits(pin, data);
    taskEXIT_CRITICAL(&s_mux);

    if (!ok) return false;

    // Checksum
    uint8_t checksum = (uint8_t)((data[0] + data[1] + data[2] + data[3]) & 0xFF);
    if (data[4] != checksum) return false;

    // Conversão DHT22
    *humidity = ((data[0] << 8) | data[1]) * 0.1f;

    int16_t raw_t = (int16_t)(((data[2] & 0x7F) << 8) | data[3]);
    float t = raw_t * 0.1f;
    if (data[2] & 0x80) t *= -1.0f;
    *temperature = t;

    return true;
}

// Cache + rate-limit (vale para sucesso e falha)
static bool dht22_get_cached(float *out_t, float *out_h) {
    if (!s_mu) return false;

    // lê cache/controle
    xSemaphoreTake(s_mu, portMAX_DELAY);
    bool has = s_has_cache;
    int64_t last_ok_us = s_last_ok_us;
    int64_t last_attempt_us = s_last_attempt_us;
    float ct = s_last_t;
    float ch = s_last_h;
    xSemaphoreGive(s_mu);

    int64_t now = now_us();

    // Cache hit (última leitura válida ainda “nova”)
    if (has) {
        int64_t age_ms = (now - last_ok_us) / 1000LL;
        if (age_ms >= 0 && age_ms < DHT_MIN_INTERVAL_MS) {
            if (out_t) *out_t = ct;
            if (out_h) *out_h = ch;
            ESP_LOGD(TAG, "CACHE HIT -> age=%lld ms T=%.1fC H=%.1f%%", age_ms, ct, ch);
            return true;
        }
    }

    // Rate-limit mesmo quando falha (para não martelar o DHT)
    int64_t since_attempt_ms = (now - last_attempt_us) / 1000LL;
    if (since_attempt_ms >= 0 && since_attempt_ms < DHT_MIN_RETRY_MS) {
        ESP_LOGD(TAG, "SKIP -> retry em %lld ms", (int64_t)DHT_MIN_RETRY_MS - since_attempt_ms);
        return has; // se tiver cache, pelo menos mantém; se não tiver, retorna false
    }

    // registra tentativa
    xSemaphoreTake(s_mu, portMAX_DELAY);
    s_last_attempt_us = now;
    xSemaphoreGive(s_mu);

    ESP_LOGD(TAG, "CACHE MISS -> nova leitura");

    float t = -127.0f, h = -1.0f;
    bool ok = dht_read(dht_gpio, &t, &h);

    if (!ok) {
        // retry curto (sem ficar insistindo)
        vTaskDelay(pdMS_TO_TICKS(20));
        ok = dht_read(dht_gpio, &t, &h);
    }

    if (ok) {
        xSemaphoreTake(s_mu, portMAX_DELAY);
        s_last_t = t;
        s_last_h = h;
        s_last_ok_us = now_us();
        s_has_cache = true;
        xSemaphoreGive(s_mu);

        ESP_LOGI(TAG, "READ OK -> T=%.1fC H=%.1f%% (nova leitura)", t, h);
        if (out_t) *out_t = t;
        if (out_h) *out_h = h;
        return true;
    }

    ESP_LOGW(TAG, "READ FAIL -> mantendo ultimo valor valido");
    return has;
}

float dht22_read_temp_c(void) {
    float t = -127.0f, h = -1.0f;
    if (!dht22_get_cached(&t, &h)) {
        ESP_LOGW(TAG, "dht22_read_temp_c(): sem leitura valida agora");
    }
    return t;
}

float dht22_read_rh_pct(void) {
    float t = -127.0f, h = -1.0f;
    if (!dht22_get_cached(&t, &h)) {
        ESP_LOGW(TAG, "dht22_read_rh_pct(): sem leitura valida agora");
    }
    return h;
}
