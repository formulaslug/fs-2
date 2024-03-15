#include "LTC6811.h"

#include "mbed.h"
#include "rtos.h"

#include "LTC681xParallelBus.h"

LTC6811::LTC6811(LTC681xBus &bus, uint8_t id) : m_bus(bus), m_id(id) {
  m_config =
    Configuration{.gpio5 = GPIOOutputState::kPassive,
                  .gpio4 = GPIOOutputState::kPassive,
                  .gpio3 = GPIOOutputState::kPassive,
                  .gpio2 = GPIOOutputState::kPassive,
                  .gpio1 = GPIOOutputState::kPassive,
                  .referencePowerOff = ReferencePowerOff::kAfterConversions,
                  .dischargeTimerEnabled = DischargeTimerEnable::kDisabled,
                  .adcMode = AdcModeOption::kDefault,
                  .undervoltageComparison = 0,
                  .overvoltageComparison = 0,
                  .dischargeState = {.value = 0},
                  .dischargeTimeout = DischargeTimeoutValue::kDisabled};
}

void LTC6811::updateConfig() {
  // Create configuration data to write
  uint8_t config[6];
  config[0] = (uint8_t) m_config.gpio5 << 7
    | (uint8_t) m_config.gpio4 << 6
    | (uint8_t) m_config.gpio3 << 5
    | (uint8_t) m_config.gpio2 << 4
    | (uint8_t) m_config.gpio1 << 3
    | (uint8_t) m_config.referencePowerOff << 2
    | (uint8_t) m_config.dischargeTimerEnabled << 1
    | (uint8_t) m_config.adcMode;
  config[1] = m_config.undervoltageComparison & 0xFF;
  config[2] = ((m_config.undervoltageComparison >> 8) & 0x0F)
    | (((uint8_t)m_config.overvoltageComparison & 0x0F) << 4);
  config[3] = (m_config.overvoltageComparison >> 4) & 0xFF;
  config[4] = m_config.dischargeState.value & 0xFF;
  config[5] = (((uint8_t)m_config.dischargeTimeout & 0x0F) << 4)
    | ((m_config.dischargeState.value >> 8) & 0x0F);

  auto cmd = LTC681xBus::BuildAddressedBusCommand(WriteConfigurationGroupA(), m_id);

  m_bus.SendDataCommand(cmd, config);
}

LTC6811::Configuration &LTC6811::getConfig() { return m_config; }

uint16_t *LTC6811::getVoltages() {
  auto cmd = StartCellVoltageADC(AdcMode::k7k, false, CellSelection::kAll);
  m_bus.SendCommand(LTC681xBus::BuildAddressedBusCommand(cmd, m_id));

  // Wait 2 ms for ADC to finish
  ThisThread::sleep_for(2ms); // TODO: Change

  // 4  * (Register of 6 Bytes + PEC)
  uint8_t rxbuf[8 * 4];

  m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupA(), m_id), rxbuf);
  m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupB(), m_id), rxbuf + 8);
  m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupC(), m_id), rxbuf + 16);
  m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupD(), m_id), rxbuf + 24);

  // Voltage = val • 100μV
  uint16_t *voltages = new uint16_t[12];
  for (unsigned int i = 0; i < sizeof(rxbuf); i++) {
    // Skip over PEC
    if (i % 8 == 6 || i % 8 == 7) continue;

    // Skip over odd bytes
    if (i % 2 == 1) continue;

    // Wack shit to skip over PEC
    voltages[(i / 2) - (i / 8)] = ((uint16_t)rxbuf[i]) | ((uint16_t)rxbuf[i + 1] << 8);
  }

  return voltages;
}

uint16_t *LTC6811::getGpio() {
  auto cmd = StartGpioADC(AdcMode::k7k, GpioSelection::kAll);
  m_bus.SendCommand(LTC681xBus::BuildAddressedBusCommand(cmd, m_id));

  // Wait 15 ms for ADC to finish
  ThisThread::sleep_for(5ms); // TODO: This could be done differently

  uint8_t rxbuf[8 * 2];

  m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadAuxiliaryGroupA(), m_id), rxbuf);
  m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadAuxiliaryGroupB(), m_id), rxbuf + 8);

  uint16_t *voltages = new uint16_t[5];

  for (unsigned int i = 0; i < sizeof(rxbuf); i++) {
    // Skip over PEC
    if (i % 8 == 6 || i % 8 == 7) continue;

    // Skip over odd bytes
    if (i % 2 == 1) continue;

    // Wack shit to skip over PEC
    voltages[(i / 2) - (i / 8)] = ((uint16_t)rxbuf[i]) | ((uint16_t)rxbuf[i + 1] << 8);
  }

  return voltages;
}

uint16_t *LTC6811::getGpioPin(GpioSelection pin) {
  auto cmd = StartGpioADC(AdcMode::k7k, pin);
  m_bus.SendCommand(LTC681xBus::BuildAddressedBusCommand(cmd, m_id));

  // Wait 5 ms for ADC to finish
  ThisThread::sleep_for(pin == GpioSelection::kAll ? 15ms : 5ms); // TODO: Change to polling

  uint8_t rxbuf[8 * 2];

  m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadAuxiliaryGroupA(), m_id), rxbuf);
  m_bus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadAuxiliaryGroupB(), m_id), rxbuf + 8);

  uint16_t *voltages = new uint16_t[5];

  for (unsigned int i = 0; i < sizeof(rxbuf); i++) {
    // Skip over PEC
    if (i % 8 == 6 || i % 8 == 7) continue;

    // Skip over odd bytes
    if (i % 2 == 1) continue;

    // Wack shit to skip over PEC
    voltages[(i / 2) - (i / 8)] = ((uint16_t)rxbuf[i]) | ((uint16_t)rxbuf[i + 1] << 8);
  }

  return voltages;
}
