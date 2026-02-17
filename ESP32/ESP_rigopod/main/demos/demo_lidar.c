#include "main.h"

/* MODULE PINOUT

    PIN 6 (RED):    VCC             --> +5V
    PIN 5 (BLACK):  GROUND          --> GND
    PIN 4 (WHITE):  UART RX         --> ESP 4 (UART2 TX)
    PIN 3 (BLUE):   UART TX         --> ESP 5 (UART2 RX)
    PIN 2 (GREEN):  GPIO/DATA READY --> N.C.
    PIN 1 (YELLOW): MODE            --> +3V3
*/

void lidar_demo(){
    #define DATASET 50
    
    // uint8_t registers[ 21 ] = {0, 1, 2, 3, 4, 5, 6, 7, 64, 65, 66 ,86, 87, 22, 23, 24, 25, 26, 59, 60, 61};
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
    uint8_t i;
    int16_t measurements[ DATASET ] = {0}, minimum, maximum;
    float variance, standard_deviation, mean;

    printf( "\n" );
    while( true ){
        minimum = 30000;
        maximum = 0;
        mean = 0;
        variance = 0;
        standard_deviation = 0;
        printf( "starting measurements.\n" );
        for( i = 0; i < DATASET; i++ ){
            // acquire data and check for error messages
            measurements[ i ] = 0;
            while( measurements[ i ] < 1 ){
                measurements[ i ] = xts1_measure_distance();
                if( measurements[ i ] < 0 )
                    printf( "  --> %s\n", distance_diag[ measurements[ i ] + 13 ] );
            }
            // check for minimum
            if( measurements[ i ] < minimum )
                minimum = measurements[ i ];
            // check for maximum
            if( measurements[ i ] > maximum )
                maximum = measurements[ i ];
            /* calculating mean dynamically
            if(i == 0){
                mean = measurements[ i ];
            }
            else{
                mean *= i;
                mean += measurements[ i ];
                mean /= i + 1;
            }
            */
            delay_milli( 20 ); // maximum sample rate = 50Hz
        }
        // calculating mean
        for( i = 0; i < DATASET; i++ )
            mean += measurements[ i ];
        mean /= DATASET;
        // calculating variance
        for( i = 0; i < DATASET; i++ )
            variance +=  pow( measurements[ i ] - mean, 2 );
        variance /= DATASET - 1; // -1 due to Bessel's Correction
        // calculating standard deviation
        standard_deviation = sqrt( variance );

        printf( " minimum = %5d \n", minimum );
        printf( " maximum = %5d \n", maximum );
        printf( " mean = %f \n", mean );
        printf( " variance = %f \n", variance );
        printf( " standard deviation = %f \n\n", standard_deviation );
    }
}
