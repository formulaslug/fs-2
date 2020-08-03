#include "BmsFault.h"

void throwBmsFault(uint8_t FaultState) {
  // store which fault(s) have occurred
  // say what type of fault happened
  // disable specific pins

  if (FaultState == FaultVoltageThresholdHigh) {
    serial->printf("***** BMS HIGH VOLTAGE FAULT *****"); // we would have to find the temp first to include the rest of this: \nTemp at %d\n\n", temp);
  }

  if (FaultState == FaultVoltageThresholdLow) {
    serial->printf("***** BMS LOW VOLTAGE FAULT *****");
  }

  if (FaultState == FaultTempThresholdHigh) {
    serial->printf("***** BMS HIGH TEMP FAULT *****");
  }

  if (FaultState == FaultTempThresholdLow) {
    serial->printf("***** BMS LOW TEMP FAULT *****");
  }
}

