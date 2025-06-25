#ifndef __SERVO_H
#define __SERVO_H

#include <stdbool.h>
#include <stdint.h>
#include <driver/ledc.h>

void servo_setup();

void servo_pos(int16_t angle);

#endif /* __SERVO_H */