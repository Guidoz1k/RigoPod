#include "main.h"

// ========== GLOBAL VARIABLES ==========

uint32_t time_counter = 0;  // global timer

enum demo_run_t{            // demo program to run
    DEMO_LED = 1,
    DEMO_SERVO = 2,
    DEMO_LIDAR = 3,
    NO_DEMO = 4, // runs program normally executing main_program
} demo_run = DEMO_LED;

// ========== MAIN PROGRAM AND DEMOS ==========

void led_demo(void){
    const uint8_t maximum = 100;
    uint8_t red = 0, green = 2 * maximum / 3, blue = 2 * maximum / 3;
    bool red_status = true, green_status = true, blue_status = false;

    while(1){ // this code tests the buil-in led
        delay_milli(10);

        if(red_status == true)
            if(red < maximum)
                red++;
            else
                red_status = false;
        else
            if(red > 0)
                red--;
            else
                red_status = true;

        if(green_status == true)
            if(green < maximum)
                green++;
            else
                green_status = false;
        else
            if(green > 0)
                green--;
            else
                green_status = true;

        if(blue_status == true)
            if(blue < maximum)
                blue++;
            else
                blue_status = false;
        else
            if(blue > 0)
                blue--;
            else
                blue_status = true;

        serial_write_string(" red: ", false);
        serial_write_byte(red, DEC, false);
        serial_write_string(" green: ", false);
        serial_write_byte(green, DEC, false);
        serial_write_string(" blue: ", false);
        serial_write_byte(blue, DEC, true);
        led_color(red, green, blue);
    }
}

/* Processes the manual control of the servo for tests and calibration
    All numerical entries will be comprised of '+' or '-' followed by a 4 digits number, ranging from 0 to 9000
    first char: command letter
        S = set servo position, followed by numerical entry
        X = calibrate the maximum PWM value of chosen servo, followed by numerical entry
        N = calibrate the minimum PWM value of chosen servo, followed by numerical entry
        R = read any calibration parameter of chosen servo, followed by:
            N = minimum PWM value
            X = maximum PWM value
            A = angle set
    second char: servo
        P = pitch servo
        R = roll servo
    following chars: command dependent
*/
void servo_demo(void){
    char buffer[8] = {0};
    servo_t servo = SERVO_PITCH;
    int16_t value = 0;

    while(1){ // this codes reads serial and controls servos
        while(serial_read_size() < 7)
            delay_milli(10);
        delay_milli(100);

        serial_write_string("\n COMMAND: ", false);
        if(serial_read_chars(buffer, 7) != 7){
            serial_write_string(" ERROR!", true);
        }
        else{
            if( (buffer[1] == 'P') || (buffer[1] == 'R') ){
                if(buffer[1] == 'P'){
                    servo = SERVO_PITCH;
                }
                else{
                    servo = SERVO_ROLL;
                }
                switch(buffer[0]){
                case 'S':
                    value = servo_string_to_number(&buffer[2]);
                    if(value != 0x7FFF){
                        serial_write_string("SET ", false);
                        serial_write_char(buffer[1], false, false);
                        serial_write_string(" to ", false);
                        serial_write_word(value, false);
                        servo_pos(value, servo);
                    }
                    else{
                        serial_write_string(" INVALID POSITION", true);
                    }
                    break;
                case 'X':
                    value = servo_string_to_number(&buffer[2]);
                    if(value != 0x7FFF){
                        serial_write_string("CALIBRATED MAX of ", false);
                        serial_write_char(buffer[1], false, false);
                        serial_write_string(" to ", false);
                        serial_write_word(value, false);
                        servo_cal(0, value, servo);
                    }
                    else{
                        serial_write_string(" INVALID CALIBRATION", true);
                    }                    
                    break;
                case 'N':
                    value = servo_string_to_number(&buffer[2]);
                    if(value != 0x7FFF){
                        serial_write_string("CALIBRATED MIN of ", false);
                        serial_write_char(buffer[1], false, false);
                        serial_write_string(" to ", false);
                        serial_write_word(value, false);
                        servo_cal(value, 0, servo);
                    }
                    else{
                        serial_write_string(" INVALID CALIBRATION", true);
                    } 
                    break;
                case 'R':
                    switch(buffer[2]){
                    case 'X':
                        serial_write_string("RANGE MAX of ", false);
                        serial_write_char(buffer[1], false, false);
                        serial_write_string(" to ", false);
                        if(buffer[1] == 'P'){
                            serial_write_word(servo_return_cal(PITCH_MAX), false);
                        }
                        else{
                            serial_write_word(servo_return_cal(ROLL_MAX), false);
                        }
                        break;
                    case 'N':
                        serial_write_string("RANGE MIN of ", false);
                        serial_write_char(buffer[1], false, false);
                        serial_write_string(" to ", false);
                        if(buffer[1] == 'P'){
                            serial_write_word(servo_return_cal(PITCH_MIN), false);
                        }
                        else{
                            serial_write_word(servo_return_cal(ROLL_MIN), false);
                        }
                        break;
                    case 'A':
                        serial_write_string("RANGE ANGLE of ", false);
                        serial_write_char(buffer[1], false, false);
                        serial_write_string(" to ", false);
                        if(buffer[1] == 'P'){
                            serial_write_word(servo_return_cal(PITCH_ANG), false);
                        }
                        else{
                            serial_write_word(servo_return_cal(ROLL_ANG), false);
                        }
                        break;
                    default:
                        serial_write_string(" INVALID REGISTER", true);
                        break;
                    }
                    break;
                default:
                    serial_write_string(" INVALID COMMAND", true);
                    break;
                }
            }
            else{
                serial_write_string(" INVALID SERVO", true);
            }
        }
        serial_flush();
    }
}

