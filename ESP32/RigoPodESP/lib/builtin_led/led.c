/* it used to be bit banging, which is horrible
// from https://github.com/espressif/ESP8266_RTOS_SDK/issues/914

#include <driver/gpio.h>
#include <driver/dedic_gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "delay.h"

static dedic_gpio_bundle_handle_t gpio_bundle = NULL; // hardware handle

static inline void led_gpio_fast_set(void){
    dedic_gpio_bundle_write(gpio_bundle, 1, 1);
}

static inline void led_gpio_fast_reset(void){
    dedic_gpio_bundle_write(gpio_bundle, 1, 0);
}

static inline void IRAM_ATTR ws2812b_bit_0(){
    led_gpio_fast_set();
    //delay ~ 0.3875µs
    asm("nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop;");
    led_gpio_fast_reset();
    //delay ~ 0.8375µs
    asm("nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop;");
}

static inline void IRAM_ATTR ws2812b_bit_1(){
    led_gpio_fast_set();
    //delay ~ 0.6625µs
    asm("nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop;");
    led_gpio_fast_reset();
    //delay ~ 0.5625us
    asm("nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;");
}

static inline void IRAM_ATTR ws2812b_byte(uint8_t byte){
    register volatile uint8_t i;

    for(i = 0; i < 8; i++) {
        if(byte & 0x80)
            ws2812b_bit_1();
        else
            ws2812b_bit_0();
        byte <<= 1;
    }
    delay_micro(50);
}

// Mandatory setup initialization function
void led_setup_(void){
    dedic_gpio_bundle_config_t bundle_config = {
        .gpio_array = (int[]) {48},
        .array_size = 1,
        .flags = {
            .in_en = 0,
            .out_en = 1,
        },
    };

    ESP_ERROR_CHECK(dedic_gpio_new_bundle(&bundle_config, &gpio_bundle));
    led_color(0, 0, 0);
    ESP_LOGI(TAG, "LED initialized");
}

// CRITICAL SECTION SUBROUTINE, MUST RUN ON CORE 0
void IRAM_ATTR led_color(uint8_t r, uint8_t g, uint8_t b){
    taskDISABLE_INTERRUPTS(); // Critical Section to preserve timing
    ws2812b_byte(g);
    ws2812b_byte(r);
    ws2812b_byte(b);
    taskENABLE_INTERRUPTS();
}
*/

#include "led.h"

// Define the GPIO pin connected to the LED
#define LED_GPIO_PIN        48
#define RMT_RESOLUTION_HZ   40000000

// WS2812B timings for a 40MHz clock source (25ns period), we are going to have:
#define TOH 16  // t0H = 400ns - 16
#define TOL 34  // t0L = 850ns - 34
#define T1H 32  // t1H = 800ns - 32
#define T1L 22  // t1L = 450ns - 18

static const char *TAG = "WS2812B"; // esp_err variable
rmt_encoder_handle_t bytes_encoder = NULL;
rmt_channel_handle_t tx_chan = NULL;

void led_color(uint8_t r, uint8_t g, uint8_t b){
    uint8_t payload[3] = {g, r, b};
    rmt_transmit_config_t tx_config = {
        .loop_count = 0,    // no transfer loop
    };

    ESP_ERROR_CHECK(rmt_transmit(tx_chan, bytes_encoder, payload, 3, &tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(tx_chan, 100));
}

void led_setup(void){
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,   // select source clock
        .gpio_num = LED_GPIO_PIN,         // GPIO number
        .mem_block_symbols = 64,          // memory block size, 64 * 4 = 256 Bytes
        .resolution_hz = 40 * 1000 * 1000,// 40 MHz tick resolution, i.e., 1 tick = 25ns
        .trans_queue_depth = 4,           // set the number of transactions that can pend in the background
        .flags.invert_out = false,        // do not invert output signal
        .flags.with_dma = false,          // do not need DMA backend,
    };
    static const rmt_symbol_word_t ws2812_zero = {
        .level0 = 1,
        .duration0 = TOH,
        .level1 = 0,
        .duration1 = TOL,
    };
    static const rmt_symbol_word_t ws2812_one = {
        .level0 = 1,
        .duration0 = T1H,
        .level1 = 0,
        .duration1 = T1L,
    };
    const rmt_bytes_encoder_config_t encoder_config = {
        .bit0 = ws2812_zero,
        .bit1 = ws2812_one,
        .flags.msb_first = true,
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &tx_chan));
    ESP_ERROR_CHECK(rmt_new_bytes_encoder(&encoder_config, &bytes_encoder));
    ESP_ERROR_CHECK(rmt_enable(tx_chan));
    ESP_LOGI(TAG, "RMT DRIVER ENABLED");

    led_color(0, 0, 0);
}
