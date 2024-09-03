#pragma once

#include "BmsConfig.h"

#include <array>
#include <cstdint>
#include <optional>
#include <stdint.h>

#include "mbed.h"
#include "rtos.h"
#include "Mail.h"

class BmsEvent {
public:
  uint16_t voltageValues[BMS_BANK_COUNT * BMS_BANK_CELL_COUNT];
  int8_t temperatureValues[BMS_BANK_COUNT * BMS_BANK_TEMP_COUNT];
  uint8_t minVolt;
  uint8_t maxVolt;
  int8_t minTemp;
  int8_t maxTemp;
  int8_t avgTemp;
  bool isBalancing;
  BMSThreadState bmsState;
};

class MainToBMSEvent {
public:
  bool balanceAllowed = false;
  bool charging = false;
};

static constexpr auto mailboxSize = 4;
using BmsEventMailbox = Queue<BmsEvent, mailboxSize>;
using MainToBMSMailbox = Queue<MainToBMSEvent, mailboxSize>;

// Measurement
//  - Temp
//  - Voltage
//  - Current

// isospi error
//  - init error
//  - bad pec (recoverable)
// bms error
//  - high voltage
//  - low voltage
//  - high temp
//  - low temp
//  - invalid temp
//  - failed self test

// CommunicationsError
//   can fail to start
//   sd card fail to init
//   isospi fail to init
//   
