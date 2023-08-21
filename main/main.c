#include "ota.h"

void app_main()
{
  nvs_flash_init();
  ESP_ERROR_CHECK(wifi_init());

  httpd_handle_t server = NULL;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server, NULL));

  server = start_server();
  while(server){
    sleep(5);
  }
}

