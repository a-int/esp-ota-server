#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

esp_err_t init_memory(const char* base_path, const char* partition_label); // initialize SPIFFS
esp_err_t spiffs_health_check(const char* partition_label); //check the corectness of initialized SPIFFS
