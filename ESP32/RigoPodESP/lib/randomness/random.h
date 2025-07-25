#ifndef __RANDOM_H
#define __RANDOM_H

#include <stdio.h>
#include <esp_random.h>
#include <bootloader_random.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdbool.h>
#include <stdint.h>

void random_setup();

uint32_t random_32();

bool random_1();

uint8_t random_n( uint32_t limit );

#endif /* __RANDOM_H */