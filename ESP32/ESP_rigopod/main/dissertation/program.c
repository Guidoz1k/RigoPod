#include "program.h"

/* NOTES
    This program needs all the peripherals to be connected, specially the ToF
    The ToF will throw an error in case it's not connected
*/

// ========== DEFINITIONS ==========

#define PITCH_SERVO_CALIB_BIAS  -200
#define YAW_SERVO_CALIB_MIN    2100

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

bool active_state_flag = false;         // FSM accessory flag
system_mode_t system_mode = MODE_IDLE;  // current FSM state
extern uint32_t time_counter;           // global timer updated in core 2 every 1µs (main.c)

const int16_t max_angle = 1500;         // [cents fixed point] servo does 0.13s per 60deg @5V → 15.38deg per cycle @30Hz
int16_t distance_measured = 0;
int16_t current_pitch = 0, current_yaw = 0;    // [cents fixed point] gimbal control data for the FSM events
int16_t command_pitch = 0, command_yaw = 0;    // [cents fixed point] gimbal control data from UART input

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
    { '0',   0,   0 }, // MODE_IDLE
    { 'W',   0,   0 }, // MODE_WAITING
    { 'S',   0,   0 }, // MODE_SERVO_ERROR
    { 'T',   0,   0 }, // MODE_TOF_ERROR
    { 'E',   0,   0 }, // MODE_COMM_ERROR
    { 'A',   1,   1 }, // MODE_ACTIVE
    { 'G',   0,   0 }, // MODE_GIMBAL
    { 'R',   1,   1 }, // MODE_TOF
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
void inverse_kinematics(){
    int16_t set_pitch = - current_pitch;
    int16_t set_yaw = - current_yaw;

    servo_set_angle( set_pitch, SERVO_PITCH );
    servo_set_angle( set_yaw, SERVO_YAW );
}

// compensate for 3D printing tolerances
void servo_calib(){
    servo_move_pwm( PITCH_SERVO_CALIB_BIAS, SERVO_PITCH );
    servo_set_pwm_min( YAW_SERVO_CALIB_MIN, SERVO_YAW );
}

// checks for the gimbal's angular maximum traverse
// works for gimbal or active mode, but ACTIVE MODE WILL OVERRIDE ITS DATA
bool gimbal_range(){
    float x_coord = command_yaw - current_yaw;                          // x component of change vector
    float y_coord = command_pitch - current_pitch;                      // y component of change vector
    float vector_mag = sqrtf( x_coord * x_coord + y_coord * y_coord );  // change vector magnitude
    bool return_value = false;

    if( vector_mag < max_angle ){
        return_value = true;
        current_pitch = command_pitch;
        current_yaw = command_yaw;
    }
    else{
        // maximum magnitude multiplied by normalized vector gives you a max vector with the requested direction
        // this vector is added to the current coordinates, to respect the limit of the gimbal
        current_yaw += (int16_t)( max_angle * ( x_coord / vector_mag ) );
        current_pitch += (int16_t)( max_angle * ( y_coord / vector_mag ) );
        uart_event = SERIAL_IN_GIMBAL_ERROR; // OVERRIDES UART EVENT
    }

    return return_value;
}

void write_uart_response( system_mode_t mode, uint16_t first_byte, uint16_t second_byte ){
    uart_response[ mode ][ 1 ] = (uint8_t)first_byte;
    uart_response[ mode ][ 2 ] = (uint8_t)second_byte;
}

// PERIPHERAL ToF sensor: report all erros as one, else reports distance value
void rangefinding(){
    int16_t response = 0;

    //    if( xts1_measure_distance( &response ) < 0 ){
    response = xts1_measure_distance_V2();
    if( response < 0 ){
        response = 0;
        tof_event = TOF_INVALID;
    }
    else
        tof_event = TOF_VALID;

    distance_measured = response;
    // active mode distance is calculated via parallax correction
    write_uart_response( MODE_TOF, response >> 8, response & 0xFF );
}

// PERIPHERAL UART: check for messages from drone system
void uart_request(){
    uint8_t event_value = SERIAL_IN_EMPTY;
    uint8_t packet_0 = 0;
    uint8_t value_byte[ 2 ] = { 0 };

    command_pitch = 0;
    command_yaw = 0;

    if( serial_read_size() != 0 ){          // byte available at UART
        packet_0 = serial_read_singlechar();
        if( packet_0 == SERIAL_IN_TOF )       // single 'R' in buffer
            event_value = packet_0;
        else
            if( ( packet_0 == SERIAL_IN_GIMBAL ) || ( packet_0 == SERIAL_IN_ACTIVE ) )
                if( serial_read_chars( (char*)value_byte, 2 ) == 2 ){
                    command_pitch = ( value_byte[ 0 ] << 8 ) + value_byte[ 1 ];
                    if( serial_read_chars( (char*)value_byte, 2 ) == 2 ){
                        command_yaw = ( value_byte[ 0 ] << 8 ) + value_byte[ 1 ];
                        if( gimbal_range() == true )
                            event_value = packet_0;
                        else
                            event_value = SERIAL_IN_GIMBAL_ERROR;
                    }
                    else
                        event_value = SERIAL_IN_ERROR; // couldn't read 2 bytes out of the buffer
                }
                else
                    event_value = SERIAL_IN_ERROR; // couldn't read 2 bytes out of the buffer
            else
                event_value = SERIAL_IN_ERROR; // Not 'R', 'G', or 'A'
    }

    serial_flush();
    uart_event = event_value;
}

