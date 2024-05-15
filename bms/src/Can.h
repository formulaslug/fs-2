// Copyright (c) 2018 Formula Slug. All Rights Reserved.

#ifndef _FS_BMS_SRC_CAN_H_
#define _FS_BMS_SRC_CAN_H_

#include <cstdint>
#include <stdint.h>

#include "mbed.h"

// SIDs From Accumulator
constexpr uint32_t kTPDO_ACC_BOARD_State = 0x182;
constexpr uint32_t kTPDO_ACC_BOARD_Temp_0 = 0x282;
constexpr uint32_t kTPDO_ACC_BOARD_Temp_1 = 0x382;
constexpr uint32_t kTPDO_ACC_BOARD_Temp_2 = 0x482;
constexpr uint32_t kTPDO_ACC_BOARD_Temp_3 = 0x582;
constexpr uint32_t kNMT_ACC_HEARTBEAT = 0x702;

CANMessage AccBoardBootup () {
  const char * startupMessage = 0x00;
  return CANMessage(kNMT_ACC_HEARTBEAT, startupMessage);  
}

CANMessage AccBoardState(uint8_t glvVoltage, uint16_t tsVoltage, bool bmsFault, bool bmsBalancing, bool prechargeDone, bool charging, bool fansOn, bool shutdownClosed, bool unused_A, bool unused_B, uint8_t minCellVoltage, uint8_t maxCellVoltage, int16_t tsCurrent) {
    uint8_t data[8];
    data[0] = glvVoltage;
    data[1] = tsVoltage;
    data[2] = tsVoltage >> 8;
    data[3] = bmsFault + (bmsBalancing << 1) + (prechargeDone << 2) + (charging << 3) + (fansOn << 4) + (shutdownClosed << 5) + (unused_A << 6) +(unused_B << 7);
    data[4] = minCellVoltage;
    data[5] = maxCellVoltage;
    data[6] = tsCurrent;
    data[7] = tsCurrent >> 8;
    return CANMessage(kTPDO_ACC_BOARD_State, data);
}

CANMessage AccBoardTemp(int segment, uint8_t *temps) {
    uint8_t data[8];
    for (int i = 0; i < 7; i++) {
        data[i] = temps[i];
    }
    int canID;
    switch (segment) {
        case 0:
            return kTPDO_ACC_BOARD_Temp_0;
        case 1:
            return kTPDO_ACC_BOARD_Temp_1;
        case 2:
            return kTPDO_ACC_BOARD_Temp_2;
        case 3:
            return kTPDO_ACC_BOARD_Temp_3;
        default:
            return NULL;
            break;
    }
    return CANMessage(canID, data);
}

#endif // _FS_BMS_SRC_CAN_H_
