#ifndef __RANDOMNESS_H
#define __RANDOMNESS_H

// ============ ESP IDF LIBRARIES ============

#include <stdio.h>
#include <esp_random.h>
#include <bootloader_random.h>
#include <freertos/FreeRTOS.h>
#include <stdbool.h>
#include <stdint.h>

// ============ EXTERNAL FUNCTIONS ============

uint32_t randomness( uint32_t limit );

#endif /* __RANDOMNESS_H */