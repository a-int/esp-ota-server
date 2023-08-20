#include "ota.h"
#include "portmacro.h"

void app_main()
{
  nvs_flash_init();
  ESP_ERROR_CHECK(wifi_init());
}

