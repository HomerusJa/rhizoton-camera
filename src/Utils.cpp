#include "Utils.h"

namespace RhizotronCam {

namespace Utils {

void InitSD() {
    // This way, the SD card doesn't interfere with the flashlight at GPIO 4
    // https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/
    // ## Flashlight (GPIO 4)
    while (!SD_MMC.begin("", true)) {
        LOG_ERROR("Setup", "Failed to mount SD card");
        delay(1000);
    }
    LOG_INFO("Setup", "SD card initialized");
}

bool ConnectToWiFi(time_t timeout_ms) {
    const char *WIFI_SSID = g_Config.Get("wifi", "ssid").as<const char *>();
    const char *WIFI_PASSWORD =
        g_Config.Get("wifi", "password").as<const char *>();

    if (WiFi.status() == WL_CONNECTED || WiFi.SSID() == WIFI_SSID) {
        LOG_INFO("Setup", "Already connected to WiFi");
        return true;
    }

    DEBUG_PRINTF("SSID: '%s', PW: '%s'\n", WIFI_SSID, WIFI_PASSWORD);

    LOG_INFO("Setup", "Connecting to WiFi");
    time_t end_time = millis() + timeout_ms;
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() > end_time) {
            LOG_ERROR("Setup", "Failed to connect to WiFi");
            return false;
        }

        delay(500);
        DEBUG_PRINT(".");
    }
    DEBUG_PRINTLN("");
    LOG_INFO("Setup", "Connected to WiFi");
    return true;
}

void ReadSerialCommands(bool &IsRestartTime, bool &IsCaptureTime,
                        bool &IsFetchTime, Timers::Timers &g_Timers) {
    while (Serial.available()) {
        Serial.println();
        String command = Serial.readStringUntil('\n');
        command.trim();
        int spaceIndex = command.indexOf(' ');
        String commandFirst =
            (spaceIndex == -1) ? command : command.substring(0, spaceIndex);

        if (commandFirst == "timer-status") {
            // Iterate through the timers and print their status
            for (const char *timerName : {"Capture", "Fetch", "Restart"}) {
                Serial.printf("Timer '%s' has %lu ms remaining\n", timerName,
                              g_Timers.getRemainingTime(timerName));
            }
        } else if (commandFirst == "restart") {
            IsRestartTime = true;
        } else if (commandFirst == "capture") {
            IsCaptureTime = true;
        } else if (commandFirst == "fetch") {
            IsFetchTime = true;
        } else if (commandFirst == "config") {
            File file = SD_MMC.open("/config.json", "r");
            if (!file) {
                Serial.println("Failed to open file for reading");
            } else {
                Serial.println(file.readString());
                file.close();
            }
        } else if (commandFirst == "help" || commandFirst == "?" ||
                   commandFirst == "h") {
            Serial.println("Commands:");
            Serial.println("  timer-status - Print timer status");
            Serial.println("  restart - Trigger restart timer");
            Serial.println("  capture - Trigger capture timer");
            Serial.println("  fetch - Trigger fetch timer");
            Serial.println("  config - Print config file");
        } else {
            Serial.printf("Unknown command '%s'\nFor help, send 'help'",
                          commandFirst.c_str());
        }
    }
}

void Restart(uint32_t delay_ms) {
    LOG_INFO("Restart", "Restarting");
    delay(delay_ms);
    ESP.restart();
}

}  // namespace Utils

}  // namespace RhizotronCam
