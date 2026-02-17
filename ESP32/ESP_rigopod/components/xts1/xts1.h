#ifndef __XTS1_H
#define __XTS1_H

/* MODULE PINOUT

    PIN 6 (RED):    VCC             --> +5V
    PIN 5 (BLACK):  GROUND          --> GND
    PIN 4 (WHITE):  UART RX         --> ESP 4 (UART2 TX)
    PIN 3 (BLUE):   UART TX         --> ESP 5 (UART2 RX)
    PIN 2 (GREEN):  GPIO/DATA READY --> N.C.
    PIN 1 (YELLOW): MODE            --> +3V3
*/

// ============ ESP IDF LIBRARIES ============

#include <stdio.h>
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdbool.h>
#include <stdint.h>

// ============ DEFINITIONS ============

#define TIMEOUT         12      // modbus comms timeout in system tick (12ms for tickrate of 1000Hz)
#define READ_TIMEOUT_MS 10
#define BYTE_MASK       0xFF

#define CRC_DATA_LENGTH 6
#define CRC_INIT        0xFFFF
#define CRC_POLY        0xA001

#define UART_BAUD_RATE  115200
#define UART_TX_PIN     4
#define UART_RX_PIN     5

#define MODBUS_DEVICE_ID        0x01
#define MODBUS_FRAME_SIZE_WRITE 8
#define MODBUS_FRAME_SIZE_READ  7
#define MODBUS_FRAME_SIZE_SYS   9
#define MODBUS_FC_READ_HOLDING  0x03
#define MODBUS_FC_READ_INPUT    0x04
#define MODBUS_FC_WRITE_SINGLE  0x06

#define XTS1_WREG_CURR_SEL   0
#define XTS1_WREG_INT_TIME   1
#define XTS1_WREG_DLL_TIME   2
#define XTS1_WREG_MEAS_COM   3
#define XTS1_WREG_REG_OPER   4
#define XTS1_WREG_REG_ADDR   5
#define XTS1_WREG_FLASH_CO   6
#define XTS1_WREG_W_PROTEC   7

#define XTS1_RREG_SYS_ERRO   16

#define XTS1_RREG_CURR_INT  22
#define XTS1_RREG_DIST_INF  23
#define XTS1_RREG_TEMPERAT  24
#define XTS1_RREG_SIG_AMPL  25
#define XTS1_RREG_BACK_LIT  26

#define XTS1_RREG_WAFER_ID  59
#define XTS1_RREG_CHIP__ID  60
#define XTS1_RREG_SERIAL_O  61

#define XTS1_WREG_I2C_ADDR  64
#define XTS1_WREG_OPE_MODE  65
#define XTS1_WREG_MEAS_PER  66

#define XTS1_WREG_UFS_AMPV  86
#define XTS1_WREG_UFS_AMPB  87

// ============ MACROS ============

#define XTS1_IS_REG_HOLDING(idx) \
      ( ( idx == XTS1_WREG_CURR_SEL ) || \
        ( idx == XTS1_WREG_INT_TIME ) || \
        ( idx == XTS1_WREG_DLL_TIME ) || \
        ( idx == XTS1_WREG_MEAS_COM ) || \
        ( idx == XTS1_WREG_REG_OPER ) || \
        ( idx == XTS1_WREG_REG_ADDR ) || \
        ( idx == XTS1_WREG_FLASH_CO ) || \
        ( idx == XTS1_WREG_W_PROTEC ) || \
        ( idx == XTS1_WREG_I2C_ADDR ) || \
        ( idx == XTS1_WREG_OPE_MODE ) || \
        ( idx == XTS1_WREG_MEAS_PER ) || \
        ( idx == XTS1_WREG_UFS_AMPV ) || \
        ( idx == XTS1_WREG_UFS_AMPB ) )

#define XTS1_IS_REG_INPUT(idx) \
      ( ( idx == XTS1_RREG_CURR_INT ) || \
        ( idx == XTS1_RREG_DIST_INF ) || \
        ( idx == XTS1_RREG_TEMPERAT ) || \
        ( idx == XTS1_RREG_SIG_AMPL ) || \
        ( idx == XTS1_RREG_BACK_LIT ) || \
        ( idx == XTS1_RREG_WAFER_ID ) || \
        ( idx == XTS1_RREG_CHIP__ID ) || \
        ( idx == XTS1_RREG_SERIAL_O ) )

// ============ EXTERNAL FUNCTIONS ============

void xts1_setup();

//esp_err_t xts1_write_register( uint16_t address_index, uint16_t value );

//esp_err_t xts1_read_register( uint16_t address_index, uint16_t *value );

//esp_err_t xts1_sys_error( uint32_t *value );

uint16_t xts1_measure_distance();

#endif /* __XTS1_H */