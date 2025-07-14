#ifndef __DEMOS_H
#define __DEMOS_H

// ========== IDF LIBRARIES ==========

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

// ========== INTERNAL LIBRARIES ==========

#include "delay.h"
#include "led.h"
#include "xts1.h"
#include "servo.h"

void led_demo(void);

void servo_demo(void);

void lidar_demo(void);

#endif /* __DEMOS_H */