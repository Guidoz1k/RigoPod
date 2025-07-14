#ifndef __MAIN_H
#define __MAIN_H

// ========== IDF LIBRARIES ==========

#include <stdio.h>
#include <esp_intr_alloc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/gptimer.h>
#include <math.h>

// ========== INTERNAL LIBRARIES ==========

#include "delay.h"
#include "serial.h"
#include "led.h"
#include "random.h"
#include "xts1.h"
#include "servo.h"

// ========== DEFINITIONS ==========

// core 0 interrupt disabled
//#define PERIOD0     10      // interrupt period in µs
//#define SYNCPIN0    47      // interruption 0 work pin
#define PERIOD1     1000    // interrupt period in µs
#define SYNCPIN1    21      // interruption 1 work pin

#endif /* __MAIN_H */