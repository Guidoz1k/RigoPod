#include "demos.h"

char buffer[ 8 ] = {0};
servo_t servo = SERVO_PITCH;

void clear_buffer(){
    uint8_t i = 0;

    for(i = 0; i < 8; i++){
        buffer[ i ] = 0;
    }
    
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

    clear_buffer();
    printf( "\n ANGLE: " );
    while( scanf( "%c%c%c%c%c", &buffer[ 0 ], &buffer[ 1 ], &buffer[ 2 ], &buffer[ 3 ], &buffer[ 4 ] ) != 5 )
        delay_milli( 10 );

    if( ( ( buffer[0] == '+' ) || ( buffer[0] == '-' ) ) &&
        ( ( buffer[1] >= '0' ) && ( buffer[1] <= '9' ) ) &&
        ( ( buffer[2] >= '0' ) && ( buffer[2] <= '9' ) ) &&
        ( ( buffer[3] >= '0' ) && ( buffer[3] <= '9' ) ) &&
        ( ( buffer[4] >= '0' ) && ( buffer[4] <= '9' ) )
        ){
        value = buffer[ 4 ] - 48;
        value += ( buffer[ 3 ] - 48 ) * 10;
        value += ( buffer[ 2 ] - 48 ) * 100;
        value += ( buffer[ 1 ] - 48 ) * 1000;
        if( buffer[ 0 ] == '-' )
            value *= -1;
        
        if( ( value >= -9000 ) && ( value <= 9000 ) ){
            servo_set_angle( value, servo );
            printf( " DONE " );
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

    clear_buffer();
    printf( "\n PWM MAX: " );
    while( scanf( "%c%c%c%c%c", &buffer[ 0 ], &buffer[ 1 ], &buffer[ 2 ], &buffer[ 3 ], &buffer[ 4 ] ) != 5 )
        delay_milli( 10 );

    if( ( ( buffer[0] >= '0' ) && ( buffer[0] <= '9' ) ) &&
        ( ( buffer[1] >= '0' ) && ( buffer[1] <= '9' ) ) &&
        ( ( buffer[2] >= '0' ) && ( buffer[2] <= '9' ) ) &&
        ( ( buffer[3] >= '0' ) && ( buffer[3] <= '9' ) ) &&
        ( ( buffer[4] >= '0' ) && ( buffer[4] <= '9' ) )
        ){
        value = buffer[ 4 ] - 48;
        value += ( buffer[ 3 ] - 48 ) * 10;
        value += ( buffer[ 2 ] - 48 ) * 100;
        value += ( buffer[ 1 ] - 48 ) * 1000;
        if( buffer[ 0 ] == '-' )
            value *= -1;
        
        if( ( value >= 0 ) && ( value <= PMW_MAX ) ){
            servo_set_pwm_max( value, servo );
            printf( " DONE " );
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

    clear_buffer();
    printf( "\n PWM MIN: " );
    while( scanf( "%c%c%c%c%c", &buffer[ 0 ], &buffer[ 1 ], &buffer[ 2 ], &buffer[ 3 ], &buffer[ 4 ] ) != 5 )
        delay_milli( 10 );

    if( ( ( buffer[0] >= '0' ) && ( buffer[0] <= '9' ) ) &&
        ( ( buffer[1] >= '0' ) && ( buffer[1] <= '9' ) ) &&
        ( ( buffer[2] >= '0' ) && ( buffer[2] <= '9' ) ) &&
        ( ( buffer[3] >= '0' ) && ( buffer[3] <= '9' ) ) &&
        ( ( buffer[4] >= '0' ) && ( buffer[4] <= '9' ) )
        ){
        value = buffer[ 4 ] - 48;
        value += ( buffer[ 3 ] - 48 ) * 10;
        value += ( buffer[ 2 ] - 48 ) * 100;
        value += ( buffer[ 1 ] - 48 ) * 1000;
        if( buffer[ 0 ] == '-' )
            value *= -1;
        
        if( ( value >= 0 ) && ( value <= PMW_MAX ) ){
            servo_set_pwm_min( value, servo );
            printf( " DONE " );
        }
        else
            printf( "INVALID RANGE" );
    }
    else{
        printf( "INVALID CALIBRATION" );
    }
}

void reading_values(){
    clear_buffer();
    printf( "\n READING: " );
    while( scanf( "%c", buffer ) != 1 )
        delay_milli( 10 );

    switch( buffer[ 0 ] ){
    case 'A':
        printf( " Angle = %d ", servo_return_angle( servo ) );
        break;
    case 'X':
        printf( " Maximum PWM = %d ", servo_return_pwm_max( servo )  );
        break;
    case 'N':
        printf( " Minimum PWM = %d ", servo_return_pwm_min( servo )  );
        break;
    default:
        printf( "INVALID COMMAND" );
        break;
    }
}

void setting_function(){
    clear_buffer();
    printf( "\n FUNCTION: " );
    while( scanf( "%c", buffer ) != 1 )
        delay_milli( 10 );

    switch( buffer[ 0 ] ){
    case 'A':
        printf( " Angle config " );
        setting_angle();
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
    clear_buffer();
    printf( "\n\n SERVO: " );
    while( scanf( "%c", buffer ) != 1 )
        delay_milli( 10 );

    switch( buffer[ 0 ] ){
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


