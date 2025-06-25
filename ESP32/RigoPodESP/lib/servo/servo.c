#include "servo.h"

// ========== IDF LIBRARIES ==========

#include <stdio.h>
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_log.h>

#define PWM_FREQ    250     // period of 4ms
#define PWM_RES     14      // 14 bits
#define PWM_MIN     2048    // (0.5ms / 4ms) * 2^14
#define PWM_MID     6144    // (1.5ms / 4ms) * 2^14
#define PMW_MAX     10240   // (2.5ms / 4ms) * 2^14

void servo_setup(){
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_14_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = PWM_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = PWM_RES,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0, // Start with 0% duty
        .hpoint         = 0
    };

    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&ledc_channel);
    servo_pos(0);
}

/* Sets the servo in a designated angle
    uses input as triple of 9000, representing 90.00 degrees
    each increment in angle represents a third of a cent of an degree
*/
void servo_pos(int16_t angle){
    uint32_t pos = (((PMW_MAX - PWM_MID) * angle)/27000) + PWM_MID;

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, pos);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}
