#include "BmsConfig.h"
#include "mbed.h"
#include <time.h>


class BmsFaultStates {
  public:
    enum class BmsFaultState : uint8_t {
      FaultVoltageThresholdHigh,
      FaultVoltageThresholdLow,
      FaultTempThresholdHigh,
      FaultTempThresholdLow
    }
}

