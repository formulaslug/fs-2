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
#define BMS_BANK_COUNT 4
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
#define BMS_FAULT_TEMP_THRESHOLD_HIGH 58
#endif

// Upper threshold when fault will be thrown for cell temperature when charging
//
// Units: degrees celcius
#ifndef BMS_FAULT_TEMP_THRESHOLD_CHARING_HIGH
#define BMS_FAULT_TEMP_THRESHOLD_CHARING_HIGH 45
#endif

// Lower threshold when fault will be thrown for cell temperature
//
// Units: degrees celcius
#ifndef BMS_FAULT_TEMP_THRESHOLD_LOW
#define BMS_FAULT_TEMP_THRESHOLD_LOW 0
#endif

// Upper threshold when fault will be thrown for cell voltage
//
// Units: millivolts
#ifndef BMS_FAULT_VOLTAGE_THRESHOLD_HIGH
#define BMS_FAULT_VOLTAGE_THRESHOLD_HIGH 4150
#endif

// Lower threshold when fault will be thrown for cell voltage
//
// Units: millivolts
#ifndef BMS_FAULT_VOLTAGE_THRESHOLD_LOW
#define BMS_FAULT_VOLTAGE_THRESHOLD_LOW 2650
#endif

// Threshold when cells will be discharged when discharging is enabled.
//
// Units: millivolts
#ifndef BMS_DISCHARGE_THRESHOLD
#define BMS_DISCHARGE_THRESHOLD 15
#endif


// Threshold when cells are allowed to balance. should be in upper 3/4 or so of charging (defualt 3900)
//
// Units: millivolts
#ifndef BMS_BALANCE_THRESHOLD
#define BMS_BALANCE_THRESHOLD 3900
#endif

// highest power allowed
#ifndef CAR_MAX_POWER
#define CAR_MAX_POWER 80000
#endif

// percent of highest power allowed (for tolerance)
#ifndef CAR_POWER_PERCENT
#define CAR_POWER_PERCENT 0.95
#endif
// maximum allowed current draw by the motor controller
#ifndef CAR_CURRENT_MAX
#define CAR_CURRENT_MAX 300
#endif

// percent of precharge needed to consider precharging done (and close the +AIR)
#ifndef PRECHARGE_PERCENT
#define PRECHARGE_PERCENT 0.95
#endif

// voltage to charge to
#ifndef CHARGE_VOLTAGE
#define CHARGE_VOLTAGE 112
#endif

// AC wall charging current in A, 12 for NEMA 5-15
#ifndef CHARGE_AC_LIMIT
#define CHARGE_AC_LIMIT 12
#endif

// Charge current, in mA
#ifndef CHARGE_DC_LIMIT
#define CHARGE_DC_LIMIT 15000
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


// Acc board shutdown measurement
//
// To measure the state of the shutdown circuit
#ifndef ACC_SHUTDOWN_MEASURE

#ifdef TARGET_NUCLEO_L432KC
  #define ACC_SHUTDOWN_MEASURE PA_6
#else
  #error "Unknown board for ACC_SHUTDOWN_MEASURE"
#endif

#endif


// Acc board fan control
//
// To be set high or low to enable/disable fans
#ifndef ACC_FAN_CONTROL

#ifdef TARGET_NUCLEO_L432KC
  #define ACC_FAN_CONTROL PA_7
#else
  #error "Unknown board for ACC_FAN_CONTROL"
#endif

#endif


// Acc board current sensor vref read
//
// To measure the vref on the current sensor
#ifndef ACC_BUFFERED_C_VREF

#ifdef TARGET_NUCLEO_L432KC
  #define ACC_BUFFERED_C_VREF PA_3
#else
  #error "Unknown board for ACC_BUFFERED_C_VREF"
#endif

#endif


// Acc board current sensor output read
//
// To measure the output on the current sensor
#ifndef ACC_BUFFERED_C_OUT

#ifdef TARGET_NUCLEO_L432KC
  #define ACC_BUFFERED_C_OUT PA_1
