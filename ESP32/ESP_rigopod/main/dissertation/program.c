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

// ========== GLOBAL VARIABLES ==========

extern uint32_t time_counter;  // global timer updated in core 2 every 1µs (main.c)

const uint8_t max_angle = 1500; // servo does 0.13s per 60deg @5V → 15.38deg per cycle @30Hz

typedef enum _mode_t{
    MODE_IDLE = 0,
    MODE_WAITING = 1,
    MODE_SERVO_ERROR = 2,
    MODE_TOF_ERROR = 3,
    MODE_COMM_ERROR = 4,
    MODE_ACTIVE = 5,
} system_mode_t;
typedef enum _comms_t{
    COMMS_IDLE = 0,
    COMMS_DIRECTION = 1,
    COMMS_TOF = 2,
    COMMS_PARALLAX = 3,
    COMMS_ERROR = 4,
} drone_command_t;
typedef struct _drone_t_{
    uint8_t function;
    uint8_t data_hi;
    uint8_t data_lo;
} drone_package_t;

/*

- A producer pushes an event to the queue.
Event producers: the UART ISR, parser, timer, ToF driver and gimbal controller. They only generate events.

- The main loop pops it and runs the switch on current_state
FSM core: the main switch and transition_to(). This is where decisions are made and state changes happen.
The main FSM switch: switch(current_state) and inside each case you handle the incoming event and decide where to go. This is where the transition logic lives.
When it decides a change is needed, it calls transition_to(NEW_STATE).

- The switch decides the next state and calls transition_to()
transition_to(): this does the same steps for every state, so it mostly doesn’t need a switch at all. Entry actions come from the descriptor table, indexed by the new state:
    set LED to table[new_state].led_color
    set gimbal mode from table[new_state].gimbal_mode
    send table[new_state].uart_msg if it isn’t null
    arm the timeout if table[new_state].timeout is nonzero

- transition_to() looks up the table and calls the output functions (LED, gimbal, UART) to apply the new state’s behavior.
Actions/outputs: the LED driver, gimbal control and UART TX. Only the FSM core calls these.

*/


// ============ FUNCTIONS ============

// translate coordinates to the gimbal standard zero and axis orientation
void inverse_kinematics( int16_t pitch, int16_t roll ){
    int16_t set_pitch = -pitch;
    int16_t set_roll = -roll;

    servo_set_angle( set_pitch, SERVO_PITCH );
    servo_set_angle( set_roll, SERVO_ROLL );
}

// compensate for 3D printing tolerances
void servo_calib(){
    servo_move_pwm( PITCH_SERVO_CALIB_BIAS, SERVO_PITCH );
    servo_set_pwm_min( ROLL_SERVO_CALIB_MIN, SERVO_ROLL );
}

// function to set the RGB led according to current mode
void update_led( system_mode_t system_mode ){
    switch( system_mode ){
    case MODE_IDLE:
        led_color( 0, 0, LED_MAX );
        break;
    case MODE_WAITING:
        led_color( LED_MAX, 0, 0 );
        break;
    case MODE_SERVO_ERROR:
        led_color( LED_MIN, LED_MIN, 0 );
        break;
    case MODE_TOF_ERROR:
        led_color( LED_MIN, 0, LED_MIN );
        break;
    case MODE_COMM_ERROR:
        led_color( 0, LED_MIN, LED_MIN );
        break;
    case MODE_ACTIVE:
        led_color( 0, LED_MAX, 0 );
        break;
    default:
        break;
    }
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

drone_command_t drone_comms(){

}

void drone_response( uint8_t ){

}

// ============ MAIN FUNCTIONS ============
/*

*/
void main_task(){
    static system_mode_t system_mode = MODE_IDLE;
    drone_command_t comms = COMMS_IDLE;
    int16_t current_pitch = 0, current_roll = 0;

    comms = drone_comms();

    switch( system_mode ){
    case MODE_IDLE:
        
        break;
    case MODE_WAITING:
        
        break;
    case MODE_SERVO_ERROR:
        
        break;
    case MODE_TOF_ERROR:
        
        break;
    case MODE_COMM_ERROR:
        
        break;
    case MODE_ACTIVE:
        
        break;
    default:
        break;
    }

    update_led( system_mode );
}

void test_routine(){
    servo_calib();
    while( 1 ){
        main_task();
        delay_tick();
    }
}
