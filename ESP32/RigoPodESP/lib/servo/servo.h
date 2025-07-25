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

typedef enum {
    PITCH_MIN = 1,
    PITCH_MAX = 2,
    PITCH_ANG = 3,
    ROLL_MIN = 4,
    ROLL_MAX = 5,
    ROLL_ANG = 6,
} calib_t;

void servo_setup();

void servo_pos( int16_t angle, servo_t servo );

void servo_cal( int16_t angle_min, int16_t angle_max, servo_t servo );

int16_t servo_string_to_number( char *pointer );

uint16_t servo_return_cal( calib_t parameter );

#endif /* __SERVO_H */