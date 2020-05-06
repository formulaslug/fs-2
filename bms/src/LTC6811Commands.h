#pragma once

class LTC6811Command {
 public:
  virtual uint16_t toValue() const = 0;
};

enum class AdcMode : uint8_t {
  k422 = 0,
  k27k = 1,
  k7k = 2,
  k26 = 3,
  k1k = 0,
  k14k = 1,
  k3k = 2,
  k2k = 3
};
enum class CellSelection : uint8_t {
  kAll = 0,
  k1_7 = 1,
  k2_8 = 2,
  k3_9 = 3,
  k4_10 = 4,
  k5_11 = 5,
  k6_12 = 6
};
enum class PullDirection : uint8_t { kPullDown = 0, kPullUp = 1 };
enum class SelfTestMode : uint8_t { kSelfTest1 = 1, kSelfTest2 = 2 };
enum class GpioSelection : uint8_t {
  kAll = 0,
  k1 = 1,
  k2 = 2,
  k3 = 3,
  k4 = 4,
  k5 = 5,
  kRef = 6
};

class WriteConfigurationGroupA : public LTC6811Command {
  uint16_t toValue() const { return 0x0001; }
};
class WriteConfigurationGroupB : public LTC6811Command {
  uint16_t toValue() const { return 0x0024; }
};
class ReadConfigurationGroupA : public LTC6811Command {
  uint16_t toValue() const { return 0x0002; }
};
class ReadConfigurationGroupB : public LTC6811Command {
  uint16_t toValue() const { return 0x0025; }
};

class ReadCellVoltageGroupA : public LTC6811Command {
  uint16_t toValue() const { return 0x0004; }
};
class ReadCellVoltageGroupB : public LTC6811Command {
  uint16_t toValue() const { return 0x0006; }
};
class ReadCellVoltageGroupC : public LTC6811Command {
  uint16_t toValue() const { return 0x0008; }
};
class ReadCellVoltageGroupD : public LTC6811Command {
  uint16_t toValue() const { return 0x000A; }
};
class ReadCellVoltageGroupE : public LTC6811Command {
  uint16_t toValue() const { return 0x0009; }
};
class ReadCellVoltageGroupF : public LTC6811Command {
  uint16_t toValue() const { return 0x000B; }
};

class ReadAuxiliaryGroupA : public LTC6811Command {
  uint16_t toValue() const { return 0x000C; }
};
class ReadAuxiliaryGroupB : public LTC6811Command {
  uint16_t toValue() const { return 0x000E; }
};
class ReadAuxiliaryGroupC : public LTC6811Command {
  uint16_t toValue() const { return 0x000D; }
};
class ReadAuxiliaryGroupD : public LTC6811Command {
  uint16_t toValue() const { return 0x000F; }
};

class ReadStatusGroupA : public LTC6811Command {
  uint16_t toValue() const { return 0x0010; }
};
class ReadStatusGroupB : public LTC6811Command {
  uint16_t toValue() const { return 0x0012; }
};

class WriteSControlGroup : public LTC6811Command {
  uint16_t toValue() const { return 0x0016; }
};
class WritePWMGroup : public LTC6811Command {
  uint16_t toValue() const { return 0x0020; }
};
class ReadSControlGroup : public LTC6811Command {
  uint16_t toValue() const { return 0x0016; }
};
class ReadPWMGroup : public LTC6811Command {
  uint16_t toValue() const { return 0x0022; }
};

class StartSControlPulsing : public LTC6811Command {
  uint16_t toValue() const { return 0x0019; }
};
class ClearSControlGroup : public LTC6811Command {
  uint16_t toValue() const { return 0x0018; }
};

class StartCellVoltageADC : public LTC6811Command {
 public:
  StartCellVoltageADC(AdcMode a, bool d, CellSelection c)
      : adcMode(a), dischargePermitted(d), cellSelection(c) {}
  AdcMode adcMode;
  bool dischargePermitted;
  CellSelection cellSelection;
  uint16_t toValue() const {
    return 0x0260 | (((uint16_t)adcMode) << 7) |
           (dischargePermitted ? 0x0010 : 0x0000) | ((uint16_t)cellSelection);
  }
};
class StartOpenWireADC : public LTC6811Command {
 public:
  AdcMode adcMode;
  PullDirection pull;
  bool dischargePermitted;
  CellSelection cellSelection;
  uint16_t toValue() const {
    return 0x0228 | ((uint16_t)adcMode << 7) | ((uint16_t)pull << 6) |
           (dischargePermitted ? 0x0010 : 0x0000) | ((uint16_t)cellSelection);
  }
};
class StartSelfTestConversion : public LTC6811Command {
 public:
  AdcMode adcMode;
  SelfTestMode testMode;
  uint16_t toValue() const {
    return 0x0207 | ((uint16_t)adcMode << 7) | ((uint16_t)testMode << 5);
  }
};
// Lost my mind at "Start Overlap Measurement of Cell 7 Voltage"
class StartGpioADC : public LTC6811Command {
 public:
  StartGpioADC(AdcMode a, GpioSelection g) : adcMode(a), gpioSelection(g) {}
  AdcMode adcMode;
  GpioSelection gpioSelection;
  uint16_t toValue() const {
    return 0x0460 | ((uint16_t)adcMode << 7) | ((uint16_t)gpioSelection);
  }
};
