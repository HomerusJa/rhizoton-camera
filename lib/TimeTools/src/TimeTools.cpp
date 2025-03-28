#include "TimeTools.h"

namespace TimeTools {

void InitTime(uint32_t GMT_OFFSET, uint32_t DAYLIGHT_OFFSET) {
    configTime(GMT_OFFSET, DAYLIGHT_OFFSET, "pool.ntp.org");
    Serial.println("Time initialized with GMT_OFFSET: " + String(GMT_OFFSET) +
                   " and DAYLIGHT_OFFSET: " + String(DAYLIGHT_OFFSET));
}

uint32_t GetTimestamp() {
    time_t now;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return 0;
    }
    time(&now);

    Serial.println("Obtained timestamp: " + String(now));

    return now;
}

}  // namespace TimeTools
