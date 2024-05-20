// Copyright (c) 2018 Formula Slug. All Rights Reserved.

#ifndef _FS_BMS_SRC_CAN_H_
#define _FS_BMS_SRC_CAN_H_

#include <cstdint>
#include <stdint.h>

#include "mbed.h"

// SIDs From Accumulator
constexpr uint32_t kTPDO_ACC_BOARD_State = 0x182;
constexpr uint32_t kTPDO_ACC_BOARD_Temp = 0x282;
constexpr uint32_t kTPDO_ACC_BOARD_Volt = 0x382;
constexpr uint32_t kNMT_ACC_HEARTBEAT = 0x702;
constexpr uint32_t kRPDO_MAX_CURRENTS = 0x286;

/* Bootup message */
CANMessage accBoardBootup();

/* TPDO that sends various states and information about the accumulator */
CANMessage accBoardState(uint8_t glvVoltage, uint16_t tsVoltage, bool bmsFault, bool bmsBalancing, bool prechargeDone, bool charging, bool fansOn, bool shutdownClosed, bool unused_A, bool unused_B, uint8_t minCellVoltage, uint8_t maxCellVoltage, int16_t tsCurrent);

/* TPDO that sends all temperatures for one segment */
CANMessage accBoardTemp(uint8_t segment, int8_t *temps);

/* TPDO that sends all voltages for one segment */
CANMessage accBoardVolt(uint8_t segment, uint16_t *voltages);

/* RPDO for limiting the current to the Motor Controller (AC-X1) */
CANMessage motorControllerCurrentLim(uint16_t chargeCurLim, uint16_t dischargeCurLim);

#endif // _FS_BMS_SRC_CAN_H_
