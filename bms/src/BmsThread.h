#pragma once

#include <initializer_list>
#include <vector>
#include "tl/optional.hpp"

#include "mbed.h"
#include "rtos.h"

#include "BmsConfig.h"
#include "EnergusTempSensor.h"
#include "LTC6811.h"
#include "LTC6811Bus.h"

class BMSThread {
 public:
 BMSThread(LTC6811Bus* bus, unsigned int frequency) : m_bus(bus) {
    m_delay = 1000 / frequency;
    for (int i = 0; i < BMS_BANK_COUNT; i++) {
      m_chips.push_back(LTC6811(*bus, i));
    }
    for (int i = 0; i < BMS_BANK_COUNT; i++) {
      //m_chips[i].getConfig().gpio5 = LTC6811::GPIOOutputState::kLow;
      //m_chips[i].getConfig().gpio4 = LTC6811::GPIOOutputState::kPassive;

      m_chips[i].updateConfig();
    }
    m_thread.start(callback(&BMSThread::startThread, this));
  }
  static void startThread(BMSThread *p) {
    p->threadWorker();
  }

 private:
  Thread m_thread;
  unsigned int m_delay;
  LTC6811Bus* m_bus;
  std::vector<LTC6811> m_chips;
  bool m_discharging = false;

  void throwBmsFault() {
    m_discharging = false;
    //palClearLine(LINE_BMS_FLT);
    //palSetLine(LINE_CHARGER_CONTROL);
  }
  void threadWorker() {
    uint16_t* allVoltages = new uint16_t[BMS_BANK_COUNT * BMS_BANK_CELL_COUNT];
    tl::optional<int8_t>* allTemps = new tl::optional<int8_t>[BMS_BANK_COUNT * BMS_BANK_CELL_COUNT];
    uint16_t averageVoltage = -1;
    uint16_t prevMinVoltage = -1;

    while (true) {
      //systime_t timeStart = chVTGetSystemTime();
      // Should be changed to ticker

      uint32_t allBanksVoltage = 0;
      uint16_t minVoltage = 0xFFFF;
      uint16_t maxVoltage = 0x0000;
      int8_t minTemp = INT8_MAX;
      int8_t maxTemp = INT8_MIN;

      for (int i = 0; i < BMS_BANK_COUNT; i++) {
        // Get a reference to the config for toggling gpio
        LTC6811::Configuration& conf = m_chips[i].getConfig();

        // Turn on status LED
        conf.gpio5 = LTC6811::GPIOOutputState::kLow;
        m_chips[i].updateConfig();

        uint16_t* voltages = m_chips[i].getVoltages();

        int temperatures[BMS_BANK_TEMP_COUNT];

        // Measure all temp sensors
        /*
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
          chThdSleepMilliseconds(3);

          uint16_t* temps = m_chips[i].getGpioPin(GpioSelection::k4);
          temperatures[j] = temps[3];

          delete temps;
        }*/

        // Turn off status LED
        //conf.gpio5 = LTC6811::GPIOOutputState::kHigh;
        m_chips[i].updateConfig();

        // Done with communication at this point
        // Now time to crunch numbers

        serial->printf("Slave %d:\n", i);

        // Process voltages
        unsigned int totalVoltage = 0;
        serial->printf("Voltages: ");
        for (int j = 0; j < 12; j++) {
          uint16_t voltage = voltages[j] / 10;


          int index = BMS_CELL_MAP[j];
          if (index != -1) {
            allVoltages[(BMS_BANK_CELL_COUNT * i) + index] = voltage;

            if (voltage < minVoltage && voltage != 0) minVoltage = voltage;
            if (voltage > maxVoltage) maxVoltage = voltage;

            totalVoltage += voltage;
            serial->printf("%dmV ", voltage);

            if (voltage >= BMS_FAULT_VOLTAGE_THRESHOLD_HIGH) {
              // Set fault line
              serial->printf("***** BMS LOW VOLTAGE FAULT *****\nVoltage at %d\n\n", voltage);
              throwBmsFault();
            }
            if (voltage <= BMS_FAULT_VOLTAGE_THRESHOLD_LOW) {
              // Set fault line
              serial->printf("***** BMS HIGH VOLTAGE FAULT *****\nVoltage at %d\n\n", voltage);
              throwBmsFault();
            }

            // Discharge cells if enabled
            if(m_discharging) {
              if((voltage > prevMinVoltage) && (voltage - prevMinVoltage > BMS_DISCHARGE_THRESHOLD)) {
                // Discharge

                serial->printf("DISCHARGE CELL %d: %dmV (%dmV)\n", index, voltage, (voltage - prevMinVoltage));

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
        serial->printf("\n");

        serial->printf("Total Voltage: %dmV\n",
                 totalVoltage);
        serial->printf("Min Voltage: %dmV\n",
                 minVoltage);
        serial->printf("Max Voltage: %dmV\n",
                 maxVoltage);
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

      serial->printf("Temperatures: \n");
      for(int i = 0; i < BMS_BANK_COUNT * BMS_BANK_CELL_COUNT; i++){
        allTemps[i].map_or_else([&](auto temp) {
            if (temp >= BMS_FAULT_TEMP_THRESHOLD_HIGH) {
              serial->printf("***** BMS HIGH TEMP FAULT *****\nTemp at %d\n\n", temp);
              throwBmsFault();
            } else if (temp <= BMS_FAULT_TEMP_THRESHOLD_LOW) {
              serial->printf("***** BMS LOW TEMP FAULT *****\nTemp at %d\n\n", temp);
              throwBmsFault();
            }

            serial->printf("%3d ", temp);
          },
          [&]() {
            serial->printf("ERR ");
            //serial->printf("***** BMS INVALID TEMP FAULT *****\n");
            //throwBmsFault();
          });
        if((i + 1) % BMS_BANK_CELL_COUNT == 0)
          serial->printf("\n");
      }

      /*
      {
	auto txmsg = BMSFaultMessage(gCurrent);
        serial->printf("Current: %d\n", gCurrent);
        canTransmit(&BMS_CAN_DRIVER, CAN_ANY_MAILBOX, &txmsg, TIME_MS2I(100));
      }
      */
      canBus->write(BMSStatMessage(allBanksVoltage / 10, maxVoltage, minVoltage, maxTemp, minTemp));
      
      // Send CAN
      for (size_t i = 0; i < BMS_BANK_COUNT; i++) {
        canBus->write(BMSTempMessage(i, allTemps + (BMS_BANK_TEMP_COUNT * i)));
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
      //serial->printf("BMS Thread time elapsed: %dms\n", timeElapsed);
#endif
      ThisThread::sleep_for(m_delay);
    }
  }
};
