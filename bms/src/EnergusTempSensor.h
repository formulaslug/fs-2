#pragma once

#include <cstdint>
#include "tl/optional.hpp"

// Convert voltage reading from LTC6811 to temperature measurement for
// Energus temp sensor
//
// voltage: voltage in mV
// value: temp as unsigned byte
tl::optional<int8_t> convertTemp(uint16_t voltage);
