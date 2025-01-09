#include "BmsThread.h"

#include "LTC681xBus.h"
#include "LTC681xCommand.h"
#include "ThisThread.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>

#include "EnergusTempSensor.h"

BMSThread::BMSThread(LTC681xBus &bus, unsigned int frequency, BmsEventMailbox* bmsEventMailbox, MainToBMSMailbox* mainToBMSMailbox)
    : m_bus(bus), bmsEventMailbox(bmsEventMailbox), mainToBMSMailbox(mainToBMSMailbox) {
  for (int i = 0; i < BMS_BANK_COUNT; i++) {
    m_chips.push_back(LTC6811(bus, i));
  }
  for (int i = 0; i < BMS_BANK_COUNT; i++) {
    // m_chips[i].getConfig().gpio5 = LTC6811::GPIOOutputState::kLow;
    // m_chips[i].getConfig().gpio4 = LTC6811::GPIOOutputState::kPassive;

    m_chips[i].updateConfig();
  }
}

void BMSThread::threadWorker() {
  printf("BMS threadWorker()\n");
  // Perform self tests

  // Cell Voltage self test
  m_bus.WakeupBus();
  m_bus.SendCommand(LTC681xBus::BuildBroadcastBusCommand(
      StartSelfTestCellVoltage(AdcMode::k7k, SelfTestMode::kSelfTest1)));
  ThisThread::sleep_for(4ms);
  m_bus.WakeupBus();
  printf("BMS A\n");
  for (int i = 0; i < BMS_BANK_COUNT; i++) {
    uint16_t rawVoltages[12];

    if (m_bus.SendReadCommand(
            LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupA(), i),
            (uint8_t *)rawVoltages) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. SelfTestVoltageA\n");
    }
    if (m_bus.SendReadCommand(
            LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupB(), i),
            (uint8_t *)rawVoltages + 6) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. SelfTestVoltageB\n");
    }
    if (m_bus.SendReadCommand(
            LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupC(), i),
            (uint8_t *)rawVoltages + 12) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. SelfTestVoltageC\n");
    }
    if (m_bus.SendReadCommand(
            LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupD(), i),
            (uint8_t *)rawVoltages + 18) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. SelfTestVoltageD\n");
    }

    for (int j = 0; j < 12; j++) {
      printf("AXST %2d: %4x\n", j, rawVoltages[i]);
    }
  }
  printf("BMS B\n");

  // Cell GPIO self test
  m_bus.WakeupBus();
  m_bus.SendCommand(LTC681xBus::BuildBroadcastBusCommand(
      StartSelfTestGpio(AdcMode::k7k, SelfTestMode::kSelfTest1)));
  ThisThread::sleep_for(4ms);
  m_bus.WakeupBus();
  for (int i = 0; i < BMS_BANK_COUNT; i++) {
    uint16_t rawVoltages[12];
    if (m_bus.SendReadCommand(
            LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupA(), i),
            (uint8_t *)rawVoltages) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. SelfTestVoltageA\n");
    }
    if (m_bus.SendReadCommand(
            LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupB(), i),
            (uint8_t *)rawVoltages + 6) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. SelfTestVoltageB\n");
    }

    for (int j = 0; j < 12; j++) {
      printf("CVST %2d: %4x\n", j, rawVoltages[i]);
    }
  }

  printf("SELF TEST DONE \n");
  bmsState = BMSThreadState::BMSIdle;

  std::array<uint16_t, BMS_BANK_COUNT * BMS_BANK_CELL_COUNT> allVoltages;
  std::array<int8_t, BMS_BANK_COUNT * BMS_BANK_TEMP_COUNT> allTemps;
  while (true) {

      bool isBalancing = false;
      
    while(!mainToBMSMailbox->empty()) {
        MainToBMSEvent *mainToBMSEvent;
        
        osEvent evt = mainToBMSMailbox->get();
        if (evt.status == osEventMessage) {
            mainToBMSEvent = (MainToBMSEvent*)evt.value.p;
        } else {
            continue;
        }

        balanceAllowed = mainToBMSEvent->balanceAllowed;
        charging = mainToBMSEvent->charging;
        // printf("Balance Allowed: %x\nCharging: %x\n", balanceAllowed, charging);
        delete mainToBMSEvent;
    }



    m_bus.WakeupBus();

    // Set all status lights high
    // TODO: This should be in some sort of config class

    for (int i = 0; i < BMS_BANK_COUNT; i++) {
      LTC6811::Configuration &config = m_chips[i].getConfig();
      config.gpio5 = LTC6811::GPIOOutputState::kLow;
      m_chips[i].updateConfig();
    }

    // turn off cell balancing for voltage reading
    for (int i = 0; i < BMS_BANK_COUNT; i++) {
        LTC6811::Configuration &config = m_chips[i].getConfig();
        
        config.dischargeState.value = 0x0000;

        m_chips[i].updateConfig();
    }
    ThisThread::sleep_for(5ms);

    // Start ADC on all chips
    auto startAdcCmd =
        StartCellVoltageADC(AdcMode::k7k, false, CellSelection::kAll);
    if (m_bus.SendCommand(LTC681xBus::BuildBroadcastBusCommand(startAdcCmd)) !=
        LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. StartADC\n");
    }

    ThisThread::sleep_for(10ms);

    // Read back values from all chips
    for (int i = 0; i < BMS_BANK_COUNT; i++) {
      if (m_bus.PollAdcCompletion(
              LTC681xBus::BuildAddressedBusCommand(PollADCStatus(), 0)) ==
          LTC681xBus::LTC681xBusStatus::PollTimeout) {
        printf("Poll timeout.\n");
      } else {
        //printf("Poll OK.\n");
      }

      uint16_t rawVoltages[12];

      if (m_bus.SendReadCommand(
              LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupA(), i),
              (uint8_t *)rawVoltages) != LTC681xBus::LTC681xBusStatus::Ok) {
        printf("Things are not okay. VoltageA\n");
      }
      if (m_bus.SendReadCommand(
              LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupB(), i),
              (uint8_t *)rawVoltages + 6) != LTC681xBus::LTC681xBusStatus::Ok) {
        printf("Things are not okay. VoltageB\n");
      }
      if (m_bus.SendReadCommand(
              LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupC(), i),
              (uint8_t *)rawVoltages + 12) !=
          LTC681xBus::LTC681xBusStatus::Ok) {
        printf("Things are not okay. VoltageC\n");
      }
      if (m_bus.SendReadCommand(
              LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupD(), i),
              (uint8_t *)rawVoltages + 18) !=
          LTC681xBus::LTC681xBusStatus::Ok) {
        printf("Things are not okay. VoltageD\n");
      }

      // MUX get temp from each cell
      for (uint8_t j = 0; j < BMS_BANK_CELL_COUNT; j++) {

        LTC6811::Configuration &config = m_chips[i].getConfig();
        config.gpio1 = (j & 0b001) ? LTC6811::GPIOOutputState::kHigh
                                   : LTC6811::GPIOOutputState::kLow;
        config.gpio2 = ((j & 0b010) >> 1) ? LTC6811::GPIOOutputState::kHigh
                                          : LTC6811::GPIOOutputState::kLow;
        config.gpio3 = ((j & 0b100) >> 2) ? LTC6811::GPIOOutputState::kHigh
                                          : LTC6811::GPIOOutputState::kLow;
        config.gpio4 = LTC6811::GPIOOutputState::kPassive;

        m_chips[i].updateConfig();

        auto gpioADCcmd = StartGpioADC(AdcMode::k7k, GpioSelection::k4);
        if (m_bus.SendCommand(LTC681xBus::BuildAddressedBusCommand(
                gpioADCcmd, i)) != LTC681xBus::LTC681xBusStatus::Ok) {
          printf("Things are not okay. StartGPIO ADC\n");
        }

        ThisThread::sleep_for(5ms);

        uint8_t rxbuf[8 * 2];

        m_bus.SendReadCommand(
            LTC681xBus::BuildAddressedBusCommand(ReadAuxiliaryGroupA(), i),
            rxbuf);
        m_bus.SendReadCommand(
            LTC681xBus::BuildAddressedBusCommand(ReadAuxiliaryGroupB(), i),
            rxbuf + 8);

        uint16_t tempVoltage = ((uint16_t)rxbuf[8]) | ((uint16_t)rxbuf[9] << 8);

        int8_t temp = convertTemp(tempVoltage / 10);
        // printf("%d: T: %d\n", j, temp);
        allTemps[(BMS_BANK_CELL_COUNT * i) + j] = temp;
      }

      for (int j = 0; j < 12; j++) {
        // Endianness of the protocol allows a simple cast :-)
        uint16_t voltage = rawVoltages[j] / 10;

        int index = BMS_CELL_MAP[j];
        if (index != -1) {
          allVoltages[(BMS_BANK_CELL_COUNT * i) + index] = voltage;

        //   printf("%d: V: %d\n", index, voltage);
        }
      }
    }
    // printf("Fuck: ");
    uint16_t minVoltage = allVoltages[0];
    uint16_t maxVoltage = 0;
    for (int i = 0; i < BMS_BANK_COUNT * BMS_BANK_CELL_COUNT; i++) {
        // if (allVoltages[i] > BMS_FAULT_VOLTAGE_THRESHOLD_HIGH) {
        //     printf("%d, ", i);
        // }
      if (allVoltages[i] < minVoltage) {
        minVoltage = allVoltages[i];
      } else if (allVoltages[i] > maxVoltage) {
        maxVoltage = allVoltages[i];
      }
    }
    // printf ("\n");
    
    int8_t minTemp = allTemps[0];
    int8_t maxTemp = 0;
    int8_t avgTemp = 0;
    int16_t tempSum = 0;
    for (int i = 0; i < BMS_BANK_COUNT * BMS_BANK_TEMP_COUNT; i++) {
      tempSum += allTemps[i];
      if (allTemps[i] < minTemp) {
        minTemp = allTemps[i];
      } else if (allTemps[i] > maxTemp) {
        maxTemp = allTemps[i];
      }
    }
    avgTemp = tempSum / (BMS_BANK_COUNT * BMS_BANK_TEMP_COUNT);
    // printf("0 Temps: %d, %d, %d, %d, %d, %d, %d\n", allTemps[0], allTemps[1], allTemps[2], allTemps[3], allTemps[4], allTemps[5], allTemps[6]);
    // printf("1 Temps: %d, %d, %d, %d, %d, %d, %d\n", allTemps[7], allTemps[8], allTemps[9], allTemps[10], allTemps[11], allTemps[12], allTemps[13]);
    // printf("2 Temps: %d, %d, %d, %d, %d, %d, %d\n", allTemps[14], allTemps[15], allTemps[16], allTemps[17], allTemps[18], allTemps[19], allTemps[20]);
    // printf("3 Temps: %d, %d, %d, %d, %d, %d, %d\n\n", allTemps[21], allTemps[22], allTemps[23], allTemps[24], allTemps[25], allTemps[26], allTemps[27]);
    // printf("min temp: %d, max temp: %d\nmin volt: %d, max volt %d\n", minTemp, maxTemp, minVoltage, maxVoltage);

    if (minVoltage <= BMS_FAULT_VOLTAGE_THRESHOLD_LOW ||
        maxVoltage >= BMS_FAULT_VOLTAGE_THRESHOLD_HIGH ||
        minTemp <= BMS_FAULT_TEMP_THRESHOLD_LOW ||
        maxTemp >= ((charging) ? BMS_FAULT_TEMP_THRESHOLD_CHARING_HIGH : BMS_FAULT_TEMP_THRESHOLD_HIGH)) {
        
        if (minVoltage <= BMS_FAULT_VOLTAGE_THRESHOLD_LOW) {
            printf("Voltage too low: %d\n", minVoltage);
        }
        if (maxVoltage >= BMS_FAULT_VOLTAGE_THRESHOLD_HIGH) {
            printf("Voltage too high: %d\n", maxVoltage);
            printf("Voltages: ");
            for (int l = 0; l < BMS_BANK_COUNT * BMS_BANK_CELL_COUNT; l++) {
                printf("%d, ", allVoltages[l]);
            }
            printf("\n");
        }
        if (minTemp <= BMS_FAULT_TEMP_THRESHOLD_LOW) {
            printf("Temp too low: %d\n", minTemp);
        }
        if (maxTemp >= ((charging) ? BMS_FAULT_TEMP_THRESHOLD_CHARING_HIGH : BMS_FAULT_TEMP_THRESHOLD_HIGH)) {
            printf("Temp too high: %d\n", maxTemp);
        }



        if (bmsState == BMSThreadState::BMSFaultRecover || bmsState == BMSThreadState::BMSFault) {
            printf("FAULT STATE\n");
            throwBmsFault();
        } else {
            printf("ENTERING FAULT RECOVERY\n");
            bmsState = BMSThreadState::BMSFaultRecover;
            ThisThread::sleep_for(10ms);
            continue;
        }
    }

    if (bmsState == BMSThreadState::BMSIdle && balanceAllowed) {
      for (int i = 0; i < BMS_BANK_COUNT; i++) {

        LTC6811::Configuration &config = m_chips[i].getConfig();

        uint16_t dischargeValue = 0x0000;

        for (int j = 0; j < 12; j++) {
          if (BMS_CELL_MAP[j] == -1) {
            continue;
          }
          int cellNum = BMS_CELL_MAP[j];
          uint16_t cellVoltage = allVoltages[i * BMS_BANK_CELL_COUNT + cellNum];
          if (cellVoltage >= BMS_BALANCE_THRESHOLD &&
              cellVoltage >= minVoltage + BMS_DISCHARGE_THRESHOLD) {
            // printf("Balancing cell %d\?n", cellNum);
            dischargeValue |= (0x1 << j);
            isBalancing = true;
          }
        }

        // printf("discharge value: %x\n", dischargeValue);

        config.dischargeState.value = dischargeValue;

        m_chips[i].updateConfig();
      }
    } else {
      for (int i = 0; i < BMS_BANK_COUNT; i++) {

        LTC6811::Configuration &config = m_chips[i].getConfig();
        config.dischargeState.value = 0x0000;
        m_chips[i].updateConfig();
      }
    }

    for (int i = 0; i < BMS_BANK_COUNT; i++) {
      LTC6811::Configuration &config = m_chips[i].getConfig();
      config.gpio5 = LTC6811::GPIOOutputState::kLow;
      m_chips[i].updateConfig();
    }

    if (!bmsEventMailbox->full()) {
        BmsEvent* msg = new BmsEvent();
        for (int i = 0; i < BMS_BANK_COUNT*BMS_BANK_CELL_COUNT; i++) {
            msg->voltageValues[i] = allVoltages[i];
        }
        for (int i = 0; i < BMS_BANK_COUNT*BMS_BANK_TEMP_COUNT; i++) {
            msg->temperatureValues[i] = allTemps[i];
        }
        msg->bmsState = bmsState;
        msg->isBalancing = isBalancing;
        msg->minVolt = (uint8_t)(minVoltage*50/1000.0);
        msg->maxVolt = (uint8_t)(maxVoltage*50/1000.0);
        msg->minTemp = minTemp;
        msg->maxTemp = maxTemp;
        msg->avgTemp = avgTemp;
        bmsEventMailbox->put((BmsEvent *)msg);
    }
    

    if (charging) {
        ThisThread::sleep_for(500ms); // longer duty cycle when charging, 500 default
    } else {
        ThisThread::sleep_for(100ms); // 100 ms
    }
    if (bmsState == BMSThreadState::BMSFaultRecover) {
        bmsState = BMSThreadState::BMSIdle;
    }
  }
}

void BMSThread::throwBmsFault() {
    bmsState = BMSThreadState::BMSFault;
}
