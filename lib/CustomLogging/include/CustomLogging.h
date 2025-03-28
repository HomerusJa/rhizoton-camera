#pragma once

#include <logger.h>

namespace CustomLogging {

namespace SerialLog {
void InitSerialLogger(bool enable = true);

void LogToSerial(logging::LoggerLevel level, const char* module,
                 const char* message);
void LogToSerial(logging::LoggerLevel level, const char* module,
                 const char* message, const char* arg);
void LogToSerial(logging::LoggerLevel level, const char* module,
                 const char* message, const char* arg1, const char* arg2);
}  // namespace SerialLog

}  // namespace CustomLogging

#undef LOG_DEBUG
#undef LOG_DEBUG_1
#undef LOG_DEBUG_2

#undef LOG_INFO
#undef LOG_INFO_1
#undef LOG_INFO_2

#undef LOG_WARN
#undef LOG_WARN_1
#undef LOG_WARN_2

#undef LOG_ERROR
#undef LOG_ERROR_1
#undef LOG_ERROR_2

#undef DEBUG_PRINTF
#undef DEBUG_PRINTLN
#undef DEBUG_PRINT

#define LOG_DEBUG(module, message)         \
    CustomLogging::SerialLog::LogToSerial( \
        logging::LoggerLevel::LOGGER_LEVEL_DEBUG, module, message)

#define LOG_DEBUG_1(module, message, arg)  \
    CustomLogging::SerialLog::LogToSerial( \
        logging::LoggerLevel::LOGGER_LEVEL_DEBUG, module, message, arg)

#define LOG_DEBUG_2(module, message, arg1, arg2) \
    CustomLogging::SerialLog::LogToSerial(       \
        logging::LoggerLevel::LOGGER_LEVEL_DEBUG, module, message, arg1, arg2)

#define LOG_INFO(module, message)          \
    CustomLogging::SerialLog::LogToSerial( \
        logging::LoggerLevel::LOGGER_LEVEL_INFO, module, message)

#define LOG_INFO_1(module, message, arg)   \
    CustomLogging::SerialLog::LogToSerial( \
        logging::LoggerLevel::LOGGER_LEVEL_INFO, module, message, arg)

#define LOG_INFO_2(module, message, arg1, arg2) \
    CustomLogging::SerialLog::LogToSerial(      \
        logging::LoggerLevel::LOGGER_LEVEL_INFO, module, message, arg1, arg2)

#define LOG_WARN(module, message)          \
    CustomLogging::SerialLog::LogToSerial( \
        logging::LoggerLevel::LOGGER_LEVEL_WARN, module, message)

#define LOG_WARN_1(module, message, arg)   \
    CustomLogging::SerialLog::LogToSerial( \
        logging::LoggerLevel::LOGGER_LEVEL_WARN, module, message, arg)

#define LOG_WARN_2(module, message, arg1, arg2) \
    CustomLogging::SerialLog::LogToSerial(      \
        logging::LoggerLevel::LOGGER_LEVEL_WARN, module, message, arg1, arg2)

#define LOG_ERROR(module, message)         \
    CustomLogging::SerialLog::LogToSerial( \
        logging::LoggerLevel::LOGGER_LEVEL_ERROR, module, message)

#define LOG_ERROR_1(module, message, arg)  \
    CustomLogging::SerialLog::LogToSerial( \
        logging::LoggerLevel::LOGGER_LEVEL_ERROR, module, message, arg)

#define LOG_ERROR_2(module, message, arg1, arg2) \
    CustomLogging::SerialLog::LogToSerial(       \
        logging::LoggerLevel::LOGGER_LEVEL_ERROR, module, message, arg1, arg2)

#define DEBUG_PRINTF(format, ...) Serial.printf(format, __VA_ARGS__)
#define DEBUG_PRINTLN(text) Serial.println(text)
#define DEBUG_PRINT(text) Serial.print(text)
