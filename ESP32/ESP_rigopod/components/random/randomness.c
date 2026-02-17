#include "randomness.h"

/*
    limit = 0 for full 32bits random number
    limit != 0 for limited random number
*/
uint32_t randomness( uint32_t limit ){
    uint32_t result = 0;

    bootloader_random_enable();
    vTaskDelay( 1 );
    if( limit == 0 )
        result = esp_random();
    else
        result = esp_random() % limit;
    bootloader_random_disable();

    return result;
}
