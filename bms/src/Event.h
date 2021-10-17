#pragma once

#include "BmsConfig.h"

#include <array>
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
  virtual BmsEventType getType() const = 0;
  //virtual uint8_t* encodeCAN() const = 0;
  // virtual json encodeJSON() const = 0;
  // serialize to json

  virtual ~BmsEvent() {};
};

static constexpr auto mailboxSize = 1;
using BmsEventMailbox = Queue<BmsEvent, mailboxSize>;

// Measurement
//  - Temp
//  - Voltage
//  - Current

class ErrorEvent : public BmsEvent {};

class VoltageMeasurement : public BmsEvent {
public:
  BmsEventType getType() const { return BmsEventType::VoltageMeasurement; }
  std::array<uint16_t, BMS_BANK_COUNT * BMS_BANK_CELL_COUNT> voltageValues;
};

class TemperatureMeasurement : public BmsEvent {
public:
  BmsEventType getType() const { return BmsEventType::TemperatureMeasurement; }
  std::array<std::optional<int8_t>, BMS_BANK_COUNT * BMS_BANK_TEMP_COUNT> temperatureValues;
};


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
