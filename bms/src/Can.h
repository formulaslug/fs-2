// Copyright (c) 2018 Formula Slug. All Rights Reserved.

#pragma once

#include <stdint.h>

#include "mbed.h"

// SIDs From Accumulator
constexpr uint32_t kFuncIdHeartBeatAcc = 0x701;
constexpr uint32_t kFuncIdCellStartup = 0x420;
constexpr uint32_t kFuncIdFaultStatus = 0x421;
constexpr uint32_t kFuncIdBmsStat = 0x422;
constexpr uint32_t kFuncIdCellVoltage[7] = {0x423, 0x424, 0x425, 0x426,
                                            0x427, 0x428, 0x429};
constexpr uint32_t kFuncIdCellTempAdc[4] = {0x42a, 0x42b, 0x42c, 0x42d};

CANMessage BMSCellStartup () {
  const char * startupMessage = "SPICYBOI";
  return CANMessage(kFuncIdCellStartup, startupMessage);  
}

CANMessage BMSFaultStatus (uint8_t contactorState, uint8_t faultState, uint8_t warningState, uint8_t SoC, uint16_t DoD, uint16_t totalCurrent) {
  uint8_t data[8];
  data[0] = contactorState;
  data[1] = faultState;
  data[2] = warningState;
  data[3] = SoC;
  data[4] = DoD >> 8;
  data[5] = DoD;
  data[6] = totalCurrent >> 8;
  data[7] = totalCurrent;
  return CANMessage(kFuncIdFaultStatus, data);
}

CANMessage BMSStatMessage (uint16_t totalVoltage, uint16_t maxVoltage, uint16_t minVoltage, uint8_t maxTemp, uint8_t minTemp) {
  uint8_t data[8];
  data[0] = totalVoltage >> 8;
  data[1] = totalVoltage;
  data[2] = maxVoltage >> 8;
  data[3] = maxVoltage;
  data[4] = minVoltage >> 8;
  data[5] = minVoltage;
  data[6] = maxTemp;
  data[7] = minTemp;
  return CANMessage(kFuncIdBmsStat, data);
}

CANMessage BMSVoltageMessage (uint8_t row, uint16_t * voltages) {
  uint8_t data[8];
  for (int i = 0; i < 8; i += 2) {
    data[i] = voltages[i/2] >> 8;
    data[i + 1] = voltages [i/2];
  }
  return CANMessage(kFuncIdCellVoltage[row], data);
}

CANMessage BMSTempMessage (uint8_t row, uint8_t * temps) {
  uint8_t data[7];
  for (int i = 0; i < 7; i++) {
    data[i] = temps[i];
  }
  return CANMessage(kFuncIdCellTempAdc[row], data, 7);
}
