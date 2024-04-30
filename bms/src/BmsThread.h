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

  BMSThread(LTC681xBus& bus, unsigned int frequency, BmsEventMailbox* mailbox);

  // Function to allow for starting threads from static context
  static void startThread(BMSThread *p) {
    p->threadWorker();
  }

 private:
  LTC681xBus& m_bus;
  std::vector<LTC6811> m_chips;
  BmsEventMailbox* mailbox;

  BMSThreadState bmsState = BMSThreadState::BMSStartup;

  void throwBmsFault();
  void threadWorker();
};
