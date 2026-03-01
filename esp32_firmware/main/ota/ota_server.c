#include "ota_server.h"

static const char *TAG = "ota_server";

/* Token do Kconfig; vazio = auth desligada */
#ifdef CONFIG_OTA_AUTH_TOKEN
    #define OTA_TOKEN_STR CONFIG_OTA_AUTH_TOKEN
#else
    #define OTA_TOKEN_STR ""
#endif

/* auth ligada? */
static inline bool ota_auth_required(void) {
    return OTA_TOKEN_STR[0] != '\0';
}

/* valida Authorization: Bearer <token> */
static esp_err_t check_auth(httpd_req_t *req) {
    if (!ota_auth_required()) return ESP_OK;

    size_t need = httpd_req_get_hdr_value_len(req, "Authorization");
    ESP_LOGI(TAG, "HDR need(Authorization)=%u, token_exp_len=%d",
             (unsigned)need, (int)strlen(OTA_TOKEN_STR));
    if (need == 0) {
        ESP_LOGE(TAG, "Authorization ausente");
        return ESP_ERR_INVALID_ARG;
    }

    char auth[256] = {0};
    if (need >= sizeof(auth)) {
        ESP_LOGE(TAG, "Authorization muito grande (%u)", (unsigned)need);
        return ESP_ERR_INVALID_ARG;
    }

    int rc = httpd_req_get_hdr_value_str(req, "Authorization", auth, sizeof(auth));
    
    ESP_LOGI(TAG, "HDR get rc=%d, got_len=%u, first='%c'",
         rc, (unsigned)strlen(auth), auth[0] ? auth[0] : '?');

    if (rc != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao ler Authorization");
        return ESP_ERR_INVALID_ARG;
    }

    /* trim início/fim */
    char *start = auth;
    while (*start == ' ' || *start == '\t') start++;
    char *end = auth + strlen(auth);
    while (end > start && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) end--;
    *end = '\0';

    /* precisa começar com Bearer */
    const char *prefix = "Bearer ";
    size_t plen = strlen(prefix);
    if (strncmp(start, prefix, plen) != 0) {
        ESP_LOGE(TAG, "Authorization sem prefixo 'Bearer ' (recebido: '%s')", start);
        return ESP_ERR_INVALID_ARG;
    }

    /* compara token recebido com o do Kconfig */
    const char *recv_token = start + plen;

    /* trim final de novo (depois do prefixo) */
    end = (char *)recv_token + strlen(recv_token);
    while (end > recv_token && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) end--;
    *end = '\0';

    ESP_LOGI(TAG, "cmp recv_len=%d vs exp_len=%d",
             (int)strlen(recv_token), (int)strlen(OTA_TOKEN_STR));

    if (strcmp(recv_token, OTA_TOKEN_STR) != 0) {
        ESP_LOGE(TAG, "Token inválido");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Token OK");
    return ESP_OK;
}

/* helper de erro em texto puro */
static void send_err_plain(httpd_req_t *req, const char *status, const char *msg) {
    httpd_resp_set_status(req, status);
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_send(req, msg, strlen(msg));
}

/* POST /ota */
static esp_err_t ota_post(httpd_req_t *req) {
    if (check_auth(req) != ESP_OK) {
        /* se auth falhar, devolve 401 direto */
        send_err_plain(req, "401 Unauthorized", "unauthorized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "OTA start: content_len=%d", req->content_len);

    /* pega partição de update */
    const esp_partition_t *update_part = esp_ota_get_next_update_partition(NULL);
    if (!update_part) {
        send_err_plain(req, "500 Internal Server Error", "no update partition");
        return ESP_OK;
    }

    /* inicia OTA */
    esp_ota_handle_t h = 0;
    esp_err_t err = esp_ota_begin(update_part, OTA_SIZE_UNKNOWN, &h);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ota_begin failed: %s", esp_err_to_name(err));
        send_err_plain(req, "500 Internal Server Error", "ota_begin failed");
        return ESP_OK;
    }

    /* buffer de chunk no heap */
    const size_t CHUNK = 4096;
    uint8_t *buf = (uint8_t *)malloc(CHUNK);
    if (!buf) {
        esp_ota_end(h);
        send_err_plain(req, "500 Internal Server Error", "no mem");
        return ESP_OK;
    }

    int remaining = req->content_len;
    size_t written = 0;

    /* recebe e grava a imagem */
    while (remaining > 0) {
        int to_read = remaining > (int)CHUNK ? (int)CHUNK : remaining;
        int n = httpd_req_recv(req, (char*)buf, to_read);
        if (n <= 0) {
            if (n == HTTPD_SOCK_ERR_TIMEOUT) {
                ESP_LOGW(TAG, "recv timeout, retry");
                continue;
            }
            ESP_LOGE(TAG, "recv error: %d", n);
            free(buf);
            esp_ota_end(h);
            send_err_plain(req, "500 Internal Server Error", "recv");
            return ESP_OK;
        }

        err = esp_ota_write(h, buf, n);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ota_write failed: %s", esp_err_to_name(err));
            free(buf);
            esp_ota_end(h);
            send_err_plain(req, "500 Internal Server Error", "ota_write");
            return ESP_OK;
        }

        written += n;
        remaining -= n;

        /* progresso a cada ~128 KB */
        if ((written % (128 * 1024)) < CHUNK) {
            ESP_LOGI(TAG, "OTA progress: %u / %d bytes",
                     (unsigned)written, req->content_len);
        }
    }

    free(buf);

    /* finaliza OTA */
    if ((err = esp_ota_end(h)) != ESP_OK) {
        ESP_LOGE(TAG, "ota_end failed: %s", esp_err_to_name(err));
        send_err_plain(req, "500 Internal Server Error", "ota_end");
        return ESP_OK;
    }

    /* marca partição como boot */
    if ((err = esp_ota_set_boot_partition(update_part)) != ESP_OK) {
        ESP_LOGE(TAG, "set_boot failed: %s", esp_err_to_name(err));
        send_err_plain(req, "500 Internal Server Error", "set_boot");
        return ESP_OK;
    }

    /* responde e reinicia */
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_sendstr(req, "OK, rebooting");
    ESP_LOGI(TAG, "OTA done, rebooting...");
    vTaskDelay(pdMS_TO_TICKS(300));
    esp_restart();
    return ESP_OK;
}

/* inicia servidor OTA */
esp_err_t ota_server_start(void) {
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.server_port = CONFIG_OTA_BIND_PORT;

    /* ajuste pra upload grande */
    cfg.stack_size = 12288;        // aumenta stack da task
    cfg.recv_wait_timeout = 60;    // tempo de espera recv
    cfg.send_wait_timeout = 60;    // tempo de espera send

    httpd_handle_t server = NULL;
    esp_err_t err = httpd_start(&server, &cfg);
    if (err != ESP_OK) return err;

    httpd_uri_t ota = {
        .uri = "/ota",
        .method = HTTP_POST,
        .handler = ota_post,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &ota);

    ESP_LOGI(TAG, "OTA pronto em http://<IP>:%d/ota", CONFIG_OTA_BIND_PORT);
    return ESP_OK;
}