// ============ PARALLAX FUNCTIONS ============

void parallax_correction( bool reset ){
    static int16_t gimbal_pitch = 0;
    static int16_t gimbal_yaw = 0;
    static vector_t gimbal_vector;
    vector_t measured_range, estimate_range, camera_axis, tof_axis, new_gimbal_axis;
    int16_t true_value = 0;

    if( reset == true ){
        gimbal_pitch = command_pitch;
        gimbal_yaw = command_yaw;
        gimbal_vector = vector_gimbal_origin();
    }
    else{ // overrides current_pitch and current_yaw
        tof_axis = vector_new_pol( distance_measured, gimbal_yaw, gimbal_pitch );       // updates the tof's sight vector
        camera_axis = vector_new_pol( distance_measured, command_yaw, command_pitch );  // updates the camera's sight vector
        measured_range = vector_add( gimbal_vector, tof_axis );                         // calculates the vector formed by tof sensor and it's arm into a measured range
        estimate_range = vector_projection( measured_range, camera_axis );              // projects measured range into the camera's sight vector to obtain an estimate range
        true_value = vector_magni( estimate_range );                                    // calculates the magnitude of estimate range to obtain the true distance vector
        write_uart_response( MODE_ACTIVE, true_value >> 8, true_value & 0xFF );            // writes the calculated value into UART

        new_gimbal_axis = vector_sub( camera_axis, gimbal_vector ); // calculates the vector that points from the gimbal's origin to the estimate range
        gimbal_pitch = vector_pitch( new_gimbal_axis );             // calculate the vector's pitch
        gimbal_yaw = vector_yawww ( new_gimbal_axis );              // calculate the vector's yaw
        current_pitch = gimbal_pitch;                               // updates the gimbal's new direction
        current_yaw = gimbal_yaw;                                   // updates the gimbal's new direction
    }
}

// ============  FSM FUNCTIONS ============

// function to detect if the drone system went idle
bool timeout_check( bool reset ){
    static uint32_t time_start = 0;
    bool counter_detect = false;

    if( reset == true )
        time_start = time_counter;
    else{
        if( ( time_counter - time_start ) >= WAIT_TIME) // 1s waiting
            counter_detect = true;
    }

    return counter_detect;
}

// function to set the RGB led according to current mode
void mode_transition( system_mode_t next_mode ){
    bool mode_change = system_mode != next_mode;// resets in case of entering active mode for the first time
    led_color( rgb_pattern[ next_mode ].red, rgb_pattern[ next_mode ].green, rgb_pattern[ next_mode ].blue );

    if( next_mode == MODE_ACTIVE ){
        active_state_flag = true;
        parallax_correction( active_state_flag == false ); // reset only when coming from IDLE
        active_state_flag = true;
    }
    if( next_mode == MODE_WAITING )
        timeout_check( mode_change );

    if( next_mode == MODE_IDLE ){
        active_state_flag = false;
        parallax_correction( true );
    }

    serial_write_bytes( uart_response[ next_mode ], 3 );
    inverse_kinematics(); // remember: active mode overrides gimbal range check
    system_mode = next_mode;
}

// ============ MAIN FUNCTIONS ============

void main_task(){
    uart_request();
    gimbal_range();
    rangefinding();

    switch( system_mode ){
    case MODE_IDLE: // waiting for any request
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
    case MODE_WAITING: // mode exclusive for ACTIVE_MODE waiting loop
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
            if( timeout_check( false ) == true ){ // timeout occurred
                mode_transition( MODE_IDLE );
            }
            // else, just keeps waiting
            break;
        }
        break;
    case MODE_ACTIVE: // chained measurements
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
            mode_transition( MODE_WAITING );
            break;
        }
        break;
    case MODE_COMM_ERROR:   // invalid command
    case MODE_SERVO_ERROR:  // ACTIVE or GIMBAL modes
    case MODE_TOF_ERROR:    // ACTIVE or TOF modes
        if( active_state_flag == true )     // won't disengage active mode
            mode_transition(MODE_ACTIVE );
        else
            mode_transition( MODE_IDLE );
        break;
    case MODE_GIMBAL:
    case MODE_TOF:
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
