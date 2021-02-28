#include "BmsThread.h"

BMSThread::BMSThread(LTC6811Bus* bus, unsigned int frequency, std::vector<Queue<BmsEvent, mailboxSize>*> mailboxes) : m_bus(bus), mailboxes(mailboxes) {
  m_delay = 1000 / frequency;
  for (int i = 0; i < BMS_BANK_COUNT; i++) {
    m_chips.push_back(LTC6811(*bus, i));
  }
  for (int i = 0; i < BMS_BANK_COUNT; i++) {
    //m_chips[i].getConfig().gpio5 = LTC6811::GPIOOutputState::kLow;
    //m_chips[i].getConfig().gpio4 = LTC6811::GPIOOutputState::kPassive;

    m_chips[i].updateConfig();
  }
}

void BMSThread::threadWorker() {
  std::array<uint16_t, BMS_BANK_COUNT * BMS_BANK_CELL_COUNT> allVoltages;
  std::array<std::optional<int8_t>, BMS_BANK_COUNT * BMS_BANK_TEMP_COUNT> allTemps;

  while (true) {
    // Measure from all BMS banks
    for (int i = 0; i < BMS_BANK_COUNT; i++) {
      LTC6811::Configuration& conf = m_chips[i].getConfig();

      // Turn on status LED
      conf.gpio5 = LTC6811::GPIOOutputState::kLow;
      m_chips[i].updateConfig();

      uint16_t* voltages = m_chips[i].getVoltages();

      // Measure all temperature sensors on the current bank
      uint16_t temperatures[BMS_BANK_TEMP_COUNT];
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
        temperatures[j] = temps[j] / 10;

        delete temps;
      }

      // Turn off status LED
      conf.gpio5 = LTC6811::GPIOOutputState::kHigh;
      m_chips[i].updateConfig();

      //
      // Process values
      //
      for (int j = 0; j < 12; j++) {
        uint16_t voltage = voltages[j] / 10;

        int index = BMS_CELL_MAP[j];
        if (index != -1) {
          allVoltages[(BMS_BANK_CELL_COUNT * i) + index] = voltage;
        }
      }

      for (unsigned int j = 0; j < BMS_BANK_TEMP_COUNT; j++) {
        auto temp = convertTemp(temperatures[j]);
        allTemps[(BMS_BANK_TEMP_COUNT * i) + j] = temp;
      }

      delete voltages;
    }

    for(auto mailbox : mailboxes) {
      if(!mailbox->full()) {
        {
          auto msg = new VoltageMeasurement();
          msg->voltageValues = allVoltages;
          mailbox->put((BmsEvent*) msg);
        }
        {
          auto msg = new TemperatureMeasurement();
          msg->temperatureValues = allTemps;
          mailbox->put((BmsEvent*) msg);
        }
      }
    }
  }
}

void BMSThread::throwBmsFault()  {
  m_discharging = false;
  //palClearLine(LINE_BMS_FLT);
  //palSetLine(LINE_CHARGER_CONTROL);
}
