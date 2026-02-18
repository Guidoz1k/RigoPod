#include "demos.h"

void servo_control(char first_char, char second_char, char *buffer){
    int16_t value = 0;
    servo_t servo = SERVO_PITCH;

    if( second_char == 'R' )
        servo = SERVO_ROLL;
    switch(first_char){
        case 'S':
            value = servo_string_to_number( buffer );
            if( value != 0x7FFF ){
                printf( "SET %c to %d", second_char, value );
                servo_pos( value, servo );
            }
            else
                printf( "INVALID POSITION" );
            break;
        case 'X':
            value = servo_string_to_number( buffer );
            if( value != 0x7FFF ){
                printf( "CALIBRATED MAX of %c to %d", second_char, value );
                servo_cal( 0, value, servo );
            }
            else
                printf( " INVALID CALIBRATION" ); 
            break;
        case 'N':
            value = servo_string_to_number( buffer );
            if( value != 0x7FFF ){
                printf( "CALIBRATED MIN of %c to %d", second_char, value );
                servo_cal( value, 0, servo );
            }
            else
                printf( " INVALID CALIBRATION" );
            break;
        case 'R':
            switch( buffer[ 0 ] ){
            case 'X':
                printf( "RANGE MAX of %c is ", second_char );
                if( second_char == 'P' )
                    printf( "%d", servo_return_cal( PITCH_MAX ) );
                else
                    printf( "%d", servo_return_cal( ROLL_MAX ) );
                break;
            case 'N':
                printf( "RANGE MIN of %c is ", second_char );
                if( second_char == 'P' )
                    printf( "%d", servo_return_cal( PITCH_MIN ) );
                else
                    printf( "%d", servo_return_cal( ROLL_MIN ) );
                break;
            case 'A':
                printf( "RANGE ANGLE of %c is ", second_char );
                if( second_char == 'P' )
                    printf( "%d", servo_return_cal( PITCH_ANG ) );
                else
                    printf( "%d", servo_return_cal( ROLL_ANG ) );
                break;
            default:
                printf( "INVALID REGISTER" );
                break;
            }
            break;
        default:
            break;
    }
}

void calibration_process(){
    // servos variables
    char char_command, char_servo, buf[ 5 ];
    int16_t value = 0;

    while( true ){
        value = 0;
        char_servo = 0;
        char_command = 0;
        printf( "\n COMMAND: " );
        while( value < 1 ){
            delay_milli( 100 );
            value = scanf( "%c", &char_command);
        }

        if( char_command == 'L' )
            char_command = 0;//lidar_statistics();
        else{
            scanf("%c", &char_servo);
            if( ( char_servo == 'P' ) || ( char_servo == 'R' ) ){
                if( char_command == 'R' ){
                    if( scanf("%c", buf) != -1 )
                        servo_control(char_command, char_servo, buf);
                    else
                        printf( "INVALID COMMAND" );
                }
                else{
                    if( char_command == 'S' ||
                        char_command == 'X' ||
                        char_command == 'N'
                        ){
                        if( scanf("%c%c%c%c%c", buf, &buf[ 1 ], &buf[ 2 ], &buf[ 3 ], &buf[ 4 ]) != -1 )
                            servo_control(char_command, char_servo, buf);
                        else
                            printf( "INVALID COMMAND" );
                    }
                    else
                        printf( "INVALID COMMAND" );
                }
            }
            else
                printf( "INVALID SERVO" );
        }
        while( scanf("%c", buf) != -1); // fflush doesn't work
    }
}

