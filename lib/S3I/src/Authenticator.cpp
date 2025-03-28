#include "Authenticator.h"

using namespace S3I;

Token::Token(const char* Response) {
    Valid = false;

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, Response);
    if (err) {
        LOG_ERROR_1("S3I", "Failed to parse token response", err.c_str());
        return;
    }
    LOG_INFO("S3I", "Parsed token response");

    // Severe
    if (doc["access_token"].is<const char*>()) {
        AccessToken = doc["access_token"];
    } else {
        LOG_ERROR("S3I", "No access token in response");
        return;
    }

    if (doc["token_type"].is<const char*>()) {
        TokenType = doc["token_type"];
    } else {
        LOG_ERROR("S3I", "No token type in response");
        return;
    }

    // Not severe
    if (doc["refresh_token"].is<const char*>()) {
        RefreshToken = doc["refresh_token"];
    } else {
        RefreshToken = nullptr;
        LOG_WARN("S3I", "No refresh token in response");
    }

    if (doc["expires_in"].is<uint32_t>()) {
        ExpiresAt = millis() + doc["expires_in"].as<uint32_t>();
    } else if (doc["expires_at"].is<uint32_t>()) {
        ExpiresAt = doc["expires_at"].as<uint32_t>();
    } else {
        ExpiresAt = 0;
        LOG_WARN("S3I", "No expiration time in response");
    }

    Valid = true;

    LOG_INFO("S3I", "Token parsed successfully");
}

Token Authenticator::Authenticate() {
    String Body =
        "grant_type=client_credentials&client_id=" + String(m_Credentials.Id) +
        "&client_secret=" + String(m_Credentials.Secret);

    // TODO: Check if the client is a memory heavy object
    WiFiClientSecure Client;

    if (m_IDPServer.Certificate) {
        Client.setCACert(m_IDPServer.Certificate);
    } else {
        LOG_WARN("S3I",
                 "No certificate for IDP server. Using an insecure connection");
    }

    if (!Client.connect(m_IDPServer.Server, 443)) {
        LOG_ERROR("S3I", "Failed to connect to IDP server");
        return Token();
    }
    LOG_INFO("S3I", "Connected to IDP server");

    Client.printf("POST /%s HTTP/1.1\n", m_IDPServer.Path);
    Client.printf("Host: %s\n", m_IDPServer.Server);
    Client.printf("Content-Type: application/x-www-form-urlencoded\n");
    Client.printf("Content-Length: %d\n\n", Body.length());
    Client.printf("%s", Body.c_str());

    LOG_INFO("S3I", "Request sent! Waiting for response");

    // Read headers and discard them ]:->
    while (Client.connected()) {
        String Header = Client.readStringUntil('\n');
        LOG_DEBUG_1("S3I", "Got header", Header.c_str());

        if (Header == "\r") break;
    }
    LOG_INFO("S3I", "Headers read! Starting to read body");

    // Read the body
    String Response;
    while (Client.available()) {
        char c = Client.read();
        Response += c;
    }

    if (Response.length() == 0) {
        LOG_ERROR("S3I", "No response from IDP server");
        return Token();
    }

    Client.stop();

    LOG_INFO("S3I", "Response read! Parsing token");
    Serial.println(Response);

    m_LatestToken =
        Token(Response.c_str());  // Constructor will parse the response
    return m_LatestToken;
}

Token Authenticator::GetToken() {
    Authenticate();  // Implement smart token refreshing and caching
    return m_LatestToken;
}
