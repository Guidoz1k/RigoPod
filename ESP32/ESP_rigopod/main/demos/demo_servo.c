#include "demo_servo.h"

#define INT_ERROR   -0x8000
#define ABS(x)  (((x)<0)?(-(x)):(x))

servo_t servo = SERVO_PITCH;

// converts series of chars to int16_t, including signal as first char
int16_t buffer_to_int( char *buffer, bool is_signed, uint8_t size){
    int16_t value = 0;
    int16_t decimal = 1;
    int16_t signal = 1;
    uint8_t i = 0;

    if( ( is_signed == true ) && ( buffer[ 0 ] != '+' ) && ( buffer[ 0 ] != '-' ) )
        value = INT_ERROR;
    else{
        if( is_signed == true ){
            if( buffer[ 0 ] == '-' )
                signal = -1;
            buffer++;
        }
        while( ( i < size ) && ( buffer[ size - 1 - i ] >= '0' ) && ( buffer[ size - 1 - i ] <= '9' ) ){
            value += ( buffer[ size - 1 - i ] - 48 ) * decimal;
            decimal *= 10;
            i++;
        }
        value *= signal;
    }

    return value;
}

char* print_ang(int16_t angle){
    static char buffer[ 8 ] = { '+', '0', '0', '.', '0', '0', '\0', '\0' };

    if( angle < 0 )
        buffer[ 0 ] = '-';
    sprintf( (char *)&buffer[ 1 ], "%02d.%02d", ABS(angle) / 100, (uint8_t)( ABS(angle) % 100 ) );

    return buffer;
}

void print_help(){
    printf( " Welcome to the servo demo \n\n" );
    printf( " First select your servo:\n" );
    printf( "    P for pitch servo\n" );
    printf( "    R for roll servo\n\n" );
    printf( " Then select your function:\n\n" );
    printf( " Setting angle [A]: [signal][4 chars number]\n" );
    printf( "     signal = + or - depending of direction\n" );
    printf( "     number = from +9000 to -9000, representing 90 degrees\n\n" );
    printf( " Adjusting PWM zero [Z]: [signal][3 chars number]\n" );
    printf( "     signal = + or - depending of direction\n" );
    printf( "     number = from +256 to -256 by each adjustment\n\n" );
    printf( " Calibrating maximum PWM [X]: [5 chars number]\n" );
    printf( "     number = 5 characters for PWM\n\n" );
    printf( " Calibrating minimum PWM [N]: [5 chars number]\n" );
    printf( "     number = 5 characters for PWM\n\n" );
    printf( " Reading values [R]: [values]\n" );
    printf( "     values = A for angle, X for max PWM, N for minimum PWM\n\n" );
    printf( " You can also send the entire command like \"RA+9000\" for setting Roll Angle as 90deg\n" );
    printf( " \n" );
}

void setting_angle(){
    int16_t value = 0;
    char buffer[ 5 ] = {0};

    fflush(stdin);
    printf( "\n ANGLE: " );
    while( scanf( "%c%c%c%c%c", &buffer[ 0 ], &buffer[ 1 ], &buffer[ 2 ], &buffer[ 3 ], &buffer[ 4 ] ) != 5 )
        delay_milli( 10 );

    value = buffer_to_int( buffer, true, 4 );
    if( value != INT_ERROR ){
        if( ( value >= N_HALF_ANGLE ) && ( value <= HALF_ANGLE ) ){
            servo_set_angle( value, servo );
            printf( " %s angle DONE ", print_ang( value ) );
        }
        else
            printf( "INVALID RANGE" );
    }
    else{
        printf( "INVALID POSITION" );
    }
}

void setting_bias(){
    int16_t value = 0;
    char buffer[ 4 ] = {0};

    fflush(stdin);
    printf( "\n ZERO BIAS: " );
    while( scanf( "%c%c%c%c", &buffer[ 0 ], &buffer[ 1 ], &buffer[ 2 ], &buffer[ 3 ] ) != 4 )
        delay_milli( 10 );

    value = buffer_to_int( buffer, true, 3 );
    if( value != INT_ERROR ){
        if( ( value >= PWM_DEC_MAX ) && ( value <= PWM_INC_MAX ) ){
            servo_move_pwm( value, servo );
            printf( " %+04d PWM BIAS DONE ", value );
        }
        else
            printf( "INVALID RANGE" );
    }
    else{
        printf( "INVALID POSITION" );
    }
}

