#include "serial.h"

// ========== DEFINITIONS ==========

#define MAXSIZE 32
#define MAXWAIT 50  // 10 milliseconds to read buffer

// ========== GLOBAL VARIABLES ==========

static const char *TAG = "UART0";   // esp_err variable
static uint8_t read_timeout = pdMS_TO_TICKS(10);    // UART reading timeout

// ============ EXTERNAL FUNCTIONS ============

void serial_setup(){
    uart_config_t configs = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK( uart_set_pin( UART_NUM_0, 43, 44, -1, -1 ) );
    ESP_ERROR_CHECK( uart_param_config( UART_NUM_0, &configs ) );
    ESP_ERROR_CHECK( uart_driver_install( UART_NUM_0, 2048, 0, 0, NULL, 0 ) );

    ESP_LOGI( TAG, "Serial  initialized" );
}

void serial_write_word( int16_t number, bool newline ){
    uint8_t buffer[ 5 ] = { 48 };
    uint8_t i;
    bool number_detect = false;

    if( number < 0 ){
        uart_write_bytes( UART_NUM_0, "-", 1 );
        number *= -1;
    }
    else
        uart_write_bytes( UART_NUM_0, "+", 1 );

    for( i = 0; i < 5; i++ ){
        buffer[ 4 - i ] = ( number % 10 ) + 48;
        number /= 10;
    }
    i = 0;
    while( ( i < ( 5 - 1 ) ) && ( number_detect == false ) ){
        if( buffer[ i ] == '0' )
            buffer[ i ] = ' ';
        else
            number_detect = true;
        i++;
    }

    uart_write_bytes( UART_NUM_0, buffer, 5 );

    if( newline == true )
        serial_new_line();
}

void serial_write_byte( uint8_t number, base_t base, bool newline ){
    uint8_t i;
    uint8_t size = 0;
    char buffer[ 9 ];

    switch( base ){
    case BIN:
        for( i = 0; i < 8; i++ ){
            buffer[ 7 - i ] = number % 2 + 48;
            number /= 2;
        }
        for( i = 9; i >= 5; i-- )
            buffer[ i ] = buffer[ i - 1 ];
        buffer[ 4 ] = '.';

        size = 9;
        break;
    case DEC:
        buffer[ 0 ] = ( ( number / 100 ) % 10 ) + 48;
        buffer[ 1 ] = ( ( number / 10 ) % 10 ) + 48;
        buffer[ 2 ] = ( number % 10 ) + 48;

        size = 3;
        break;
    case HEX:
        buffer[ 0 ] = '0';
        buffer[ 1 ] = 'x';
        buffer[ 2 ] = ( ( number >> 4 ) & 0x0F ) + 48;
        if( buffer[ 2 ] > ( 9 + 48 ) )
            buffer[ 2 ] += 7;
        buffer[ 3 ] = ( number & 0x0F ) + 48;
        if( buffer[ 3 ] > ( 9 + 48 ) )
            buffer[ 3 ] += 7;

        size = 4;
        break;
    default:
        break;
    }

    if( size != 0 ){
        uart_write_bytes( UART_NUM_0, buffer, size );
        if( newline == true )
            serial_new_line();
    }
}

void serial_write_char( uint8_t character, bool numerical, bool newline ){
    char buffer[ 1 ] = { character };

    if( numerical == true )
        buffer[ 0 ] += 48;

    uart_write_bytes( UART_NUM_0, buffer, 1 );
    if( newline == true )
        serial_new_line();
}

void serial_new_line(){
    char buffer = '\n';

    uart_write_bytes( UART_NUM_0, &buffer, 1 );
}

void serial_write_string( const char *pointer, bool newline ){
	uint8_t counter = 0;
    char buffer[ MAXSIZE + 1 ] = { 0 };

	while( ( counter < MAXSIZE ) && ( *pointer != '\0' ) )
        buffer[ counter++ ] = *( pointer++ );
    if( newline == true )
        buffer[ counter ] = '\n';
    else
        counter--;

    uart_write_bytes( UART_NUM_0, buffer, counter + 1 );
}

uint8_t serial_read_chars( char *buffer, uint8_t size ){
    uint8_t chars_read = 0;

    if( size <= MAXSIZE )
        chars_read = uart_read_bytes( UART_NUM_0, buffer, size, read_timeout );

    return chars_read;
}

char serial_read_singlechar(){
    char uto = 0;

    uart_read_bytes( UART_NUM_0, &uto, 1, read_timeout );

    return uto;
}

uint8_t serial_read_size(){
    size_t size_IDF = 0;
    uint8_t size = 0;

    uart_get_buffered_data_len( UART_NUM_0, &size_IDF );
    size = ( uint8_t )size_IDF;

    return size;
}

void serial_flush(){
    uart_flush( UART_NUM_0 );
}
