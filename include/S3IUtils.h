#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <S3I.h>
#include <SD_MMC.h>
#include <Timers.h>

#include <memory>
#include <string>
#include <vector>

#include "Camera.h"
#include "Config.h"
#include "Utils.h"

extern S3I::MessageBroker* g_MessageBroker;
extern RhizotronCam::Config g_Config;
extern Timers::Timers g_Timers;

namespace RhizotronCam {
namespace S3IUtils {

void SendErrorAsEvent(const char* ErrorText, const char* ErrorDetail,
                      const char* ErrorSource);
void SendStatusAsEvent(const char* StatusName,
                       const char* StatusDetail = nullptr);
void SendImageAsEvent(const char* ImagePath);
void SendImage(const char* Part1, const char* ImagePath, const char* Part2,
               const char* Endpoint);
void FetchAndProcessMessages();

}  // namespace S3IUtils
}  // namespace RhizotronCam