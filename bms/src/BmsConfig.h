#pragma once

#include "mbed.h"

// Global pointer to can bus object
//
// This allows for all files to access the can bus output
extern CAN* canBus;

//Global pointer to DigitalOut objects for BMS_FAULT and CHARGER_CONTROL pins
//
//This allows for all files to access BMS_FAULT and CHARGER_CONTROL pins
extern DigitalOut* bmsFault;
extern DigitalOut* chargerControl;

//
// BMS Master Configuration
//

// Number of LTC6811 battery banks to communicate with
#ifndef BMS_BANK_COUNT
#define BMS_BANK_COUNT 1
#endif

// Number of cell voltage readings per LTC6811
#ifndef BMS_BANK_CELL_COUNT
#define BMS_BANK_CELL_COUNT 7
#endif

// Number of cell temperature readings per LTC6811
#ifndef BMS_BANK_TEMP_COUNT
#define BMS_BANK_TEMP_COUNT BMS_BANK_CELL_COUNT
#endif

// Upper threshold when fault will be thrown for cell temperature
//
// Units: degrees celcius
#ifndef BMS_FAULT_TEMP_THRESHOLD_HIGH
#define BMS_FAULT_TEMP_THRESHOLD_HIGH 55
#endif

// Upper threshold when fault will be thrown for cell temperature
//
// Units: degrees celcius
#ifndef BMS_FAULT_TEMP_THRESHOLD_LOW
#define BMS_FAULT_TEMP_THRESHOLD_LOW 0
#endif

// Upper threshold when fault will be thrown for cell voltage
//
// Units: millivolts
#ifndef BMS_FAULT_VOLTAGE_THRESHOLD_HIGH
#define BMS_FAULT_VOLTAGE_THRESHOLD_HIGH 4200
#endif

// Lower threshold when fault will be thrown for cell voltage
//
// Units: millivolts
#ifndef BMS_FAULT_VOLTAGE_THRESHOLD_LOW
#define BMS_FAULT_VOLTAGE_THRESHOLD_LOW 2550
#endif

// Threshold when cells will be discharged when discharging is enabled.
//
// Units: millivolts
#ifndef BMS_DISCHARGE_THRESHOLD
#define BMS_DISCHARGE_THRESHOLD 15
#endif

// BMS Cell lookup
//
// This defines the mapping from LTC6811 pins to cell indicies.
// Values of -1 indicate the pin is not connected.
const int BMS_CELL_MAP[12] = {0, 1, 2, 3, -1, -1, 4, 5, 6, -1, -1, -1};


//
// IO Configuration
//

// BMS fault line
//
// To be set high and held high when software enters fault state
#ifndef BMS_PIN_BMS_FLT

#ifdef TARGET_LPC1768
  #define BMS_PIN_BMS_FLT p10
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_BMS_FLT PB_0
#elif TARGET_NUCLEO_L432KC
  #define BMS_PIN_BMS_FLT PB_7
#else
  #error "Unknown board for BMS_PIN_BMS_FLT"
#endif

#endif




/*
// BMS fault latch
//
// Readback from BMS fault relay to be broadcasted on CAN bus
#ifndef BMS_PIN_BMS_FLT_LAT

#ifdef TARGET_LPC1768
  #define BMS_PIN_BMS_FLT_LAT p12
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_BMS_FLT_LAT PF_1
#else
  #error "Unknown board for BMS_PIN_BMS_FLT_LAT"
#endif

#endif
*/



/*
// IMD status input
//
// Reads PWM output from IMD board
#ifndef BMS_PIN_IMD_STATUS

#ifdef TARGET_LPC1768
  #define BMS_PIN_IMD_STATUS p14
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_IMD_STATUS PF_0
#else
  #error "Unknown board for BMS_PIN_IMD_STATUS"
#endif

#endif
*/



