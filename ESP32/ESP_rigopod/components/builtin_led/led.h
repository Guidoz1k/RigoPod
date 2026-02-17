#ifndef __LED
#define __LED

// ============ ESP IDF LIBRARIES ============

#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdbool.h>
#include <stdint.h>
#include <driver/rmt_types.h>
#include <driver/rmt_tx.h>

// ============ DEFINITIONS ============

#define LED_GPIO_PIN        48
#define RMT_RESOLUTION_HZ   40000000
#define LED_BUFFER_SIZE     3
#define LED_TX_WAIT_TIME    100

// WS2812B timings for a 40MHz clock source (25ns period), we are going to have:
#define TOH 16  // t0H = 400ns - 16
#define TOL 34  // t0L = 850ns - 34
#define T1H 32  // t1H = 800ns - 32
#define T1L 22  // t1L = 450ns - 18

// ============ EXTERNAL FUNCTIONS ============

void led_setup();

void led_color( uint8_t r, uint8_t g, uint8_t b );

#endif /* __LED */