#include "ota.h"

static const char* TAG = "OTA";
static uint32_t reconnect_try_counter = 0;

static esp_err_t hello_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "Hello World!"
};

void connect_handler(void* arg, esp_event_base_t event_type, int32_t event_id, void* data) {
  //reset counter of reconnect tries
  reconnect_try_counter = 0;

  //print the IP
  ip_event_got_ip_t* ip = (ip_event_got_ip_t*)data;
  ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&ip->ip_info.ip));

  //start server if haven't started
  httpd_handle_t* server = (httpd_handle_t*)arg;
  if(*server == NULL){
    ESP_LOGI(TAG, "Start the server...");
    *server = start_browser();
  }
}

void disconnect_handler(void* arg, esp_event_base_t event_type, int32_t event_id, void* data) {
  ESP_LOGI(TAG, "Try to reconnect to SSID: %s PASS: %s", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
  if (reconnect_try_counter < CONFIG_ESP_WIFI_MAX_RECONNECT_COUNTER) {
    reconnect_try_counter++;
    esp_wifi_connect();
  } else {
    ESP_LOGE(TAG, "failed to reconnect to SSID: %s", CONFIG_ESP_WIFI_SSID);
  }
}

esp_err_t wifi_init() {
  ESP_LOGI(TAG, "wifi_init() start...");
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();
  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));


  wifi_config_t wifi_config = {.sta = {.ssid = CONFIG_ESP_WIFI_SSID,
                                       .password = CONFIG_ESP_WIFI_PASSWORD,
                                       .scan_method = WIFI_ALL_CHANNEL_SCAN,
                                       .sort_method = WIFI_CONNECT_AP_BY_SIGNAL}};

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_connect());
  return ESP_OK;
}

httpd_handle_t start_browser() {
  httpd_handle_t server = NULL;
  httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
  if (httpd_start(&server, &server_config) == ESP_OK) {
    ESP_LOGI(TAG, "Registering URI...");
    httpd_register_uri_handler(server, &hello);
    ESP_LOGI(TAG, "URI registering complete!");
  }
  return server;
}

