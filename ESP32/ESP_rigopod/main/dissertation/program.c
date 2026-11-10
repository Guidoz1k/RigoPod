#include "program.h"

/* NOTES
    This program needs all the peripherals to be connected, specially the ToF
    The ToF will throw an error in case it's not connected
*/

// ========== DEFINITIONS ==========

#define PITCH_SERVO_CALIB_BIAS  -200
#define ROLL_SERVO_CALIB_MIN    2100

#define LED_MAX 20
#define LED_MIN 10

#define WAIT_TIME 1000000 // 3s in µs for the wait

// ========== GLOBAL VARIABLES ==========

extern uint32_t time_counter;  // global timer updated in core 2 every 1µs (main.c)

int16_t current_pitch = 0, current_roll = 0;
uint8_t max_angle = 1500; // servo does 0.13s per 60deg @5V → 15.38deg per cycle @30Hz

enum mode_t{
    MODE_IDLE = 0,
    MODE_WAITING = 1,
    MODE_SERVO_ERROR = 2,
    MODE_TOF_ERROR = 3,
    MODE_COMM_ERROR = 4,
    MODE_ACTIVE = 5,
} system_mode;

// ============ FUNCTIONS ============

void inverse_kinematics( int16_t pitch, int16_t roll ){
    int16_t set_pitch = -pitch;
    int16_t set_roll = -roll;

    servo_set_angle( set_pitch, SERVO_PITCH );
    servo_set_angle( set_roll, SERVO_ROLL );
}

void servo_calib(){
    servo_move_pwm( PITCH_SERVO_CALIB_BIAS, SERVO_PITCH );
    servo_set_pwm_min( ROLL_SERVO_CALIB_MIN, SERVO_ROLL );
}

// function to set the RGB led according to current mode
void update_led(){
    switch( system_mode ){
    case MODE_IDLE:
        led_color( 0, 0, LED_MAX );
        break;
    case MODE_WAITING:
        led_color( LED_MAX, 0, 0 );
        break;
    case MODE_SERVO_ERROR:
        led_color( LED_MIN, LED_MIN, 0 );
        break;
    case MODE_TOF_ERROR:
        led_color( LED_MIN, 0, LED_MIN );
        break;
    case MODE_COMM_ERROR:
        led_color( 0, LED_MIN, LED_MIN );
        break;
    case MODE_ACTIVE:
        led_color( 0, LED_MAX, 0 );
        break;
    default:
        break;
    }
}

// function to detect if the drone system went idle
bool timeout_check( bool reset ){
    static uint32_t time_start = 0;
    uint32_t current_time = 0;
    bool counter_detect = false;

    if( reset == true )
        time_start = time_counter;
    else{
        current_time = time_counter - time_start;
        if( current_time >= WAIT_TIME) // 3s waiting
            counter_detect = true;
    }

    return counter_detect;
}

// ============ MAIN FUNCTION ============

void main_task(){
/*
sensor = xts1_measure_distance( &current );
char *distance_diag[ 13 ] = {
    "ERROR -13: overexposure",
    "ERROR -12: no object detected",
    "ERROR -11: abnormal TOF image",
    "ERROR -10: abnormal temperature image",
    "ERROR -9: abnormal grey scale image",
    "ERROR -8: reserve",
    "ERROR -7: signal too weak",
    "ERROR -6: signal too strong",
    "ERROR -5: reserve",
    "ERROR -4: sample data below min value",
    "ERROR -3: sample data beyond max value",
    "ERROR -2: pixel saturation",
    "ERROR -1: SPI communication error",
};
*/
}

void test_routine(){
    servo_calib();
    while( 1 ){
        main_task();
        delay_tick();
    }
}
