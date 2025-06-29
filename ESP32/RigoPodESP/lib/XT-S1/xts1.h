#ifndef __XTS1_H
#define __XTS1_H

#include <stdbool.h>
#include <stdint.h>
#include <esp_log.h>

typedef enum{
    INPUT_REG = 0,
    HOLDING_REG = 1,
} register_t;

void xts1_setup();

void modbus_flush();

uint8_t modbus_check_buffer(void);

void modbus_read_register(uint16_t start_address, uint8_t ammout, register_t type);

void modbus_write_register(uint16_t start_address, uint16_t value);

void modbus_read_buffer(uint8_t *buffer, uint8_t size);

#endif /* __XTS1_H */