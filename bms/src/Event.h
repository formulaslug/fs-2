#pragma once

#include "BmsConfig.h"

#include <array>
#include <cstdint>
#include <optional>
#include <stdint.h>

#include "mbed.h"
#include "rtos.h"
#include "Mail.h"

enum class BmsEventType : uint16_t {
  VoltageMeasurement,
  TemperatureMeasurement
};

class BmsEvent {
public:
  uint16_t voltageValues[BMS_BANK_COUNT * BMS_BANK_CELL_COUNT];
  int8_t temperatureValues[BMS_BANK_COUNT * BMS_BANK_TEMP_COUNT];
  BMSThreadState bmsState;
};

static constexpr auto mailboxSize = 4;
using BmsEventMailbox = Queue<BmsEvent, mailboxSize>;

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
