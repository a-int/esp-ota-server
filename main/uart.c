#include "uart.h"

static const char* TAG = "uart";

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define RX_BUF_SIZE (1024)
#define UART (UART_NUM_2)

esp_err_t init_uart(void) {
    ESP_LOGI(TAG, "Initializing UART");
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART, RX_BUF_SIZE * 2, 0 , 0, NULL, 0);
    uart_param_config(UART, &uart_config);
    uart_set_pin(UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI(TAG, "UART initialized");
    return ESP_OK;
}

int sendData(const char* data, const int len)
{
    const int txBytes = uart_write_bytes(UART, data, len);
    ESP_LOGI(TAG, "Wrote %d bytes", txBytes);
    return txBytes;
}
