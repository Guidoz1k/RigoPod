#ifndef __SERVO_H
#define __SERVO_H

// ============ ESP IDF LIBRARIES ============

#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdbool.h>
#include <stdint.h>
#include <driver/ledc.h>

// ============ DEFINITIONS ============

#define PWM_MIN     2048    // (0.5ms / 4ms) * 2^14
#define PMW_MAX     10240   // (2.5ms / 4ms) * 2^14

#define FULL_ANGLE      18000
#define HALF_ANGLE      9000
#define N_HALF_ANGLE    -9000

// ============ EXTERNAL TYPES ============

// pin definition
typedef enum {
    SERVO_PITCH = 13,
    SERVO_ROLL = 14,
} servo_t;

// ============ EXTERNAL FUNCTIONS ============

void servo_setup();

void servo_set_angle( int16_t angle, servo_t servo );

void servo_set_pwm_min( int16_t new_pwm_min, servo_t servo );

void servo_set_pwm_max( int16_t new_pwm_max, servo_t servo );

int16_t servo_return_angle( servo_t servo );

uint16_t servo_return_pwm_max( servo_t servo );

uint16_t servo_return_pwm_min( servo_t servo );

#endif /* __SERVO_H */