void setting_max(){
    int16_t value = 0;
    char buffer[ 5 ] = {0};

    fflush(stdin);
    printf( "\n PWM MAX: " );
    while( scanf( "%c%c%c%c%c", &buffer[ 0 ], &buffer[ 1 ], &buffer[ 2 ], &buffer[ 3 ], &buffer[ 4 ] ) != 5 )
        delay_milli( 10 );

    value = buffer_to_int( buffer, false, 5 );
    if( value != INT_ERROR ){
        if( ( value > servo_return_pwm_min( servo ) ) && ( value <= PWM_MAX ) ){
            servo_set_pwm_max( value, servo );
            printf( " %04d PWM MAX DONE ", value );
        }
        else
            printf( "INVALID RANGE" );
    }
    else{
        printf( "INVALID CALIBRATION" );
    }
}

void setting_min(){
    int16_t value = 0;
    char buffer[ 5 ] = {0};

    fflush(stdin);
    printf( "\n PWM MIN: " );
    while( scanf( "%c%c%c%c%c", &buffer[ 0 ], &buffer[ 1 ], &buffer[ 2 ], &buffer[ 3 ], &buffer[ 4 ] ) != 5 )
        delay_milli( 10 );

    value = buffer_to_int( buffer, false, 5 );
    if( value != INT_ERROR ){
        if( ( value >= PWM_MIN ) && ( value < servo_return_pwm_max( servo ) ) ){
            servo_set_pwm_min( value, servo );
            printf( " %04d PWM MIN DONE ", value );
        }
        else
            printf( "INVALID RANGE" );
    }
    else{
        printf( "INVALID CALIBRATION" );
    }
}

void reading_values(){
    char buffer = 0;

    fflush(stdin);
    printf( "\n READING: " );
    while( scanf( "%c", (char *)&buffer ) != 1 )
        delay_milli( 10 );

    switch( buffer ){
    case 'A':
        printf( " Angle = %s ", print_ang( servo_return_angle( servo ) ) );
        break;
    case 'X':
        printf( " Maximum PWM = %05d ", servo_return_pwm_max( servo )  );
        break;
    case 'N':
        printf( " Minimum PWM = %05d ", servo_return_pwm_min( servo )  );
        break;
    default:
        printf( "INVALID COMMAND" );
        break;
    }
}

void setting_function(){
    char buffer = 0;

    fflush(stdin);
    printf( "\n FUNCTION: " );
    while( scanf( "%c", (char *)&buffer ) != 1 )
        delay_milli( 10 );

    switch( buffer ){
    case 'A':
        printf( " Angle config " );
        setting_angle();
        break;
    case 'Z':
        printf( " Zero config " );
        setting_bias();
        break;
    case 'X':
        printf( " Maximum PWM config " );
        setting_max();
        break;
    case 'N':
        printf( " Minimum PWM config " );
        setting_min();
        break;
    case 'R':
        printf( " Value reading " );
        reading_values();
        break;
    default:
        printf( "INVALID COMMAND" );
        break;
    }
}

void setting_servo(){
    char buffer = 0;

    fflush(stdin);
    printf( "\n\n SERVO: " );
    while( scanf( "%c", (char *)&buffer ) != 1 )
        delay_milli( 10 );

    switch( buffer ){
        case 'P':
            servo = SERVO_PITCH;
            printf( " Pitch Servo selected" );
            setting_function();
            break;
        case 'R':
            servo = SERVO_ROLL;
            printf( " Roll Servo selected" );
            setting_function();
            break;
        default:
            printf( "INVALID SERVO\n" );
            break;
    }
}

void servo_demo(){
    print_help();

    while( true ){ // this codes reads serial and controls servos
        // scanf("%s", buffer) is harder to use
        // it continuously reads the buffer, returning -1, only returning 1 when it reads anything
        // if it reads a string smaller than desired size, it still returns 1, letting waste to remain in buffer

        setting_servo();
        delay_tick();
    }
}


