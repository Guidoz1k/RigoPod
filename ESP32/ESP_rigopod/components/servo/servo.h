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

#define PWM_MIN_STD     2048    // (0.50ms / 4ms) * 2^14
#define PMW_MAX_STD     10240   // (2.50ms / 4ms) * 2^14
#define PWM_MIN         1024    // (0.25ms / 4ms) * 2^14
#define PWM_MAX         11264   // (2.75ms / 4ms) * 2^14

#define PWM_INC_MAX +256
#define PWM_DEC_MAX -256

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

void servo_move_pwm( int16_t pwm_bias, servo_t servo );

int16_t servo_return_angle( servo_t servo );

uint16_t servo_return_pwm_max( servo_t servo );

uint16_t servo_return_pwm_min( servo_t servo );

#endif /* __SERVO_H */