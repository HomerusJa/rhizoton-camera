#include "Config.h"

namespace RhizotronCam {

bool Config::Load() {
    LOG_INFO("config", "Loading config file");
    if (m_Document) {
        LOG_WARN("config", "Document already loaded");
        m_Document.reset();
    }

    // Check if the file system is mounted
    File configFile = SD_MMC.open(m_Path, "r");
    if (!configFile) {
        LOG_ERROR("config", "Failed to open config file");
        return false;
    }

    m_Document = future::make_unique<JsonDocument>();
    DeserializationError error = deserializeJson(*m_Document, configFile);
    if (error) {
        LOG_ERROR("config", "Failed to parse config file");
        m_Document.reset();
        m_Document = nullptr;
        return false;
    }

    return true;
}

bool Config::Exists(const char* category, const char* key, bool logging) const {
    if (!m_Document) {
        if (logging) LOG_ERROR("config", "Document not loaded");
        return false;
    }

    if (!(*m_Document)[category].is<JsonObject>()) {
        if (logging) LOG_ERROR_1("config", "Category not found", category);
        return false;
    }

    if (!(*m_Document)[category][key].is<JsonVariant>()) {
        if (logging) LOG_ERROR_1("config", "Key not found", key);
        return false;
    }

    return true;
}

JsonObject Config::Get(const char* category) const {
    LOG_DEBUG_1("config", "Getting category", category);

    if (!m_Document) {
        LOG_ERROR("config", "Document not loaded");
        return JsonObject();
    }
    LOG_DEBUG("config", "Document loaded");

    if (!(*m_Document)[category].is<JsonObject>()) {
        LOG_ERROR_1("config", "Category not found", category);
        return JsonObject();
    }
    LOG_DEBUG_1("config", "Category found", category);

    JsonObject Result = (*m_Document)[category].as<JsonObject>();

    if (Result.isNull()) {
        LOG_ERROR("config", "Category is null");
        return JsonObject();
    }

    // DEBUG
    // DEBUG_PRINTLN("Printing category");
    // for (JsonPair kv : Result) {
    //     DEBUG_PRINTF("Key: '%s'; Value: '%s' \n", kv.key().c_str(),
    //     "Unknown");
    // }

    return Result;
}

JsonVariant Config::Get(const char* category, const char* key) const {
    LOG_DEBUG_2("config", "Getting CATEGORY.KEY", category, key);

    JsonObject CategoryDoc = Get(category);

    if (!CategoryDoc[key].is<JsonVariant>()) {
        LOG_ERROR_1("config", "Key not found", key);

        return JsonVariant();
    }
    LOG_DEBUG("config", "Key found");

    JsonVariant Result = CategoryDoc[key].as<JsonVariant>();
    LOG_INFO_2("config", "Found value", category, key);
    return Result;
}

// template <typename T>
// T Config::Get(const char* category, const char* key) const {
//     JsonVariantConst value = this->Get(category, key);
//     return value.as<T>();
// }

template <typename T>
bool Config::Get(const char* category, const char* key, T* target) const {
    JsonVariantConst value = this->Get(category, key);
    if (value.isNull()) {
        LOG_INFO("config", "Value is null");
        return false;
    }

    *target = value.as<T>();
    return true;
}

template <typename T>
bool Config::Update(const char* category, const char* key, const T value,
                    bool ignoreMissing, bool save) {
    LOG_INFO_2("config", "Updating CATEGORY.KEY", category, key);
    if (!m_Document) {
        LOG_ERROR("config", "Document not loaded");
        return false;
    }

    if (!(*m_Document)[category].is<JsonObject>()) {
        if (!ignoreMissing) {
            LOG_ERROR("config", "Category not found");
            return false;
        }

        (*m_Document)[category] = JsonDocument().to<JsonObject>();
    }

    JsonVariantConst CategoryDoc = (*m_Document)[category];

    if (!CategoryDoc[key].is<JsonVariant>() && !ignoreMissing) {
        LOG_ERROR("config", "Key not found");
        return false;
    }

    (*m_Document)[category][key].set<T>(value);

    if (save) {
        return Save();
    }

    return true;
}

template bool Config::Update<const char*>(const char*, const char*, const char*,
                                          bool, bool);

bool Config::Save() const {
    LOG_INFO("config", "Saving config file");
    if (!m_Document) {
        LOG_ERROR("config", "Document not loaded");
        return false;
    }

    File configFile = SD_MMC.open(m_Path, "w");
    if (!configFile) {
        LOG_ERROR("config", "Failed to open config file");
        return false;
    }

    if (serializeJson(*m_Document, configFile) == 0) {
        LOG_ERROR("config", "Failed to write config file");
        return false;
    }

    return true;
}

}  // namespace RhizotronCam

RhizotronCam::Config g_Config("/config.json");