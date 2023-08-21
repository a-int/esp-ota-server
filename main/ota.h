#pragma once

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_event.h" 
#include "sdkconfig.h"
#include "unistd.h"

#define CONFIG_ESP_WIFI_MAX_RECONNECT_COUNTER 10

esp_err_t wifi_init();

httpd_handle_t start_server();

void connect_handler(void* arg, esp_event_base_t event_type, int32_t event_id, void* data);
void disconnect_handler(void* arg, esp_event_base_t event_type, int32_t event_id, void* data);
