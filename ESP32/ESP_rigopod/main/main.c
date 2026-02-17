/* IMPORTANT MENUCONFIG SETTINGS

    CONFIG_FREERTOS_HZ = 1000
    I²C, UART functions ==> IRAM safe
*/

#include "main.h"

// ========== GLOBAL VARIABLES ==========

uint32_t time_counter = 0;  // global timer

// ========== MAIN PROGRAM ==========

void task_core0(){
    //led_demo();
    //lidar_demo();
    servo_demo();
    while( 1 ){
        delay_milli(1000);
    }
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
        .on_alarm = timer_core1, // setting the callback for alarm event
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
    while(1){
        delay_milli(1000);
    }
}
*/

 // RUNS ON CORE 1
void core1Task( void* parameter ){
    // sets up the core 1 interrupt config
    timer_core1_setup();

    //task_core1(); TASK 1 DISABLED. CORE 1 USED ONLY FOR INTERRUPT

    // core 1 task termination
    vTaskDelete( NULL );
}

 // RUNS ON CORE 0
void app_main(){
    delay_milli( 1000 );
    //serial_setup();
    led_setup();
    //xts1_setup();
    servo_setup();
    //timer_core0_setup(); CORE 0 TIMER INTERRUPT DISABLED

    printf("\n\nInitialization complete!\n");
    //fflush(stdout);

    // core 1 task creation
    xTaskCreatePinnedToCore( core1Task, "timer1Creator", 10000, NULL, 1, NULL, 1 );

    // core 0 task creation
    task_core0();

    // core 0 task termination
    vTaskDelete( NULL );
    //esp_restart();
}
