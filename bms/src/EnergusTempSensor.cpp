#include "EnergusTempSensor.h"

#include <array>
#include <initializer_list>

struct ConversionPoint {
  int8_t temp;
  uint16_t voltage;
};

static constexpr size_t tempLookupTableSize = 33;
// Pairs of temp and corresponding voltage. Temp in celsius, voltage in mV
static constexpr std::array<ConversionPoint, tempLookupTableSize>
    tempLookupTable{
        {{-40, 2440}, {-35, 2420}, {-30, 2400}, {-25, 2380}, {-20, 2350},
         {-15, 2320}, {-10, 2270}, {-5, 2230},  {0, 2170},   {5, 2110},
         {10, 2050},  {15, 1990},  {20, 1920},  {25, 1860},  {30, 1800},
         {35, 1740},  {40, 1680},  {45, 1630},  {50, 1590},  {55, 1550},
         {60, 1510},  {65, 1480},  {70, 1450},  {75, 1430},  {80, 1400},
         {85, 1380},  {90, 1370},  {95, 1350},  {100, 1340}, {105, 1330},
         {110, 1320}, {115, 1310}, {120, 1300}}};

static int8_t linearInterpolateSegment(ConversionPoint low,
                                       ConversionPoint high, uint16_t value) {
  if (value < low.voltage || value > high.voltage) return -127;

  return low.temp + ((value - low.voltage) * (high.temp - low.temp) /
                     (high.voltage - low.voltage));
}

// Convert voltage reading from LTC6811 to temperature measurement for
// Energus temp sensor
//
// voltage: voltage in mV
// value: temp as unsigned byte
tl::optional<int8_t> convertTemp(uint16_t voltage) {
  for (unsigned int i = 0; i < tempLookupTableSize; i++) {
    if (voltage > tempLookupTable[i].voltage) {
      if (i != tempLookupTableSize - 1)
        return linearInterpolateSegment(tempLookupTable[i],
                                        tempLookupTable[i - 1], voltage);
      else
        return tempLookupTable[i].temp;
    }
  }

  // Fallback value
  return {};
}
