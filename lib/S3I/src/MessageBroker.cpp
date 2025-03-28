#include "MessageBroker.h"

#include <DefaultCertificate.h>

String S3I::MessageBroker::Send(const char* Endpoint,
                                     Writers::WiFiWriter& Writer) {
    if (m_Authenticator == nullptr) {
        LOG_ERROR("S3I", "Authenticator is null");
        return String();
    }
    const Token& TokenObj = m_Authenticator->GetToken();
    if (!TokenObj) {
        LOG_ERROR("S3I", "Token is not valid");
        return String();
    }

    String TokenStr = TokenObj.GetFullToken();

    // TODO: Refactor to reduce code duplication with
    // Authenticator::Authenticate
    WiFiClientSecure Client;

    Client.setCACert(GetDefaultCertificate());

    if (!Client.connect(m_BrokerUrl, 443)) {
        LOG_ERROR("S3I", "Connection to broker failed");
        return String();
    }
    LOG_INFO("S3I", "Connected to broker");

    LOG_INFO("S3I", "Starting to write headers");
    Client.printf("POST /%s HTTP/1.1\n", Endpoint);
    Client.printf("Host: %s\n", m_BrokerUrl);
    Client.printf("Authorization: %s\n", TokenStr.c_str());
    Client.printf("Content-Type: application/json\n");
    Client.printf("Content-Length: %d\n", Writer.Length());
    Client.printf("\n");

    LOG_INFO("S3I", "Starting to write body");
    Writer.Write(&Client);

    LOG_INFO("S3I", "Starting to read headers");
    while (Client.connected()) {
        String line = Client.readStringUntil('\n');
        LOG_DEBUG_1("S3I", "Header", line.c_str());

        if (line == "\r") break;
    }

    LOG_INFO("S3I", "Starting to read body");
    String Response;
    while (Client.available()) {
        char c = Client.read();
        Response += c;
    }


    LOG_INFO_2("S3I", "Read everything! LENGTH, CONTENT",
               String(Response.length()).c_str(), Response.c_str());

    Client.stop();
    return Response;
}

String S3I::MessageBroker::Receive(bool multiple) {
    if (multiple) {
        LOG_WARN("S3I", "Multiple messages not supported yet");
        // TODO: Implement multiple messages
    }

    Token TokenObj = m_Authenticator->GetToken();
    if (!TokenObj) {
        LOG_ERROR("S3I", "Token is not valid");
        return String();
    }
    String TokenStr = TokenObj.GetFullToken();

    // TODO: Refactor to reduce code duplication with
    // Authenticator::Authenticate
    WiFiClientSecure Client;

    Client.setCACert(GetDefaultCertificate());

    if (!Client.connect(m_BrokerUrl, 443)) {
        LOG_ERROR("S3I", "Connection to broker failed");
        return String();
    }
    LOG_INFO("S3I", "Connected to broker");

    LOG_INFO("S3I", "Starting to write headers");
    Client.printf("GET /%s HTTP/1.1\n", m_OwnEndpoint);
    Client.printf("Host: %s\n", m_BrokerUrl);
    Client.printf("Authorization: %s\n", TokenStr.c_str());
    Client.printf("Content-Type: application/json\n");
    Client.printf("Connection: close\n");
    Client.printf("\n");

    LOG_INFO("S3I", "Starting to read headers");
    while (Client.connected()) {
        String line = Client.readStringUntil('\n');
        LOG_DEBUG_1("S3I", "Got Header", line.c_str());

        if (line == "\r") break;
    }

    LOG_INFO("S3I", "Starting to read body");
    String Response;
    while (Client.available()) {
        char c = Client.read();
        Response += c;
    }

    LOG_INFO_2("S3I", "Read everything! LENGTH, CONTENT",
               String(Response.length()).c_str(), Response.c_str());

    Client.stop();
    return Response;
}