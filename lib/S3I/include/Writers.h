#pragma once

#include <FS.h>
#include <WiFi.h>

#include <algorithm>
#include <vector>

#include "extern/base64_stripped.hpp"

#define DEFAULT_CHUNK_SIZE 768

namespace S3I {
namespace Writers {
class WiFiWriter {
   public:
    virtual size_t Length() const = 0;
    virtual void Write(WiFiClient *client,
                       size_t chunk_size = DEFAULT_CHUNK_SIZE) = 0;
};

class CharWiFiWriter : public WiFiWriter {
   public:
    explicit CharWiFiWriter(const char *data) : m_Data(data) {};
    size_t Length() const override;
    void Write(WiFiClient *client, size_t chunk_size = DEFAULT_CHUNK_SIZE);

   private:
    const char *m_Data;
};

// FIXME: Change the B64 style to B64Url so that it is valid JSON
class B64WiFiWriter : public WiFiWriter {
   public:
    explicit B64WiFiWriter(fs::FS &fs, const char *filename)
        : m_FS(fs), m_Filename(filename) {};
    size_t Length() const override;
    void Write(WiFiClient *client, size_t chunk_size = DEFAULT_CHUNK_SIZE);

   private:
    fs::FS &m_FS;
    const char *m_Filename;
};

class ComposedWiFiWriter : public WiFiWriter {
   public:
    explicit ComposedWiFiWriter(const std::vector<WiFiWriter *> &writers)
        : m_Writers(writers) {}
    size_t Length() const override;
    void Write(WiFiClient *client, size_t chunk_size = DEFAULT_CHUNK_SIZE);

   private:
    std::vector<WiFiWriter *> m_Writers;
};

}  // namespace Writers

}  // namespace S3I
