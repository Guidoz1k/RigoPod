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

// ========== GLOBAL TYPES ==========

// system mode state variable type for the FSM
typedef enum _mode_t{
    MODE_IDLE = 0,
    MODE_WAITING = 1,
    MODE_SERVO_ERROR = 2,
    MODE_TOF_ERROR = 3,
    MODE_COMM_ERROR = 4,
    MODE_ACTIVE = 5,
    MODE_GIMBAL = 6,
    MODE_TOF = 7,
} system_mode_t;

// ========== GLOBAL VARIABLES ==========

system_mode_t system_mode = MODE_IDLE;  // current FSM state
extern uint32_t time_counter;           // global timer updated in core 2 every 1µs (main.c)

const int16_t max_angle = 1500;         // [cents fixed point] servo does 0.13s per 60deg @5V → 15.38deg per cycle @30Hz
int16_t current_distance = 0;                   // [cents fixed point] ToF sensor input data for the FSM events
int16_t current_pitch = 0, current_roll = 0;    // [cents fixed point] gimbal control data for the FSM events
int16_t command_pitch = 0, command_roll = 0;    // [cents fixed point] gimbal control data from UART input

// ========== DATA-DRIVEN OUTPUT VARIABLES ==========

// data-driven led rgb combination by mode
const struct _led_color_{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rgb_pattern[ 8 ] = {
    { 0,        0,          LED_MAX }, // MODE_IDLE         (blue)
    { LED_MAX,  0,          0       }, // MODE_WAITING      (red)
    { LED_MIN,  LED_MIN,    0       }, // MODE_SERVO_ERROR  (orange)
    { LED_MIN,  0,          LED_MIN }, // MODE_TOF_ERROR    (magenta)
    { 0,        LED_MIN,    LED_MIN }, // MODE_COMM_ERROR   (cyan)
    { 0,        LED_MAX,    0       }, // MODE_ACTIVE       (green)
    { 0,        LED_MAX,    0       }, // MODE_GIMBAL       (green)
    { 0,        LED_MAX,    0       }, // MODE_TOF          (green)
};

// data-driven UART packet combination by mode
/* COMMS standard

    reading ToF:        'R' → 'R'+'ToF byte 1'+'ToF byte 0'
    setting gimbal:     'G'+'pitch byte 1'+'pitch byte 0'+'roll byte 1'+'roll byte 0' → 'G'
    active parallax:    'A'+'pitch byte 1'+'pitch byte 0'+'roll byte 1'+'roll byte 0' → 'A'+'ToF byte 1'+'ToF byte 0'
    serial input error: → 'E'
    gimbal error:       → 'S'
    tof error:          → 'T'

    bytes with '1' are written over by rangefinding function
*/
uint8_t uart_response[ 8 ][ 3 ] = {
    {   0,   0,   0 }, // MODE_IDLE         (blue)
    {   0,   0,   0 }, // MODE_WAITING      (red)
    { 'S',   0,   0 }, // MODE_SERVO_ERROR  (orange)
    { 'T',   0,   0 }, // MODE_TOF_ERROR    (magenta)
    { 'E',   0,   0 }, // MODE_COMM_ERROR   (cyan)
    { 'A',   1,   1 }, // MODE_ACTIVE       (green)
    { 'G',   0,   0 }, // MODE_GIMBAL       (green)
    { 'R',   1,   1 }, // MODE_TOF          (green)
};

// ========== EVENT VARIABLES ==========

// serial request event type (includes gimbal event as well)
enum _uart_in_t{
    SERIAL_IN_EMPTY = 0,
    SERIAL_IN_GIMBAL = 'G',
    SERIAL_IN_TOF = 'R',
    SERIAL_IN_ACTIVE = 'A',
    SERIAL_IN_ERROR = 4,
    SERIAL_IN_GIMBAL_ERROR = 5,
} uart_event = 0;

enum _tof_reading_t{
    TOF_VALID = 0,
    TOF_INVALID = 1,
} tof_event = 0;

// ============ PERIPHERAL FUNCTIONS ============

// PERIPHERAL gimbal: translate coordinates to the gimbal standard zero and axis orientation
void inverse_kinematics( int16_t pitch, int16_t roll ){
    int16_t set_pitch = - current_pitch;
    int16_t set_roll = - current_roll;

    servo_set_angle( set_pitch, SERVO_PITCH );
    servo_set_angle( set_roll, SERVO_ROLL );
}

// compensate for 3D printing tolerances
void servo_calib(){
    servo_move_pwm( PITCH_SERVO_CALIB_BIAS, SERVO_PITCH );
    servo_set_pwm_min( ROLL_SERVO_CALIB_MIN, SERVO_ROLL );
}

// checks for the gimbal's angular maximum traverse
bool gimbal_range(){
    float x_coord = command_roll - current_roll;          // x component of change vector
    float y_coord = command_pitch - current_pitch;        // y component of change vector
    float vector_mag = sqrtf( x_coord * x_coord + y_coord * y_coord ); // change vector magnitude
    bool return_value = false;

    if( vector_mag < max_angle ){
        return_value = true;
        current_pitch = command_pitch;
        current_roll = command_roll;
    }
    else{
        current_roll = (int16_t)( max_angle * ( x_coord / vector_mag ) ); // maximum magnitude multiplied by normalized vector
        current_pitch = (int16_t)( max_angle * ( y_coord / vector_mag ) ); // maximum magnitude multiplied by normalized vector
    }

    return return_value;
}

// PERIPHERAL ToF sensor: report all erros as one, else reports distance value
void rangefinding(){
    int16_t response = 0;

    if( xts1_measure_distance( &response ) < 0 ){
        response = 0;
        tof_event = TOF_INVALID;
    }
    else
        tof_event = TOF_VALID;

    uart_response[ MODE_ACTIVE ][ 1 ] = response >> 8;
    uart_response[ MODE_TOF ][ 1 ] = response >> 8;
    uart_response[ MODE_ACTIVE ][ 0 ] = response && 0xFF;
    uart_response[ MODE_TOF ][ 0 ] = response && 0xFF;
}

// PERIPHERAL UART: check for messages from drone system
void uart_request(){
    uint8_t value = SERIAL_IN_EMPTY;
    uint8_t packet = 0;
    uint8_t value_byte[ 2 ] = { 0 };

    if( serial_read_size() != 0 ){          // byte available at UART
        packet = serial_read_singlechar();
        if( packet == SERIAL_IN_TOF )       // single 'R' in buffer
            value = packet;
        else
            if( ( packet == SERIAL_IN_GIMBAL ) || ( packet == SERIAL_IN_ACTIVE ) )
                if( serial_read_chars( (char*)value_byte, 2 ) == 2 ){
                    command_pitch = ( value_byte[ 0 ] << 8 ) + value_byte[ 1 ];
                    if( serial_read_chars( (char*)value_byte, 2 ) == 2 ){
                        command_roll = ( value_byte[ 0 ] << 8 ) + value_byte[ 1 ];

                        if( gimbal_range() == true )
                            value = packet;
                        else
                            value = SERIAL_IN_GIMBAL_ERROR;
                    }
                    else
                        value = SERIAL_IN_ERROR; // couldn't read 2 bytes out of the buffer
                }
                else
                    value = SERIAL_IN_ERROR; // couldn't read 2 bytes out of the buffer
            else
                value = SERIAL_IN_ERROR; // Not 'R', 'G', or 'A'
    }

    serial_flush();
    uart_event = value;
}

// ============  FSM FUNCTIONS ============

// function to set the RGB led according to current mode
void mode_transition( system_mode_t next_mode ){
    led_color( rgb_pattern[ next_mode ].red, rgb_pattern[ next_mode ].green, rgb_pattern[ next_mode ].blue );
    serial_write_bytes( uart_response[ next_mode ], 3 );
    inverse_kinematics( command_pitch, command_roll );
    system_mode = next_mode;
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

// ============ MAIN FUNCTIONS ============

void main_task(){
    uart_request();
    rangefinding();

    switch( system_mode ){
    case MODE_IDLE:
        switch(uart_event){
        case SERIAL_IN_GIMBAL:
            mode_transition( MODE_GIMBAL );
            break;
        case SERIAL_IN_TOF:
            if( tof_event == TOF_VALID )
                mode_transition( MODE_TOF );
            else
                mode_transition( MODE_TOF_ERROR );
            break;
        case SERIAL_IN_ACTIVE:
            if( tof_event == TOF_VALID )
                mode_transition( MODE_ACTIVE );
            else
                mode_transition( MODE_TOF_ERROR );
            break;
        case SERIAL_IN_ERROR:
            mode_transition( MODE_COMM_ERROR );
            break;
        case SERIAL_IN_GIMBAL_ERROR:
            mode_transition( MODE_SERVO_ERROR );
            break;
        case SERIAL_IN_EMPTY:
            // nothing :)
            break;
        }
        break;
    case MODE_WAITING:
        // @@@@@@ needs timeout and serial parsing
        break;
    case MODE_COMM_ERROR:
        // @@@@@@ needs timeout and serial parsing
        break;
    case MODE_ACTIVE:
        switch(uart_event){
        case SERIAL_IN_GIMBAL:
        case SERIAL_IN_TOF:
            mode_transition( MODE_IDLE ); // drone system would not make this request
            break;
        case SERIAL_IN_ACTIVE:
            if( tof_event == TOF_VALID )
                mode_transition( MODE_ACTIVE );
            else
                mode_transition( MODE_TOF_ERROR );
            break;
        case SERIAL_IN_ERROR:
            mode_transition( MODE_COMM_ERROR );
            break;
        case SERIAL_IN_GIMBAL_ERROR:
            mode_transition( MODE_SERVO_ERROR );
            break;
        case SERIAL_IN_EMPTY:
            // @@@@@@ needs timeout
            break;
        }
        break;
    case MODE_SERVO_ERROR:  // ACTIVE or GIMBAL modes, SERVO_ERROR sends back to idle
    case MODE_TOF_ERROR:    // ACTIVE or TOF modes, TOF_ERROR sends back to idle
    case MODE_GIMBAL:       // executes once and back to idle
    case MODE_TOF:          // executes once and back to idle
        mode_transition( MODE_IDLE ); // executes once and back to idle
        break;
    default:
        break;
    }
}

void test_routine(){
    servo_calib();

    led_color( 0, 0, LED_MAX );
    inverse_kinematics( 0, 0 );

    while( 1 ){
        main_task();
        delay_milli( 10 );
        //delay_tick();
    }
}
