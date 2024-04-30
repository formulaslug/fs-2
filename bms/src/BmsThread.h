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

  BMSThread(LTC681xBus& bus, unsigned int frequency, BmsEventMailbox* bmsEventMailbox, BmsBalanceAllowedMailbox* bmsBalanceAllowedMailbox);

  // Function to allow for starting threads from static context
  static void startThread(BMSThread *p) {
    p->threadWorker();
  }

 private:
  bool balanceAllowed = false;
  LTC681xBus& m_bus;
  std::vector<LTC6811> m_chips;
  BmsEventMailbox* bmsEventMailbox;
  BmsEventMailbox* bmsBalanceAllowedMailbox;

  BMSThreadState bmsState = BMSThreadState::BMSStartup;

  void throwBmsFault();
  void threadWorker();
};
