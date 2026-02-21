#include "servo.h"

// ========== DEFINITIONS ==========

#define PWM_FREQ    250     // period of 4ms

// ========== GLOBAL VARIABLES ==========

static const char *TAG = "SERVOS";   // esp_err variable

typedef struct _servo_t_{
    int16_t angle;
    uint16_t pwm_min;
    uint16_t pwm_max;  
} servo_data_t;
servo_data_t pitch_servo = {
    .angle = 0,
    .pwm_min = PWM_MIN_STD,
    .pwm_max = PMW_MAX_STD,
};
servo_data_t roll_servo = {
    .angle = 0,
    .pwm_min = PWM_MIN_STD,
    .pwm_max = PMW_MAX_STD,
};

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
        .gpio_num       = SERVO_PITCH,
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
        .gpio_num       = SERVO_ROLL,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_1,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_1,
        .duty           = 0, // Start with 0% duty
        .hpoint         = 0
    };

    ESP_ERROR_CHECK( ledc_timer_config( &ledc_timer_1 ) );
    ESP_ERROR_CHECK( ledc_channel_config( &ledc_channel_1 ) );
    ESP_ERROR_CHECK( ledc_timer_config( &ledc_timer_2 ) );
    ESP_ERROR_CHECK( ledc_channel_config( &ledc_channel_2 ) );
    servo_set_angle( 0, SERVO_PITCH );
    servo_set_angle( 0, SERVO_ROLL);
    ESP_LOGI( TAG, "Servos Initialized" );
}

/* Sets the servo in a designated angle
    uses input as 9000, representing 90.00 degrees
    total input range is -9000 to 9000
    each increment in angle represents a cent of an degree
*/
void servo_set_angle( int16_t angle, servo_t servo ){
    uint16_t pos = 0;

    if( ( angle >= N_HALF_ANGLE ) && ( angle <= HALF_ANGLE ) ){
        switch( servo ){
        case SERVO_PITCH:
            pitch_servo.angle = angle;
            pos = ( ( ( pitch_servo.pwm_max - pitch_servo.pwm_min ) * angle ) / FULL_ANGLE ); // angular coefficient
            pos += ( pitch_servo.pwm_max + pitch_servo.pwm_min ) / 2; // linear coefficient

            ledc_set_duty( LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, pos );
            ledc_update_duty( LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0 );
            break;
        case SERVO_ROLL:
            roll_servo.angle = angle;
            pos = ( ( ( roll_servo.pwm_max - roll_servo.pwm_min ) * angle ) / FULL_ANGLE ); // angular coefficient
            pos += ( roll_servo.pwm_max + roll_servo.pwm_min ) / 2; // linear coefficient

            ledc_set_duty( LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, pos );
            ledc_update_duty( LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1 );
            break;
        default:
            break;
        }
    }
}

// update servo pos calculation after range adjustments
void update_servos(){
    servo_set_angle( pitch_servo.angle, SERVO_PITCH );
    servo_set_angle( roll_servo.angle, SERVO_ROLL );
}

// adjusts the min value of the PWM range
void servo_set_pwm_min( int16_t new_pwm_min, servo_t servo ){
    if( ( new_pwm_min >= N_HALF_ANGLE ) && ( new_pwm_min <= 0 ) ){
        switch( servo ){
            case SERVO_PITCH:
                pitch_servo.pwm_min = new_pwm_min;
                break;
            case SERVO_ROLL:
                roll_servo.pwm_min = new_pwm_min;
                break;
            default:
                break;
        }
        update_servos();
    }
}

// adjusts the max value of the PWM range
void servo_set_pwm_max( int16_t new_pwm_max, servo_t servo ){
    if( ( new_pwm_max <= HALF_ANGLE ) && ( new_pwm_max >= 0 ) ){
        switch( servo ){
            case SERVO_PITCH:
                pitch_servo.pwm_max = new_pwm_max;
                break;
            case SERVO_ROLL:
                roll_servo.pwm_max = new_pwm_max;
                break;
            default:
                break;
        }
        update_servos();
    }
}

// adds or subtracts number from both PWM max and min
void servo_move_pwm( int16_t pwm_bias, servo_t servo ){
    if( ( pwm_bias <= PWM_INC_MAX ) && ( pwm_bias >= PWM_DEC_MAX ) ){
        switch( servo ){
            case SERVO_PITCH:
                pitch_servo.pwm_max += pwm_bias;
                pitch_servo.pwm_min += pwm_bias;
                break;
            case SERVO_ROLL:
                roll_servo.pwm_max += pwm_bias;
                roll_servo.pwm_min += pwm_bias;
                break;
            default:
                break;
        }
        update_servos();
    }
}

// returns the angle of the servo
int16_t servo_return_angle( servo_t servo ){
    int16_t value = 0;

    switch( servo ){
    case SERVO_PITCH:
        value = pitch_servo.angle;
        break;
    case SERVO_ROLL:
        value = roll_servo.angle;
        break;
    default:
        break;
    }

    return value;
}

// returns the max pwm of the servo
uint16_t servo_return_pwm_max( servo_t servo ){
    uint16_t value = 0;

    switch( servo ){
    case SERVO_PITCH:
        value = pitch_servo.pwm_max;
        break;
    case SERVO_ROLL:
        value = roll_servo.pwm_max;
        break;
    default:
        break;
    }

    return value;
}

// returns the min pwm of the servo
uint16_t servo_return_pwm_min( servo_t servo ){
    uint16_t value = 0;

    switch( servo ){
    case SERVO_PITCH:
        value = pitch_servo.pwm_min;
        break;
    case SERVO_ROLL:
        value = roll_servo.pwm_min;
        break;
    default:
        break;
    }

    return value;
}
