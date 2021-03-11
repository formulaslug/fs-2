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

  BMSThread(LTC681xBus& bus, unsigned int frequency, std::vector<BmsEventMailbox*> mailboxes);

  // Function to allow for starting threads from static context
  static void startThread(BMSThread *p) {
    p->threadWorker();
  }

 private:
  unsigned int m_delay;
  LTC681xBus& m_bus;
  std::vector<LTC6811> m_chips;
  std::vector<BmsEventMailbox*> mailboxes;
  
  // Things that need to go away
  bool m_discharging = false;

  void throwBmsFault();
  void threadWorker();
  /*
  void threadWorker() {
    std::array<uint16_t, BMS_BANK_COUNT * BMS_BANK_CELL_COUNT> allVoltages;
    std::array<std::optional<int8_t>, BMS_BANK_COUNT * BMS_BANK_TEMP_COUNT>
allTemps;

    while (true) {
      for (int i = 0; i < BMS_BANK_COUNT; i++) {
        // Get a reference to the config for toggling gpio
        LTC6811::Configuration& conf = m_chips[i].getConfig();

        // Turn on status LED
        conf.gpio5 = LTC6811::GPIOOutputState::kLow;
        m_chips[i].updateConfig();

        uint16_t* voltages = m_chips[i].getVoltages();

        int temperatures[BMS_BANK_TEMP_COUNT];

        // Measure all temp sensors
        for (unsigned int j = 0; j < BMS_BANK_TEMP_COUNT; j++) {
          conf.gpio1 = (j & 0x01) ? LTC6811::GPIOOutputState::kHigh
                                  : LTC6811::GPIOOutputState::kLow;
          conf.gpio2 = (j & 0x02) ? LTC6811::GPIOOutputState::kHigh
                                  : LTC6811::GPIOOutputState::kLow;
          conf.gpio3 = (j & 0x04) ? LTC6811::GPIOOutputState::kHigh
                                  : LTC6811::GPIOOutputState::kLow;
          conf.gpio4 = LTC6811::GPIOOutputState::kPassive;
          m_chips[i].updateConfig();

          // Wait for config changes to take effect
          ThisThread::sleep_for(3);

          uint16_t* temps = m_chips[i].getGpioPin(GpioSelection::k4);
          temperatures[j] = temps[3];

          delete temps;
        }

        // Turn off status LED
        conf.gpio5 = LTC6811::GPIOOutputState::kHigh;
        m_chips[i].updateConfig();

        // Done with communication at this point
        // Now time to crunch numbers

        // Process voltages
        for (int j = 0; j < 12; j++) {
          uint16_t voltage = voltages[j] / 10;

          int index = BMS_CELL_MAP[j];
          if (index != -1) {
            allVoltages[(BMS_BANK_CELL_COUNT * i) + index] = voltage;

            if (voltage >= BMS_FAULT_VOLTAGE_THRESHOLD_HIGH) {
              // Set fault line
              printf("***** BMS LOW VOLTAGE FAULT *****\nVoltage at
%d\n\n", voltage); throwBmsFault();
            }
            if (voltage <= BMS_FAULT_VOLTAGE_THRESHOLD_LOW) {
              // Set fault line
              printf("***** BMS HIGH VOLTAGE FAULT *****\nVoltage at
%d\n\n", voltage); throwBmsFault();
            }

            // Discharge cells if enabled
            if(m_discharging) {
              if((voltage > prevMinVoltage) && (voltage - prevMinVoltage >
BMS_DISCHARGE_THRESHOLD)) {
                // Discharge

                printf("DISCHARGE CELL %d: %dmV (%dmV)\n", index,
voltage, (voltage - prevMinVoltage));

                // Enable discharging
                conf.dischargeState.value |= (1 << j);
              } else {
                // Disable discharging
                conf.dischargeState.value &= ~(1 << j);
              }
            } else {
              // Disable discharging
              conf.dischargeState.value &= ~(1 << j);
            }
          }
        }

        delete voltages;


        for (unsigned int j = 0; j < BMS_BANK_TEMP_COUNT; j++) {
          auto temp = convertTemp(temperatures[j] / 10);
          allTemps[(BMS_BANK_TEMP_COUNT * i) + j] = temp;

          temp.map([&](auto t){
              if (t < minTemp) minTemp = t;
              if (t > maxTemp) maxTemp = t;
            });
        }
        allBanksVoltage += totalVoltage;
      }

      averageVoltage = allBanksVoltage / (BMS_BANK_COUNT * BMS_BANK_CELL_COUNT);
      prevMinVoltage = minVoltage;

      printf("Temperatures: \n");
      for(int i = 0; i < BMS_BANK_COUNT * BMS_BANK_CELL_COUNT; i++){
        allTemps[i].map_or_else([&](auto temp) {
            if (temp >= BMS_FAULT_TEMP_THRESHOLD_HIGH) {
              printf("***** BMS HIGH TEMP FAULT *****\nTemp at %d\n\n",
temp); throwBmsFault(); } else if (temp <= BMS_FAULT_TEMP_THRESHOLD_LOW) {
              printf("***** BMS LOW TEMP FAULT *****\nTemp at %d\n\n",
temp); throwBmsFault();
            }

            printf("%3d ", temp);
          },
          [&]() {
            printf("ERR ");
            //printf("***** BMS INVALID TEMP FAULT *****\n");
            //throwBmsFault();
          });
        if((i + 1) % BMS_BANK_CELL_COUNT == 0)
          printf("\n");
      }

      canBus->write(BMSStatMessage(allBanksVoltage / 10, maxVoltage, minVoltage,
maxTemp, minTemp));

      // Send CAN
      for (size_t i = 0; i < BMS_BANK_COUNT; i++) {
        // Convert from optional temp values to values with default of -127 (to
indicate error) auto temps = std::array<int8_t, BMS_BANK_TEMP_COUNT>();
        std::transform(allTemps.begin() + (BMS_BANK_TEMP_COUNT * i),
                       allTemps.begin() + (BMS_BANK_TEMP_COUNT * (i + 1)),
                       temps.begin(),
                       [](std::optional<int8_t> t) { return t.value_or(-127); });

        canBus->write(BMSTempMessage(i, (uint8_t*)temps.data()));
      }

      for (size_t i = 0; i < 7; i++) {
        canBus->write(BMSVoltageMessage(i, allVoltages + (4 * i)));
      }

      // Compute time elapsed since beginning of measurements and sleep for
      // m_delay accounting for elapsed time
      // TODO: use a hardware timer or a virtual timer or literally anything
      // else. kek.
      //unsigned int timeElapsed = TIME_I2MS(chVTTimeElapsedSinceX(timeStart));
#ifdef DEBUG
      //printf("BMS Thread time elapsed: %dms\n", timeElapsed);
#endif
      ThisThread::sleep_for(m_delay);
    }
  }
  */
};
