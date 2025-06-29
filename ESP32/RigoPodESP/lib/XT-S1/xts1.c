#include "xts1.h"

#define MAXWAIT 50  // 10 milliseconds to read buffer

static const char *TAG = "UART2";   // esp_err variable
static uint8_t read_timeout = pdMS_TO_TICKS(10);    // UART reading timeout

uint16_t crc16(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF; // Initial CRC value as per Modbus standard [1]

    for(size_t i = 0; i < length; ++i){
        crc ^= (uint16_t)data[i]; // XOR with current byte [1]
        for(size_t j = 0; j < 8; ++j){
            if(crc & 0x0001){ // If LSB (Least Significant Bit) is 1 [1]
                crc = (crc >> 1) ^ 0xA001; // Shift right and XOR with polynomial 0xA001 [1]
            }
            else{
                crc >>= 1; // Shift right [1]
            }
        }
    }
    return crc;
}

void modbus_flush(void){
    uart_flush(UART_NUM_2);
}

uint8_t modbus_check_buffer(void){
    size_t size_IDF = 0;
    uint8_t size = 0;

    uart_get_buffered_data_len(UART_NUM_2, &size_IDF);
    size = (uint8_t)size_IDF;

    return size;
}

void modbus_read_register(uint16_t start_address, uint8_t ammout, register_t type){
    uint8_t buffer[8] = {0};
    uint16_t crc_calc = 0;

    buffer[0] = 0x01;   // device ID

    if(type == INPUT_REG)
        buffer[1] = 0x04;   // functional code for "read input register"
    else
        buffer[1] = 0x03;   // functional code for "read holding register"

    buffer[2] = (start_address >> 8) & 0xFF;
    buffer[3] = start_address & 0xFF;
    buffer[4] = (ammout >> 8) & 0xFF;
    buffer[5] = ammout & 0xFF;

    crc_calc = crc16(buffer, 6);
    buffer[6] = crc_calc & 0xFF;
    buffer[7] = (crc_calc >> 8) & 0xFF;

    uart_write_bytes(UART_NUM_2, buffer, 8);
}

void modbus_write_register(uint16_t start_address, uint16_t value){
    uint8_t buffer[16] = {0};
    uint16_t crc_calc = 0;

    buffer[0] = 0x01;   // device ID
    buffer[1] = 0x06;   // functional code for "write single register"

    buffer[2] = (start_address >> 8) & 0xFF;
    buffer[3] = start_address & 0xFF;
    buffer[4] = (value >> 8) & 0xFF;
    buffer[5] = value & 0xFF;

    crc_calc = crc16(buffer, 6);
    buffer[6] = crc_calc & 0xFF;
    buffer[7] = (crc_calc >> 8) & 0xFF;

    uart_write_bytes(UART_NUM_2, buffer, 8);
}

void modbus_read_buffer(uint8_t *buffer, uint8_t size){
    uart_read_bytes(UART_NUM_2, buffer, size, read_timeout);
}

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

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 4, 5, -1, -1));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &configs));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, 2048, 0, 0, NULL, 0));

    modbus_write_register(0x0003, 0x0001);
    while(modbus_check_buffer() != 8)
        vTaskDelay(1);
    modbus_flush();

    ESP_LOGI(TAG, "XT-S1 Serial  initialized");
}

/*
uint16_t xts1_distance(void){
    return 1;
}
*/
