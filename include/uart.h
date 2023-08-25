#pragma once
#include "esp_err.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"

esp_err_t init_uart();
int sendData(const char* data, const int len);
