/* TickRate increased to 1000Hz
    CONFIG_FREERTOS_HZ=1000 line from CONFIG_FREERTOS_HZ=100 in the platformio sdkconfig
*/
#include "main.h"
#include "demos.h"

// ========== DEFINITIONS ==========

#define PROGRAM     led_demo();//calibration_process(); // from demos.h

// ========== GLOBAL VARIABLES ==========

uint32_t time_counter = 0;  // global timer

// ========== MAIN PROGRAM ==========

// core 0 asynchronous task
void task_core0(){
    PROGRAM
}

// ============ CORE FUNCTIONS ============

// core 1 timer interrupt subroutine
bool IRAM_ATTR timer_core1( gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx ){
    // makes it easy to measure interruption time
    gpio_set_level( SYNCPIN1, 1 );

    time_counter++;

    // makes it easy to measure interruption time
    gpio_set_level( SYNCPIN1, 0 );
    return true; // return true to yield for ISR callback to continue
}

// interrupt timer configuration for core 1 interrupt
void timer_core1_setup(){
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
    gpio_config( &outputs );
    gpio_set_level( SYNCPIN1, 0 );

    // timer and alarm commands
    gptimer_new_timer( &timer_config, &timer );
    gptimer_register_event_callbacks( timer, &cbs, NULL );
    gptimer_set_alarm_action( timer, &alarm_config );
    gptimer_enable( timer );
    gptimer_start( timer );
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
void core1Task( void* parameter ){
    timer_core1_setup();

    //task_core1(); TASK 1 DISABLED. CORE 1 USED ONLY FOR INTERRUPT

    // core 1 task termination
    vTaskDelete( NULL );
}

 // RUNS ON CORE 0
void app_main(){
    delay_milli( 1000 );
    led_setup();
    random_setup();
    xts1_setup();
    servo_setup();
    //timer_core0_setup(); CORE 0 TIMER INTERRUPT DISABLED

    // core 1 task creation
    xTaskCreatePinnedToCore( core1Task, "timer1Creator", 10000, NULL, 1, NULL, 1 );
    
    delay_milli( 1000 ); // gives time to finish the logging
    task_core0();

    // core 0 task termination
    vTaskDelete( NULL );
}