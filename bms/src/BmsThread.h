#pragma once

#include <array>
#include <initializer_list>
#include <vector>
#include <algorithm>

#include <optional>

#include "mbed.h"
#include "rtos.h"
#include "Mail.h"

#include "BmsConfig.h"
//#include "Can.h"

#include "EnergusTempSensor.h"
#include "LTC6811.h"
#include "LTC681xBus.h"
#include "Event.h"

class BMSThread {
 public:

  BMSThread(LTC681xBus& bus, unsigned int frequency, std::array<BmsEventMailbox*, 2> mailboxes);

  // Function to allow for starting threads from static context
  static void startThread(BMSThread *p) {
    p->threadWorker();
  }

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

    // BMS in comm failure mode
    //   Attempt to reconnect with banks for N seconds and if unsuccessful throw a fault
    //   send warning on CAN
    //
    //   If successful, return to previous state
    BMSBusCommunicationFailure,

    //
    // Charging states
    //
    
    // BMS waiting/sensing to avoid reading cells while charging (not accurate)
    //   BMS_PIN_CHARGER_CONTROL set low
    //   Wait ~5s then get some voltage data
    //     If we are above our charging goal, go to BMSChargingDone
    //     If we need to balance, go to BMSChargingBalance
    //     else go to BMSChargingActive
    BMSChargingSense,

    // BMS actively charging
    //   BMS_PIN_CHARGER_CONTROL set high
    //   Voltage/temp monitored
    //   Every ~5min go to BMSChargingSensing
    BMSChargingActive,

    // BMS balancing cells
    //   BMS_PIN_CHARGER_CONTROL set low
    //   discharging enabled for cells BMS_DISCHARGE_THRESHOLD above lowest cell
    //   Wait ~5min then go to BMSChargingSensing
    BMSChargingBalance,

    // BMS charging complete
    //   BMS_PIN_CHARGER_CONTROL set low
    //   discharging disabled
    //
    //   (maybe) every 5 mins go in to BMSChargingSense
    BMSChargingDone,

    // BMS charging fault
    //   BMS_PIN_CHARGER_CONTROL set low
    //
    //   Entered if any readings go outside allowable range (voltage, temp, current)
    //   BMS_PIN_BMS_FLT set high
    //
    //   Able to enter BMSIdle if fault latch cleared (after some time for debounce)
    BMSChargingFault
  };

 private:
  unsigned int m_delay;
  LTC681xBus& m_bus;
  std::array<LTC6811, BMS_BANK_COUNT> m_chips;
  std::array<BmsEventMailbox*, 2> mailboxes;
  
  // Things that need to go away
  bool m_discharging = false;

  void throwBmsFault();
  void threadWorker();
};
