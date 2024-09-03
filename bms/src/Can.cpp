#include "Can.h"
#include <cstdint>
#include <cstdio>

CANMessage accBoardBootup() {
  uint8_t startupMessage[8];
  return CANMessage(kNMT_ACC_HEARTBEAT, startupMessage);
}

CANMessage accBoardState(uint8_t glvVoltage, uint16_t tsVoltage, bool bmsFault,
                         bool bmsBalancing, bool prechargeDone, bool charging,
                         bool fansOn, bool shutdownClosed, bool unused_A,
                         bool unused_B, uint8_t maxCellTemp,
                         uint8_t avgCellTemp, int16_t tsCurrent) {
  uint8_t data[8];
  data[0] = glvVoltage;
  data[1] = tsVoltage;
  data[2] = tsVoltage >> 8;
  data[3] = bmsFault + (bmsBalancing << 1) + (prechargeDone << 2) +
            (charging << 3) + (fansOn << 4) + (shutdownClosed << 5) +
            (unused_A << 6) + (unused_B << 7);
  data[4] = maxCellTemp;
  data[5] = avgCellTemp;
  data[6] = tsCurrent;
  data[7] = tsCurrent >> 8;
  return CANMessage(kTPDO_ACC_BOARD_State, data);
}

CANMessage accBoardTemp(uint8_t segment, int8_t *temps) {
  uint8_t data[7];
  uint32_t id;
  switch (segment) {
  case 0:
    id = kTPDO_ACC_BOARD_Temp_0;
    break;
  case 1:
    id = kTPDO_ACC_BOARD_Temp_1;
    break;
  case 2:
    id = kTPDO_ACC_BOARD_Temp_2;
    break;
  case 3:
    id = kTPDO_ACC_BOARD_Temp_3;
    break;
  }
  for (int i = 0; i < 7; i++) {
    data[i] = (uint8_t)(temps[i]);
  }

  return CANMessage(id, data);
}

CANMessage accBoardVolt(uint8_t segment, uint16_t *volts) {
  uint8_t data[7];
  uint32_t id;
  switch (segment) {
  case 0:
    id = kTPDO_ACC_BOARD_Volt_0;
    break;
  case 1:
    id = kTPDO_ACC_BOARD_Volt_1;
    break;
  case 2:
    id = kTPDO_ACC_BOARD_Volt_2;
    break;
  case 3:
    id = kTPDO_ACC_BOARD_Volt_3;
    break;
  }
  for (int i = 0; i < 7; i++) {
    data[i] = (uint8_t)(50.0 * volts[i] / 1000.0);
  }

  return CANMessage(id, data);
}

CANMessage motorControllerCurrentLim(uint16_t chargeCurLim,
                                     uint16_t dischargeCurLim) {
  uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  data[0] = chargeCurLim;
  data[1] = chargeCurLim >> 8;
  data[2] = dischargeCurLim;
  data[3] = dischargeCurLim >> 8;
  return CANMessage(kRPDO_MAX_CURRENTS, data);
}

CANMessage chargerChargeControlRPDO(uint8_t destinationNodeID,
                                    uint32_t packVoltage, bool evseOverride,
                                    bool current10xMultiplier, bool enable) {
  uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  data[0] = destinationNodeID;
  data[1] = (enable << 1) | (current10xMultiplier << 3) | (evseOverride << 5);
  data[2] = packVoltage;
  data[3] = packVoltage >> 8;
  data[4] = packVoltage >> 16;
  data[5] = packVoltage >> 24;
  return CANMessage(kRPDO_ChargeControl, data);
}

CANMessage
chargerMaxAllowedVoltageCurrentRPDO(uint8_t destinationNodeID,
                                    uint32_t maxAllowedChargeVoltage,
                                    uint16_t maxAllowedChargeCurrent,
                                    uint8_t maxAllowedInputCurrentEVSEoverride) {
    uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    data[0] = destinationNodeID;
    data[1] = maxAllowedChargeVoltage;
    data[2] = maxAllowedChargeVoltage >> 8;
    data[3] = maxAllowedChargeVoltage >> 16;
    data[4] = maxAllowedChargeVoltage >> 24;
    data[5] = maxAllowedChargeCurrent;
    data[6] = maxAllowedChargeCurrent >> 8;
    data[7] = maxAllowedInputCurrentEVSEoverride;
    return CANMessage(kRPDO_ChargeLimits, data);
}