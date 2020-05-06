#pragma once

#include "mbed.h"

extern Serial* serial;

// TODO: So there's an issue here.

// BMS Master Configuration

#define BMS_BANK_COUNT 1
#define BMS_BANK_CELL_COUNT 7
#define BMS_BANK_TEMP_COUNT BMS_BANK_CELL_COUNT

#define BMS_FAULT_TEMP_THRESHOLD_HIGH 55
#define BMS_FAULT_TEMP_THRESHOLD_LOW 0

// Threshold when fault will be thrown for cell voltage
//
// Units: millivolts
#define BMS_FAULT_VOLTAGE_THRESHOLD_HIGH 4200
#define BMS_FAULT_VOLTAGE_THRESHOLD_LOW 2550

// Threshold when cells will be discharged when discharging is enabled.
//
// Units: millivolts
#define BMS_DISCHARGE_THRESHOLD 15

// IO Configuration

/*
#define LINE_BMS_FLT PAL_LINE(GPIOB, 0)
#define LINE_BMS_FLT_LAT PAL_LINE(GPIOF, 1)
#define LINE_IMD_STATUS PAL_LINE(GPIOF, 0)
#define LINE_IMD_FLT_LAT PAL_LINE(GPIOA, 8)
#define LINE_CHARGER_CONTROL PAL_LINE(GPIOB, 1)
#define LINE_SIG_CURRENT LINE_ARD_A0 // => (GPIOA, 0)

// SPI Configuration

#define BMS_SPI_DRIVER SPID1
#define BMS_SPI_CR1 (SPI_CR1_BR_2 | SPI_CR1_BR_1)  // Prescalar of 128
#define BMS_SPI_CR2 (SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0)  // 8 bits data

#define LINE_SPI_MOSI PAL_LINE(GPIOA, 7)
#define LINE_SPI_MISO PAL_LINE(GPIOA, 6)
#define LINE_SPI_SCLK PAL_LINE(GPIOA, 5)
#define LINE_SPI_SSEL PAL_LINE(GPIOA, 4)

// CAN Configuration
#define BMS_CAN_DRIVER CAND1
#define BMS_CAN_CR1 CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP
#define BMS_CAN_CR2 CAN_BTR_SJW(1) | CAN_BTR_TS1(5) | CAN_BTR_TS2(0) | CAN_BTR_BRP(2)

#define LINE_CAN_TX PAL_LINE(GPIOA, 12)
#define LINE_CAN_RX PAL_LINE(GPIOA, 11)
*/

// BMS Cell lookup
const int BMS_CELL_MAP[12] = {0, 1, 2, 3, -1, -1, 4, 5, 6, -1, -1, -1};
