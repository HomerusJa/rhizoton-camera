#pragma once

#include <Arduino.h>

constexpr uint32_t DEFAULT_GMT_OFFSET = 2 * 3600;  // GMT+2
constexpr uint32_t DEFAULT_DAYLIGHT_OFFSET = 0;

namespace TimeTools {
void InitTime(uint32_t GMT_OFFSET = DEFAULT_GMT_OFFSET,
              uint32_t DAYLIGHT_OFFSET = DEFAULT_DAYLIGHT_OFFSET);

uint32_t GetTimestamp();
}  // namespace TimeTools
