#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BIN = 2,
    DEC = 10,
    HEX = 16,
} base_t;

void serial_setup();

void serial_write_word( int16_t number, bool newline );

void serial_write_byte( uint8_t number, base_t base, bool newline );

void serial_write_char( uint8_t character, bool numerical, bool newline );

void serial_new_line();

void serial_write_string( const char *pointer, bool newline );

uint8_t serial_read_chars( char *buffer, uint8_t size );

char serial_read_singlechar();

uint8_t serial_read_size();

void serial_flush();

#endif /* __SERIAL_H */