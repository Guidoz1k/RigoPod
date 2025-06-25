#include "xts1.h"

// ========== IDF LIBRARIES ==========

#include <stdio.h>
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_log.h>

static const char *TAG = "UART1";   // esp_err variable

void xts1_setup(){
    uart_config_t configs = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, 17, 18, -1, -1));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &configs));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 2048, 0, 0, NULL, 0));

    ESP_LOGI(TAG, "XT-S1 Serial  initialized");
}