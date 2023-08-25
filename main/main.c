#include "wifi.h"
#include "spiffs.h"
#include "server.h"
#include "uart.h"

void app_main()
{
  nvs_flash_init();
  ESP_ERROR_CHECK(wifi_init());

  httpd_handle_t server = NULL;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server, NULL));

  const char* base_path = "/data";
  const char* partition_label = NULL;
  ESP_ERROR_CHECK(init_memory(base_path, partition_label)); // initialize spiffs
  ESP_ERROR_CHECK(spiffs_health_check(partition_label));
  ESP_ERROR_CHECK(start_server(base_path));
  ESP_ERROR_CHECK(init_uart());
}

