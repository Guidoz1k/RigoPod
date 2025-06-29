#ifndef __SERVO_H
#define __SERVO_H

#include <stdio.h>
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdbool.h>
#include <stdint.h>
#include <driver/ledc.h>

#define PIN_PITCH   13
#define PIN_ROLL    14

typedef enum {
    SERVO_PITCH = 1,
    SERVO_ROLL = 2,
} servo_t;

void servo_setup();

void servo_pos(int16_t angle, servo_t servo);

#endif /* __SERVO_H */