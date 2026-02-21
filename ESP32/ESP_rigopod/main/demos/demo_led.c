#include "demo_led.h"

#define MAX_ANGLE   4 * M_PI

/* OLD TRIANGLE WAVE DEMO
void led_demo(){
    const uint8_t maximum = 100;
    uint8_t red = 0, green = 2 * maximum / 3, blue = 2 * maximum / 3;
    bool red_status = true, green_status = true, blue_status = false;

    while( true ){ // this code tests the buil-in led
        delay_milli( 10 );

        if( red_status == true )
            if( red < maximum )
                red++;
            else
                red_status = false;
        else
            if( red > 0 )
                red--;
            else
                red_status = true;

        if( green_status == true )
            if( green < maximum )
                green++;
            else
                green_status = false;
        else
            if( green > 0 )
                green--;
            else
                green_status = true;

        if( blue_status == true )
            if( blue < maximum )
                blue++;
            else
                blue_status = false;
        else
            if( blue > 0 )
                blue--;
            else
                blue_status = true;

        printf( " red: %d green %d blue: %d \n", red, green, blue );
        led_color( red, green, blue );
    }
}
*/

void led_demo(){
    const uint8_t maximum = 255;
    uint8_t red = 0, green = 0, blue = 0;
    double angle_red = 0, angle_green = 0, angle_blue = 0, angle_inc = M_PI / 200;
    char aux = 0;

    printf( "\n\n Send any character to start. " );
    while( scanf("%c", &aux) != 1 )
        delay_tick();

    while( true ){ // this code tests the buil-in led
        angle_red += angle_inc;
        if( angle_red >  MAX_ANGLE)
            angle_red -= MAX_ANGLE;
        angle_green = angle_red + ( ( 2 * M_PI ) / 3 );
        angle_blue = angle_red - ( ( 2 * M_PI ) / 3 );

        red = maximum * ( ( sin( angle_red ) + 1 ) / 2 );
        green = maximum * ( ( sin( angle_green ) + 1 ) / 2 );
        blue = maximum * ( ( sin( angle_blue ) + 1 ) / 2 );

        printf( " red: %d green %d blue: %d \n", red, green, blue );
        led_color( red, green, blue );
        delay_milli( 10 );
    }
}
