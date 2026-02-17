#include "main.h"

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