#else
  #error "Unknown board for ACC_BUFFERED_C_OUT"
#endif

#endif


// Acc board GLV voltage read
//
// To measure the voltage of the GLV system
#ifndef ACC_GLV_VOLTAGE

#ifdef TARGET_NUCLEO_L432KC
  #define ACC_GLV_VOLTAGE PA_0
#else
  #error "Unknown board for ACC_GLV_VOLTAGE"
#endif

#endif


// Acc charge enable output
//
// To be set high when charging is allowed
#ifndef ACC_CHARGE_ENABLE

#ifdef TARGET_NUCLEO_L432KC
  #define ACC_CHARGE_ENABLE PA_9
#else
  #error "Unknown board for ACC_CHARGE_ENABLE"
#endif

#endif


// Acc charge state input
//
// To measure if the acc is connected to the charger
#ifndef ACC_CHARGE_STATE

#ifdef TARGET_NUCLEO_L432KC
  #define ACC_CHARGE_STATE PA_10
#else
  #error "Unknown board for ACC_CHARGE_STATE"
#endif

#endif


// Acc IMD status input
//
// To measure the state of the IMD status pin
#ifndef ACC_IMD_STATUS

#ifdef TARGET_NUCLEO_L432KC
  #define ACC_IMD_STATUS PB_0
#else
  #error "Unknown board for ACC_IMD_STATUS"
#endif

#endif


// Acc BMS Fault Output
//
// To be set low when there is a BMS fault
#ifndef ACC_BMS_FAULT

#ifdef TARGET_NUCLEO_L432KC
  #define ACC_BMS_FAULT PB_7
#else
  #error "Unknown board for ACC_BMS_FAULT"
#endif

#endif


// Acc Precharge Control
//
// To be set high when precharging is done to allow the positive AIR to close
#ifndef ACC_PRECHARGE_CONTROL

#ifdef TARGET_NUCLEO_L432KC
  #define ACC_PRECHARGE_CONTROL PB_1
#else
  #error "Unknown board for ACC_PRECHARGE_CONTROL"
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
  #define BMS_PIN_SPI_MOSI PA_7
#elif TARGET_NUCLEO_L432KC
  #define BMS_PIN_SPI_MOSI PB_5
#else
  #error "Unknown board for BMS_PIN_SPI_MOSI"
#endif

#endif

// SPI master in slave out
#ifndef BMS_PIN_SPI_MISO

#ifdef TARGET_LPC1768
  #define BMS_PIN_SPI_MISO p6
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_SPI_MISO PA_6
#elif TARGET_NUCLEO_L432KC
  #define BMS_PIN_SPI_MISO PB_4
#else
  #error "Unknown board for BMS_PIN_SPI_MISO"
#endif

#endif

// SPI clock
#ifndef BMS_PIN_SPI_SCLK

#ifdef TARGET_LPC1768
  #define BMS_PIN_SPI_SCLK p7
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_SPI_SCLK PA_5
#elif TARGET_NUCLEO_L432KC
  #define BMS_PIN_SPI_SCLK PB_3
#else
  #error "Unknown board for BMS_PIN_SPI_SCLK"
#endif

#endif

// SPI chip select
#ifndef BMS_PIN_SPI_SSEL

#ifdef TARGET_LPC1768
  #define BMS_PIN_SPI_SSEL p8
#elif TARGET_NUCLEO_F303K8
  #define BMS_PIN_SPI_SSEL PA_4
#elif TARGET_NUCLEO_L432KC
  #define BMS_PIN_SPI_SSEL PA_4
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


enum class BMSThreadState {
    // BMS startup and self test
    //   Run self test on all banks
    //   If OK, go to BMSIdle
    BMSStartup,

    // BMS in idle mode
    //   no faults and cells are being actively monitored
    //
    //   wait for either faults or a signal to move in to charging state
    BMSIdle,

    // potential BMS fault, move to BMS fault if persists for another BMS thread cycle
    BMSFaultRecover,

    // BMS in failure mode
    BMSFault
};