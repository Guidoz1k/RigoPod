#include "program.h"

/* NOTES
    This program needs all the peripherals to be connected, specially the ToF
    The ToF will throw an error in case it's not connected
*/

void servo_calib(){
/*
servo_set_angle( value, servo );
servo_move_pwm( value, servo );
servo_wide_pwm( value, servo );
servo_set_pwm_max( value, servo );
servo_set_pwm_min( value, servo );
servo_return_angle( servo );
servo_return_pwm_max( servo );
servo_return_pwm_min( servo );
*/
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
