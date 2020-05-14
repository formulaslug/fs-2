/* Formula Slug - Sam Ritzo */

#pragma once

class LTC68xxCommandCode {
public:
  virtual uint16_t toValue() const = 0;

  enum class MD : uint8_t {
    // ADCOPT = 0
    kMD_422Hz = 0b00
    kMD_27kHz = 0b01, // Fast
    kMD_7kHz  = 0b10, // Normal
    kMD_26Hz  = 0b11, // Filtered
    // ADCOPT = 1
    kMD_1kHz  = 0b00,
    kMD_14kHz = 0b01,
    kMD_3kHz  = 0b10,
    kMD_2kHz  = 0b11,
  };

  enum class DCP : uint8_t {
    kDCP_Forbid = 0b0,
    kDCP_Permit = 0b1,
  };

  enum class CH : uint8_t {
    kCH_All  = 0b000,
    kCH_1_7  = 0b001,
    kCH_2_8  = 0b010,
    kCH_3_9  = 0b011,
    kCH_4_10 = 0b100,
    kCH_5_11 = 0b101,
    kCH_6_12 = 0x110,
  };

  enum class PUP : uint8_t {
    kPUP_PullDown = 0b0,
    kPUP_PullUp   = 0b1,
  };

  enum class ST : uint8_t {
    kST_1 = 0b01,
    kST_2 = 0b10,
  };

  enum class CHG : uint8_t {
    kCHG_All    = 0b000,
    kCHG_GPIO1  = 0b001,
    kCHG_GPIO2  = 0b010,
    kCHG_GPIO3  = 0b011,
    kCHG_GPIO4  = 0b100,
    kCHG_GPIO5  = 0b101,
    kCHG_2ndRef = 0x110,
  };

  enum class CHST : uint8_t {
    kCHST_All  = 0b000,
    kCHST_SC   = 0b001,
    kCHST_ITMP = 0b010,
    kCHST_VA   = 0b011,
    kCHST_VD   = 0b100,
  };
}


// Write Configuration Register Group A
class WRCFGA : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000000001; }
}

// Write Configuration Register Group B
class WRCFGB : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000100100; }
}

// Read Configuration Register Group A
class RDCFGA : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000000010; }
}

// Read Configuration Register Group B
class RDCFGB : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000100110; }
}

// Read Cell Voltage Register Group A
class RDCVA : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000000100; }
}

// Read Cell Voltage Register Group B
class RDCVB : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000000110; }
}

// Read Cell Voltage Register Group C
class RDCVC : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000001000; }
}

// Read Cell Voltage Register Group D
class RDCVD : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000001010; }
}

// Read Cell Voltage Register Group E
class RDCVE : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000001001; }
}

// Read Cell Voltage Register Group F
class RDCVF : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000001011; }
}

// Read Auxiliary Register Group A
class RDAUXA : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000001100; }
}

// Read Auxiliary Register Group B
class RDAUXB : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000001110; }
}

// Read Auxiliary Register Group C
class RDAUXC : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000001101; }
}

// Read Auxiliary Register Group D
class RDAUXD : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000001111; }
}

// Read Status Register Group A
class RDSTATA : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000010000; }
}

// Read Status Register Group B
class RDSTATB : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000010010; }
}

// Write S Control Register Group
class WRSCTRL : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000010100; }
}

// Write PWM Register Group
class WRPWM : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000100000; }
}

// Write PWM/S Control Register Group B
class WRPSB : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000011100; }
}

// Read S Control Register Group
class RDSCTRL : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000010110; }
}

// Read PWM Register Group
class RDPWM : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000100010; }
}

// Read PWM/S Control Register Group B
class RDPSB : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000011110; }
}

// Start S Control Pulsing and Poll Status
class STSCTRL : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000011001; }
}

// Clear S Control Register Group
class CLRSCTRL : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b00000011000; }
}

// Start Cell Voltage ADC Conversion and Poll Status
class ADCV : public LTC68xxCommandCode {
  public:
    ADCV(MD md, DCP dcp, CH ch) {}
    // 0 1 MD[1:0] 1 1 DCP 0 CH[2:0]
    uint16_t toValue() const {
      return 0b01001100000 |
             ((uint8_t) md   & 0b011) << 7 |
             ((uint8_t) dcp  & 0b001) << 4 |
             ((uint8_t) ch   & 0b111) << 0
    }
}

// Start Open Wire ADC Conversion and Poll Status
class ADOW : public LTC68xxCommandCode {
  public:
    ADOW(MD md, PUP pup, DCP dcp, CH ch) {}
    // 0 1 MD[1:0] PUP 1 DCP 1 CH[2:0]
    uint16_t toValue() const {
      return 0b01000101000 |
             ((uint8_t) md   & 0b011) << 7 |
             ((uint8_t) pup  & 0b001) << 6 |
             ((uint8_t) dcp  & 0b001) << 4 |
             ((uint8_t) ch   & 0b111) << 0
    }
}