void lidar_demo(void){
    uint8_t buffer[8] = {0};
    uint32_t time_now = 0;

    while(1){ // this code tests the lidar
        serial_new_line();
        modbus_read_register(0x0017, 0x0001, INPUT_REG);
        time_now = time_counter;
        serial_write_string("msg sent", true);
        while(modbus_check_buffer() != 7)
            delay_tick();
        serial_write_string("elapsed time: ", false);
        serial_write_word(time_counter - time_now, true);
        modbus_read_buffer(buffer, 8);
        serial_write_string("distance: ", false);
        serial_write_word((buffer[3] << 8) + buffer[4], true);
        delay_milli(100);
    }
}

void main_program(void){
    while(1){
        delay_tick();
    }
}

// ============ CORE FUNCTIONS ============

// core 0 asynchronous task
void task_core0(void){
    switch(demo_run){
    case DEMO_LED:
        led_demo();
        break;
    case DEMO_SERVO:
        servo_demo();
        break;
    case DEMO_LIDAR:
        lidar_demo();
        break;
    case NO_DEMO:
        main_program();
        break;
    default:
        break;
    }
}

// core 1 timer interrupt subroutine
bool IRAM_ATTR timer_core1(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN1, 1);

    time_counter++;

    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN1, 0);
    return true; // return true to yield for ISR callback to continue
}

// interrupt timer configuration for core 1 interrupt
void timer_core1_setup(void){
    // GPIO config variables
    gpio_config_t outputs = {
        .pin_bit_mask = (uint64_t)1 << SYNCPIN1,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    // timer and alarm variables
    gptimer_handle_t timer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, // use default clock source
        .direction = GPTIMER_COUNT_UP,      // count up
        .resolution_hz = 1000000,           // 1 MHz, 1 tick = 1 us
    };
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_core1, // setting the  callback for alarm event for BASE and DRONE systems
    };
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = PERIOD1,             // 1 millisecond
        .reload_count = 0,                  // no initial reload
        .flags.auto_reload_on_alarm = true, // automatically reload counter on alarm
    };

    // GPIO config commands
    gpio_config(&outputs);
    gpio_set_level(SYNCPIN1, 0);

    // timer and alarm commands
    gptimer_new_timer(&timer_config, &timer);
    gptimer_register_event_callbacks(timer, &cbs, NULL);
    gptimer_set_alarm_action(timer, &alarm_config);
    gptimer_enable(timer);
    gptimer_start(timer);
}

/* CORE 0 TIMER INTERRUPT DISABLED
// core 0 timer interrupt subroutine
bool IRAM_ATTR timer_core0(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN0, 1);

    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN0, 0);
    return true; // return true to yield for ISR callback to continue
}

// interrupt timer configuration for core 0 interrupt
void timer_core0_setup(void){
    // GPIO config variables
    gpio_config_t outputs = {
        .pin_bit_mask = (uint64_t)1 << SYNCPIN0,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    // timer and alarm variables
    gptimer_handle_t timer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, // use default clock source
        .direction = GPTIMER_COUNT_UP,      // count up
        .resolution_hz = 1000000,           // 1 MHz, 1 tick = 1 us
    };
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_core0, // set callback for alarm event
    };
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = PERIOD0,             // 10 microseconds
        .reload_count = 0,                  // no initial reload
        .flags.auto_reload_on_alarm = true, // automatically reload counter on alarm
    };

    // GPIO config commands
    gpio_config(&outputs);
    gpio_set_level(SYNCPIN0, 0);

    // timer and alarm commands
    gptimer_new_timer(&timer_config, &timer);
    gptimer_register_event_callbacks(timer, &cbs, NULL);
    gptimer_set_alarm_action(timer, &alarm_config);
    gptimer_enable(timer);
    gptimer_start(timer);
}
*/

/*TASK 1 DISABLED. CORE 1 USED ONLY FOR INTERRUPT
// core 1 asynchronous task
void task_core1(void){
    #ifdef ENV_BASE
        while(1){
            delay_milli(1000);
        }
    #elif ENV_DRONE
        while(1){
            delay_milli(1000);
        }
    #endif
}
*/

 // RUNS ON CORE 1
void core1Task(void* parameter){
    timer_core1_setup();

    //task_core1(); TASK 1 DISABLED. CORE 1 USED ONLY FOR INTERRUPT

    // core 1 task termination
    vTaskDelete(NULL);
}

 // RUNS ON CORE 0
void app_main(){
    delay_milli(1000);
    led_setup();
    serial_setup();
    random_setup();
    //xts1_setup();
    servo_setup();
    //timer_core0_setup(); CORE 0 TIMER INTERRUPT DISABLED

    // core 1 task creation
    xTaskCreatePinnedToCore(core1Task, "timer1Creator", 10000, NULL, 1, NULL, 1);
    
    delay_milli(1000); // gives time to finish the logging
    task_core0();

    // core 0 task termination
    vTaskDelete(NULL);
}