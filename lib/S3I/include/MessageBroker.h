#pragma once

#include <CustomLogging.h>
// #include <WiFi.h>
#include <WiFiClientSecure.h>

#include "Authenticator.h"
#include "Writers.h"

namespace S3I {

class MessageBroker {
   public:
    MessageBroker(const char* BrokerUrl, const char* OwnEndpoint,
                  Authenticator* Authenticator)
        : m_BrokerUrl(BrokerUrl),
          m_OwnEndpoint(OwnEndpoint),
          m_Authenticator(Authenticator) {}
    MessageBroker() = default;

    String Send(const char* Endpoint, Writers::WiFiWriter& Writer);
    String Receive(bool multiple = false);

    int operator=(const MessageBroker& other) {
        m_BrokerUrl = other.m_BrokerUrl;
        m_OwnEndpoint = other.m_OwnEndpoint;
        m_Authenticator = other.m_Authenticator;
        return 0;
    }

   private:
    Authenticator* m_Authenticator;

    const char* m_BrokerUrl;
    const char* m_OwnEndpoint;
};

}  // namespace S3I