// Start Self Test Cell Voltage Conversion and Poll Status
class CVST : public LTC68xxCommandCode {
  public:
    CVST(MD md, ST st) {}
    // 0 1 MD[1:0] ST[1:0] 0 0 1 1 1
    uint16_t toValue() const {
      return 0b01000000111 |
             ((uint8_t) md   & 0b011) << 7 |
             ((uint8_t) st   & 0b011) << 5
    }
}

// Start Overlap Measurement of Cell 7 Voltage
class ADOL : public LTC68xxCommandCode {
  public:
    ADOL(MD md, DCP dcp) {}
    // 0 1 MD[1:0] 0 0 DCP 0 0 0 1
    uint16_t toValue() const {
      return 0b01000000001 |
             ((uint8_t) md   & 0b011) << 7 |
             ((uint8_t) dcp  & 0b001) << 4
    }
}

// Start GPIOs ADC Conversion and Poll Status
class ADAX : public LTC68xxCommandCode {
  public:
    ADAX(MD md, CHG chg) {}
    // 1 0 MD[1:0] 1 1 0 0 CHG[2:0]
    uint16_t toValue() const {
      return 0b10001100000 |
             ((uint8_t) md   & 0b011) << 7 |
             ((uint8_t) chg  & 0b111) << 0
    }
}

// Start GPIOs ADC Conversion With Digital Redundancy and Poll Status
class ADAXD : public LTC68xxCommandCode {
  public:
    ADAXD(MD md, CHG chg) {}
    // 1 0 MD[1:0] 0 0 0 0 CHG[2:0]
    uint16_t toValue() const {
      return 0b10000000000 |
             ((uint8_t) md   & 0b011) << 7 |
             ((uint8_t) chg  & 0b111) << 0
    }
}

// Start Self Test GPIOs Conversion and Poll Status
class AXST : public LTC68xxCommandCode {
  public:
    AXST(MD md, ST st) {}
    // 1 0 MD[1:0] ST[1:0] 0 0 1 1 1
    uint16_t toValue() const {
      return 0b10000000111 |
             ((uint8_t) md   & 0b011) << 7 |
             ((uint8_t) st   & 0b011) << 5
    }
}

// Start Status Group ADC Conversion and Poll Status
class ADSTAT : public LTC68xxCommandCode {
  public:
    ADSTAT(MD md, CHST chst) {}
    // 1 0 MD[1:0] 1 1 0 1 CHST[2:0]
    uint16_t toValue() const {
      return 0b10001101000 |
             ((uint8_t) md   & 0b011) << 7 |
             ((uint8_t) chst & 0b111) << 0
    }
}

// Start Status Group ADC Conversion With Digital Redundancy and Poll Status
class ADSTATD : public LTC68xxCommandCode {
  public:
    ADSTATD(MD md, CHST chst) {}
    // 1 0 MD[1:0] 0 0 0 1 CHST[2:0]
    uint16_t toValue() const {
      return 0b10000001000 |
             ((uint8_t) md   & 0b011) << 7 |
             ((uint8_t) chst & 0b111) << 0
    }
}

// Start Self Test Status Group Conversion and Poll Status
class STATST : public LTC68xxCommandCode {
  public:
    STATST(MD md, ST st) {}
    // 1 0 MD[1:0] ST[1:0] 0 1 1 1 1
    uint16_t toValue() const {
      return 0b10000001111 |
             ((uint8_t) md   & 0b011) << 7 |
             ((uint8_t) st   & 0b011) << 5
    }
}

// Start Combined Cell Voltage and GPIO1, GPIO2 Conversion and Poll Status
class ADCVAX : public LTC68xxCommandCode {
  public:
    ADCVAX(MD md, DCP dcp) {}
    // 1 0 MD[1:0] 1 1 DCP 1 1 1 1
    uint16_t toValue() const {
      return 0b10001101111 |
             ((uint8_t) md   & 0b011) << 7 |
             ((uint8_t) dcp  & 0b001) << 4
    }
}

// Start Combined Cell Voltage and SC Conversion and Poll Status
class ADCVSC : public LTC68xxCommandCode {
  public:
    ADCVSC(MD md, DCP dcp) {}
    // 1 0 MD[1:0] 1 1 DCP 0 1 1 1
    uint16_t toValue() const {
      return 0b10001100111 |
             ((uint8_t) md   & 0b011) << 7 |
             ((uint8_t) dcp  & 0b001) << 4
    }
}

// Clear Cell Voltage Register Groups
class CLRCELL : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b11100010001; }
}

// Clear Auxiliary Register Groups
class CLRAUX : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b11100010010; }
}

// Clear Status Register Groups
class CLRSTAT : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b11100010011; }
}

// Poll ADC Conversion Status
class PLADC : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b11100010100; }
}

// Diagnose MUX and Poll Status
class DIAGN : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b11100010101; }
}

// Write COMM Register Group
class WRCOMM : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b11100100001; }
}

// Read COMM Register Group
class RDCOMM : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b11100100010; }
}

// Start I2C /SPI Communication
class STCOMM : public LTC68xxCommandCode {
  uint16_t toValue() const { return 0b11100100011; }
}