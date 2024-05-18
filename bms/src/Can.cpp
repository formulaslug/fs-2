#include "Can.h"
#include <cstdint>

CANMessage accBoardBootup () {
  const char * startupMessage = 0x00;
  return CANMessage(kNMT_ACC_HEARTBEAT, startupMessage);  
}

CANMessage accBoardState(uint8_t glvVoltage, uint16_t tsVoltage, bool bmsFault, bool bmsBalancing, bool prechargeDone, bool charging, bool fansOn, bool shutdownClosed, bool unused_A, bool unused_B, uint8_t minCellVoltage, uint8_t maxCellVoltage, int16_t tsCurrent) {
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

CANMessage accBoardTemp(uint8_t segment, int8_t *temps) {
    uint8_t data[8];
    data[0] = segment;
    for (int i = 0; i < 7; i++) {
        data[i+1] = (uint8_t)(temps[i]+40);
    }
    
    return CANMessage(kTPDO_ACC_BOARD_Temp, data);
}

CANMessage accBoardVolt(uint8_t segment, uint16_t *volts) {
    uint8_t data[8];
    data[0] = segment;
    for (int i = 0; i < 7; i++) {
        data[i+1] = (uint8_t) (50.0*volts[i]/1000.0);
    }
    
    return CANMessage(kTPDO_ACC_BOARD_Volt, data);
}

CANMessage motorControllerCurrentLim(uint16_t chargeCurLim, uint16_t dischargeCurLim) {
    uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    data[0] = chargeCurLim;
    data[1] = chargeCurLim >> 8;
    data[2] = dischargeCurLim;
    data[3] = dischargeCurLim >> 8;
    return CANMessage(kRPDO_MAX_CURRENTS, data);
}