/*
// IMD fault latch
//
// Readback from IMD fault relay to be broadcasted on CAN bus
#ifndef BMS_PIN_IMD_FLT_LAT

#ifdef TARGET_LPC1768
  #define BMS_PIN_IMD_FLT_LAT p13
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_IMD_FLT_LAT PA_8
#else
  #error "Unknown board for BMS_PIN_IMD_FLT_LAT"
#endif

#endif
*/



// Charger output
//
// To be pulled high to enable charger
#ifndef BMS_PIN_CHARGER_CONTROL

#ifdef TARGET_LPC1768
  #define BMS_PIN_CHARGER_CONTROL p11
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_CHARGER_CONTROL PB_1
#elif TARGET_NUCLEO_L432KC
  #define BMS_PIN_CHARGER_CONTROL PA_9
#else
  #error "Unknown board for BMS_PIN_CHARGER_CONTROL"
#endif

#endif

// Current input
//
// Input from analog current sensor
#ifndef BMS_PIN_SIG_CURRENT

#ifdef TARGET_LPC1768
  #define BMS_PIN_SIG_CURRENT p15
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_SIG_CURRENT PA_0
#elif TARGET_NUCLEO_L432KC
  #define BMS_PIN_SIG_CURRENT PA_1
#else
  #error "Unknown board for BMS_PIN_SIG_CURRENT"
#endif

#endif


//
// SPI Configuration
//

// SPI master out slave in
#ifndef BMS_PIN_SPI_MOSI

#ifdef TARGET_LPC1768
  #define BMS_PIN_SPI_MOSI p5
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_SPI_MOSI D11
#elif TARGET_NUCLEO_L432KC
  #define BMS_PIN_SPI_MOSI D11
#else
  #error "Unknown board for BMS_PIN_SPI_MOSI"
#endif

#endif

// SPI master in slave out
#ifndef BMS_PIN_SPI_MISO

#ifdef TARGET_LPC1768
  #define BMS_PIN_SPI_MISO p6
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_SPI_MISO D12
#elif TARGET_NUCLEO_L432KC
  #define BMS_PIN_SPI_MISO D12
#else
  #error "Unknown board for BMS_PIN_SPI_MISO"
#endif

#endif

// SPI clock
#ifndef BMS_PIN_SPI_SCLK

#ifdef TARGET_LPC1768
  #define BMS_PIN_SPI_SCLK p7
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_SPI_SCLK D13
#elif TARGET_NUCLEO_L432KC
  #define BMS_PIN_SPI_SCLK D13
#else
  #error "Unknown board for BMS_PIN_SPI_SCLK"
#endif

#endif

// SPI chip select
#ifndef BMS_PIN_SPI_SSEL

#ifdef TARGET_LPC1768
  #define BMS_PIN_SPI_SSEL p8
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_SPI_SSEL A3
#elif TARGET_NUCLEO_L432KC
  #define BMS_PIN_SPI_SSEL A3
#else
  #error "Unknown board for BMS_PIN_SPI_SSEL"
#endif

#endif


//
// CAN Configuration
//

// CAN TX pin to transceiver
#ifndef BMS_PIN_CAN_TX

#ifdef TARGET_LPC1768
  #define BMS_PIN_CAN_TX p29
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_CAN_TX PA_12
#elif TARGET_NUCLEO_L432KC
  #define BMS_PIN_CAN_TX PA_12
#else
  #error "Unknown board for BMS_PIN_CAN_TX"
#endif

#endif

// CAN RX pin from transceiver
#ifndef BMS_PIN_CAN_RX

#ifdef TARGET_LPC1768
  #define BMS_PIN_CAN_RX p30
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_CAN_RX PA_11
#elif TARGET_NUCLEO_L432KC
  #define BMS_PIN_CAN_RX PA_11
#else
  #error "Unknown board for BMS_PIN_CAN_RX"
#endif

#endif

// CAN frequency to used
// default: 500k
#ifndef BMS_CAN_FREQUENCY
#define BMS_CAN_FREQUENCY 500000
#endif

