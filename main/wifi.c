#include "wifi.h"

static const char* TAG = "wifi";
static uint32_t reconnect_try_counter = 0;

void connect_handler(void* arg, esp_event_base_t event_type, int32_t event_id, void* data) {
  //reset counter of reconnect tries
  reconnect_try_counter = 0;

  //print the IP
  ip_event_got_ip_t* ip = (ip_event_got_ip_t*)data;
  ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&ip->ip_info.ip));
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

