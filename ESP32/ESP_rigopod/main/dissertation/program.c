#include "program.h"

/* NOTES
    This program needs all the peripherals to be connected, specially the ToF
    The ToF will throw an error in case it's not connected
*/

// ========== DEFINITIONS ==========

#define PITCH_SERVO_CALIB_BIAS  -200
#define ROLL_SERVO_CALIB_MIN    2100
#define ROLL_SERVO_CALIB_MAX    10100

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
    servo_set_pwm_max( ROLL_SERVO_CALIB_MAX, SERVO_ROLL );
}

void main_task(){
    const uint8_t brightness = 20;
    led_color( randomness(brightness), randomness(brightness), randomness(brightness) );
    delay_milli(100);
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
