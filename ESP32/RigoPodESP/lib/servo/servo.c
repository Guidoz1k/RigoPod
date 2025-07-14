#include "servo.h"

// ========== DEFINITIONS ==========

#define PWM_FREQ    250     // period of 4ms
#define PWM_RES     14      // 14 bits
#define PWM_MIN     2048    // (0.5ms / 4ms) * 2^14
//#define PWM_MID     6144    // (1.5ms / 4ms) * 2^14
#define PMW_MAX     10240   // (2.5ms / 4ms) * 2^14

// ========== GLOBAL VARIABLES ==========

static const char *TAG = "SERVOS";   // esp_err variable

int16_t pitch_angle = 0;
int16_t roll_angle = 0;
uint16_t pitch_pwm_min = PWM_MIN;
uint16_t roll_pwm_min = PWM_MIN;
uint16_t pitch_pwm_max = PMW_MAX;
uint16_t roll_pwm_max = PMW_MAX;

// ============ EXTERNAL FUNCTIONS ============

void servo_setup(){
    ledc_timer_config_t ledc_timer_1 = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_14_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = PWM_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_channel_config_t ledc_channel_1 = {
        .gpio_num       = PIN_PITCH,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0, // Start with 0% duty
        .hpoint         = 0
    };
    ledc_timer_config_t ledc_timer_2 = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_14_BIT,
        .timer_num        = LEDC_TIMER_1,
        .freq_hz          = PWM_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_channel_config_t ledc_channel_2 = {
        .gpio_num       = PIN_ROLL,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_1,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_1,
        .duty           = 0, // Start with 0% duty
        .hpoint         = 0
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_1));
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_1));
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_2));
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_2));
    servo_pos(0, SERVO_PITCH);
    servo_pos(0, SERVO_ROLL);
    ESP_LOGI(TAG, "Servos Initialized");
}

/* Sets the servo in a designated angle
    uses input as 9000, representing 90.00 degrees
    total input range is -9000 to 9000
    each increment in angle represents a cent of an degree
*/
void servo_pos(int16_t angle, servo_t servo){
    uint16_t pos = 0;

    switch(servo){
    case SERVO_PITCH:
        pitch_angle = angle;
        pos = (((pitch_pwm_max - pitch_pwm_min) * pitch_angle)/(9000 * 2)) + ((pitch_pwm_max + pitch_pwm_min) / 2);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, pos);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        break;
    case SERVO_ROLL:
        roll_angle = angle;
        pos = (((roll_pwm_max - roll_pwm_min) * roll_angle)/(9000 * 2)) + ((roll_pwm_max + roll_pwm_min) / 2);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, pos);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
        break;
    default:
        break;
    }
}

// adjusts the min and maximum value of the PWM range function
void servo_cal(int16_t angle_min, int16_t angle_max, servo_t servo){
    switch(servo){
    case SERVO_PITCH:
        pitch_pwm_min += angle_min;
        pitch_pwm_max += angle_max;

        servo_pos(pitch_angle, servo);
        break;
    case SERVO_ROLL:
        roll_pwm_min += angle_min;
        roll_pwm_max += angle_max;
        
        servo_pos(roll_angle, servo);
        break;
    default:
        break;
    }
}

/* Converts serial orders to int16_t position data
   serial characters are:
    0 - signal, '+' or '-'
    1 - number
    2 - number
    3 - '.' char to make prompting easier
    4 - number
    5 - number
   returns 0x7FFF at error
*/
int16_t servo_string_to_number(char *pointer){
    int16_t result = 0x7FFF;

    if( ((pointer[0] == '+') || (pointer[0] == '-')) &&
        ((pointer[1] >= '0') && (pointer[1] <= '9')) &&
        ((pointer[2] >= '0') && (pointer[2] <= '9')) &&
        ((pointer[3] >= '0') && (pointer[3] <= '9')) &&
        ((pointer[4] >= '0') && (pointer[4] <= '9'))
        ){
        result = pointer[4] - 48;
        result += (pointer[3] - 48) * 10;
        result += (pointer[2] - 48) * 100;
        result += (pointer[1] - 48) * 1000;
        if(pointer[0] == '-')
            result *= -1;
    }

    return result;
}

uint16_t servo_return_cal(calib_t parameter){
    uint16_t value = 0;

    switch(parameter){
    case PITCH_MAX:
        value = pitch_pwm_max;
        break;
    case PITCH_MIN:
        value = pitch_pwm_min;
        break;
    case PITCH_ANG:
        value = pitch_angle;
        break;
    case ROLL_MAX:
        value = roll_pwm_max;
        break;
    case ROLL_MIN:
        value = roll_pwm_min;
        break;
    case ROLL_ANG:
        value = roll_angle;
        break;
    }

    return value;
}
