#ifndef __LED
#define __LED

#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdbool.h>
#include <stdint.h>
#include <driver/rmt_types.h>
#include <driver/rmt_tx.h>

void led_setup();

void led_color( uint8_t r, uint8_t g, uint8_t b );

#endif /* __LED */