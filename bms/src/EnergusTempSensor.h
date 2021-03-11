#pragma once

#include <cstdint>
#include <optional>

// Convert voltage reading from LTC6811 to temperature measurement for
// Energus temp sensor
//
// voltage: voltage in mV
// value: temp as unsigned byte
std::optional<int8_t> convertTemp(uint16_t voltage);
