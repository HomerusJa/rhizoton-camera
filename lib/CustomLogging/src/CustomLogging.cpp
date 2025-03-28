#include "CustomLogging.h"

namespace CustomLogging {

namespace SerialLog {

static logging::Logger* Logger = nullptr;
static bool EnableSerialLogging = false;

void InitSerialLogger(bool enable) {
    EnableSerialLogging = enable;

    if (EnableSerialLogging) {
        Serial.begin(115200);
    }

    if (!EnableSerialLogging) {
        if (Logger != nullptr) {
            delete Logger;
            Logger = nullptr;
        }
        return;
    }

    Logger = new logging::Logger();
}

void LogToSerial(logging::LoggerLevel level, const char* module,
                 const char* message) {
    if (!EnableSerialLogging) return;

    if (Logger == nullptr) return;

    Logger->log(level, module, message);
}

void LogToSerial(logging::LoggerLevel level, const char* module,
                 const char* message, const char* arg) {
    if (!EnableSerialLogging) return;

    String FullMsg = String(message) + " Arg: " + arg;
    LogToSerial(level, module, FullMsg.c_str());
}

void LogToSerial(logging::LoggerLevel level, const char* module,
                 const char* message, const char* arg1, const char* arg2) {
    if (!EnableSerialLogging) return;

    String FullMsg = String(message) + " Arg1: " + arg1 + ", Arg2: " + arg2;
    LogToSerial(level, module, FullMsg.c_str());
}

}  // namespace SerialLog

}  // namespace CustomLogging
