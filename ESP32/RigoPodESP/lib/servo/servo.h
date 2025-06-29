#ifndef __SERVO_H
#define __SERVO_H

#include <stdbool.h>
#include <stdint.h>
#include <driver/ledc.h>

typedef enum {
    SERVO_PITCH = 1,
    SERVO_ROLL = 2,
} servo_t;

void servo_setup();

void servo_pos(int16_t angle, servo_t servo);

#endif /* __SERVO_H */