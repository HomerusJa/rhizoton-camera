#pragma once

#include <Arduino.h>
#include <AsyncUDP.h>
#include <CustomLogging.h>
#include <SD_MMC.h>
#include <Timers.h>
#include <WiFi.h>
#include <time.h>

#include <memory>
#include <vector>

#include "Config.h"

constexpr uint32_t GMT_OFFSET = 2 * 3600;  // GMT+2
constexpr uint32_t DAYLIGHT_OFFSET = 0;

namespace RhizotronCam {
namespace Utils {
void InitSD();
bool ConnectToWiFi(time_t timeout_ms = 30000);

void ReadSerialCommands(bool &IsRestartTime, bool &IsCaptureTime,
                        bool &IsFetchTime, Timers::Timers &g_Timers);

void Restart(uint32_t delay_ms = 1000);

void morse_blink(uint8_t blink_pin, const char *morse_code);
}  // namespace Utils
}  // namespace RhizotronCam

namespace future {
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&...args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}  // namespace future
