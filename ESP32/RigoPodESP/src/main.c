/* ABOUT

*/

// purely for visual relief
#include "main.h"

#define MAX 10240
#define MIN 2048
#define STEP 5

uint32_t time_counter = 0;

// core 0 asynchronous task
void task_core0(void){
    uint8_t buffer[2] = {0, 0};
    int16_t request = 0;

    uint8_t buffer2[8] = {0};
    uint32_t time_now = 0;

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

    while(1){ // this code tests the lidar
        serial_new_line();
        modbus_read_register(0x0017, 0x0001, INPUT_REG);
        time_now = time_counter;
        serial_write_string("msg sent", true);
        while(modbus_check_buffer() != 7)
            delay_tick();
        serial_write_string("elapsed time: ", false);
        serial_write_word(time_counter - time_now, 5, true);
        modbus_read_buffer(buffer2, 8);
        serial_write_string("distance: ", false);
        serial_write_word((buffer2[3] << 8) + buffer2[4], 5, true);
        delay_milli(1000);
    }

    while(1){ // this codes reads serial and controls servos
        while(serial_read_size() < 2)
            delay_milli(10);
        delay_milli(100);

        if(serial_read_chars(buffer, 2) != 2){
            serial_write_string("ERR", true);
        }
        else{
            request = (buffer[0] << 8) + buffer[1];
            servo_pos(request, SERVO_PITCH);
        }
        serial_flush();
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

    #ifdef ENV_BASE
        // BASE SYSTEM CODE
    #elif ENV_DRONE
        // DRONE SYSTEM CODE
    #endif

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