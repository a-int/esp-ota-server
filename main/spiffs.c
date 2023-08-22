#include "spiffs.h"
#include "esp_spiffs.h"
static const char* TAG = "spiffs";

esp_err_t init_memory(const char* base_path, const char* partition_label) {
  ESP_LOGI(TAG, "Initializing SPIFFS...");
  esp_vfs_spiffs_conf_t spiffs_config = {
      .base_path = base_path, .partition_label = partition_label, .max_files = 3, .format_if_mount_failed = true};
  esp_err_t result = esp_vfs_spiffs_register(&spiffs_config);
  if (result != ESP_OK) {
    if (result == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount or format filesystem");
    } else if (result == ESP_ERR_NOT_FOUND) {
      ESP_LOGE(TAG, "Failed to find SPIFFS partition");
    } else {
      ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(result));
    }
    return ESP_FAIL;
  }
  return ESP_OK;
}

esp_err_t spiffs_health_check(const char* partition_label) {
  ESP_LOGI(TAG, "Check the SPIFFS health...");
  size_t total = 0;
  size_t used = 0;
  esp_err_t result = esp_spiffs_info(partition_label, &total, &used);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(result));
    esp_spiffs_format(partition_label);
    return ESP_FAIL;
  } else if(used > total){
    ESP_LOGE(TAG, "used(%zu) cannot be more than the total size (%zu)", used, total);
    esp_spiffs_check(partition_label);

    if (result != ESP_OK) {
      ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(result));
      return ESP_FAIL;
    } else {
      ESP_LOGI(TAG, "SPIFFS_check() successful");
    }
  }
  return ESP_OK;
}