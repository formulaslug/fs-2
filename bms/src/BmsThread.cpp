#include "BmsThread.h"

#include "LTC681xBus.h"
#include "LTC681xCommand.h"
#include "ThisThread.h"

BMSThread::BMSThread(LTC681xBus& bus, unsigned int frequency, std::array<BmsEventMailbox*, 2> mailboxes)
  : m_bus(bus)
  , m_chips {
      LTC6811(bus, 0),
      LTC6811(bus, 1),
      LTC6811(bus, 2),
      LTC6811(bus, 3)
    }
  , mailboxes(mailboxes)
{
  m_delay = 1000 / frequency;
  for (int i = 0; i < BMS_BANK_COUNT; i++) {
    //m_chips[i].getConfig().gpio5 = LTC6811::GPIOOutputState::kLow;
    //m_chips[i].getConfig().gpio4 = LTC6811::GPIOOutputState::kPassive;

    m_chips[i].updateConfig();
  }
}

void BMSThread::threadWorker() {
  // Perform self tests
  //
  // Cell Voltage self test
  m_bus.WakeupBus();
  m_bus.SendCommand(LTC681xBus::BuildBroadcastBusCommand(StartSelfTestCellVoltage(AdcMode::k7k, SelfTestMode::kSelfTest1)));
  ThisThread::sleep_for(4);
  m_bus.WakeupBus();
  for (int i = 0; i < BMS_BANK_COUNT; i++) {
    uint16_t rawVoltages[12];
    if(m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupA(), i), (uint8_t*)rawVoltages) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. SelfTestVoltageA\n");
    }
    if(m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupB(), i), (uint8_t*)rawVoltages + 6) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. SelfTestVoltageB\n");
    }
    if(m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupC(), i), (uint8_t*)rawVoltages + 12) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. SelfTestVoltageC\n");
    }
    if(m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupD(), i), (uint8_t*)rawVoltages + 18) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. SelfTestVoltageD\n");
    }

    for (int j = 0; j < 12; j++) {
      printf("AXST %2d: %4x\n", j, rawVoltages[i]);
    }
  }

  // Cell GPIO self test
  m_bus.WakeupBus();
  m_bus.SendCommand(LTC681xBus::BuildBroadcastBusCommand(StartSelfTestGpio(AdcMode::k7k, SelfTestMode::kSelfTest1)));
  ThisThread::sleep_for(4);
  m_bus.WakeupBus();
  for (int i = 0; i < BMS_BANK_COUNT; i++) {
    uint16_t rawVoltages[12];
    if(m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupA(), i), (uint8_t*)rawVoltages) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. SelfTestVoltageA\n");
    }
    if(m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupB(), i), (uint8_t*)rawVoltages + 6) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. SelfTestVoltageB\n");
    }

    for (int j = 0; j < 12; j++) {
      printf("CVST %2d: %4x\n", j, rawVoltages[i]);
    }
  }

  printf("SELF TEST DONE \n");

  std::array<uint16_t, BMS_BANK_COUNT * BMS_BANK_CELL_COUNT> allVoltages;
  std::array<std::optional<int8_t>, BMS_BANK_COUNT * BMS_BANK_TEMP_COUNT> allTemps;
  while (true) {
    m_bus.WakeupBus();
    
    // Set all status lights high
    // TODO: This should be in some sort of config class
    uint8_t statusOn[6] = { 0x78, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t statusOff[6] = { 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00 };
    auto ledCmd = LTC681xBus::BuildBroadcastBusCommand(WriteConfigurationGroupA());
    m_bus.SendDataCommand(ledCmd, statusOn);

    // Start ADC on all chips
    auto startAdcCmd = StartCellVoltageADC(AdcMode::k7k, false, CellSelection::kAll);
    if(m_bus.SendCommand(LTC681xBus::BuildBroadcastBusCommand(startAdcCmd)) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. StartADC\n");
    }

    // Read back values from all chips
    for (int i = 0; i < BMS_BANK_COUNT; i++) {
      if(m_bus.PollAdcCompletion(LTC681xBus::BuildAddressedBusCommand(PollADCStatus(), 0)) == LTC681xBus::LTC681xBusStatus::PollTimeout) {
        printf("Poll timeout.\n");
      } else {
        printf("Poll OK.\n");
      }

      uint16_t rawVoltages[12];
      if(m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupA(), i), (uint8_t*)rawVoltages) != LTC681xBus::LTC681xBusStatus::Ok) {
        printf("Things are not okay. VoltageA\n");
      }
      if(m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupB(), i), (uint8_t*)rawVoltages + 6) != LTC681xBus::LTC681xBusStatus::Ok) {
        printf("Things are not okay. VoltageB\n");
      }
      if(m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupC(), i), (uint8_t*)rawVoltages + 12) != LTC681xBus::LTC681xBusStatus::Ok) {
        printf("Things are not okay. VoltageC\n");
      }
      if(m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupD(), i), (uint8_t*)rawVoltages + 18) != LTC681xBus::LTC681xBusStatus::Ok) {
        printf("Things are not okay. VoltageD\n");
      }

      for (int j = 0; j < 12; j++) {
        // Endianness of the protocol allows a simple cast :-)
        uint16_t voltage = rawVoltages[j] / 10;

        int index = BMS_CELL_MAP[j];
        if (index != -1) {
          allVoltages[(BMS_BANK_CELL_COUNT * i) + index] = voltage;
        }
      }
    }

    m_bus.SendDataCommand(ledCmd, statusOff);

    for(auto mailbox : mailboxes) {
      if(!mailbox->full()) {
        {
          auto msg = new VoltageMeasurement();
          msg->voltageValues = allVoltages;
          mailbox->put((BmsEvent*) msg);
        }
        /*
        {
          auto msg = new TemperatureMeasurement();
          msg->temperatureValues = allTemps;
          mailbox->put((BmsEvent*) msg);
        }
        */
      }
    }

    ThisThread::sleep_for(100);
  }
}

void BMSThread::throwBmsFault()  {
  m_discharging = false;
  //palClearLine(LINE_BMS_FLT);
  //palSetLine(LINE_CHARGER_CONTROL);
}
