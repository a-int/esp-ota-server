#pragma once

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_event.h" 
#include "sdkconfig.h"

#define CONFIG_ESP_WIFI_MAX_RECONNECT_COUNTER 10

esp_err_t wifi_init();
