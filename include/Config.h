#pragma once

#include <ArduinoJson.h>
#include <SD_MMC.h>

#include <memory>

#include "Utils.h"

namespace RhizotronCam {
class Config {
   public:
    Config(const char* path) : m_Path(path) {};
    ~Config() = default;

    bool Load();

    bool Exists(const char* category, const char* key, bool logging = true) const;

    JsonObject Get(const char* category) const;
    JsonVariant Get(const char* category, const char* key) const;

    // template <typename T>
    // T Config::Get(const char* category, const char* key) const;

    template <typename T>
    bool Get(const char* category, const char* key, T* target) const;

    template <typename T>
    bool Update(const char* category, const char* key, const T value,
                bool ignoreMissing = false, bool save = true);

    bool Save() const;

   private:
    const char* m_Path;
    std::unique_ptr<JsonDocument> m_Document;
};
}  // namespace RhizotronCam

// This is the global configuration object
extern RhizotronCam::Config g_Config;
