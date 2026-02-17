#ifndef __MAIN_H
#define __MAIN_H

// ========== IDF LIBRARIES ==========

#include <stdio.h>
#include <inttypes.h>
#include <esp_intr_alloc.h>
#include <esp_mac.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include <driver/gptimer.h>
#include <math.h>

// ========== INTERNAL LIBRARIES ==========

#include "led.h"
#include "delay.h"
#include "randomness.h"
#include "xts1.h"
#include "servo.h"
// #include "customserial.h"

#include "demos/demo_servo.c"
#include "demos/demo_led.c"
#include "demos/demo_lidar.c"

// ========== DEFINITIONS ==========

#define PERIOD1     1000 // interrupt period in µs
#define SYNCPIN1    3     // interruption 1 work pin

#endif /* __MAIN_H */