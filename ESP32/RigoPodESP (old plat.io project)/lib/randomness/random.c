#include "random.h"

// ========== GLOBAL VARIABLES ==========

static const char *TAG = "RANDOM";   // esp_err variable

// ============ EXTERNAL FUNCTIONS ============

void random_setup(){
    bootloader_random_enable();
    ESP_LOGI( TAG, "Randomness Initialized" );
}

uint32_t random_32(){
    return esp_random();
}

bool random_1(){
    bool state = false;

    if( ( esp_random() & 1 ) == 1 )
        state = true;

    return state;
}

uint8_t random_n( uint32_t limit ){
    return esp_random() % limit;
}
