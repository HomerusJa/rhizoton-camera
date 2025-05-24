#include "S3IUtils.h"

#include <TimeTools.h>

static bool IsErrorMessage(const String& Response) {
    if (Response.isEmpty()) return true;

    if (Response.indexOf("\"error\"") != -1) return true;
    return false;
}

void RhizotronCam::S3IUtils::SendStatusAsEvent(const char* StatusName,
                                               const char* StatusDetail) {
    const char* EventName =
        g_Config.Get("s3i", "status_event_name").as<const char*>();

    const char* StatusDetailContent = "";
    if (StatusDetail == nullptr) {
        // Send empty string if null
        StatusDetailContent = "\"\"";
    } else if (StatusDetail[0] == '{') {
        // Send as JSON object if it starts with '{'
        StatusDetailContent = StatusDetail;
    } else {
        // Wrap in quotes if it is a string that is not a JSON object
        StatusDetailContent = (String("\"") + StatusDetail + "\"").c_str();
    }

    // clang-format off
    String Body = String("{") +
        "\"sender\": \""         + g_Config.Get("s3i", "id").as<const char*>() + "\", "
        "\"identifier\": \""     + S3I::CreateMessageIdentifier() + "\", "
        "\"timestamp\": "        + String(TimeTools::GetTimestamp()) + ", "
        "\"topic\": \""          + EventName + "\", "
        "\"messageType\": \"eventMessage\", "
        "\"content\": {"
            "\"type\": \"status\", "
            "\"status\": \""     + StatusName + "\", "
            "\"detail\": "       + String(StatusDetailContent) + 
        "}"
        "}";
    // clang-format on

    DEBUG_PRINTF("Message Content: <<<%s>>>\n", Body.c_str());

    S3I::Writers::CharWiFiWriter BodyWriter(Body.c_str());
    String Response = g_MessageBroker->Send(EventName, BodyWriter);

    if (IsErrorMessage(Response)) {
        LOG_ERROR_1("S3IUtils", "Failed to send status, RESPONSE",
                    Response.c_str());
        return;
    }

    LOG_INFO_1("S3IUtils", "Status sent! RESPONSE", Response.c_str());
}

void RhizotronCam::S3IUtils::SendErrorAsEvent(const char* ErrorText,
                                              const char* ErrorDetail,
                                              const char* ErrorSource) {
    const char* EventName =
        g_Config.Get("s3i", "status_event_name").as<const char*>();

    // clang-format off
    String Body = String("{") +
        "\"sender\": \""         + g_Config.Get("s3i", "id").as<const char*>() + "\", "
        "\"identifier\": \""     + S3I::CreateMessageIdentifier() + "\", "
        "\"timestamp\": "        + String(TimeTools::GetTimestamp()) + ", "
        "\"topic\": \""          + EventName + "\", "
        "\"messageType\": \"eventMessage\", "
        "\"content\": {"
            "\"type\": \"status\", "
            "\"status\": \"error\", "
            "\"source\": \""     + ErrorSource + "\", "
            "\"detail\": \""     + ErrorDetail + "\", "
            "\"error_text\": \"" + ErrorText + "\""
        "}"
        "}";
    // clang-format on

    DEBUG_PRINTF("Message Content: <<<%s>>>\n", Body.c_str());

    S3I::Writers::CharWiFiWriter BodyWriter(Body.c_str());
    String Response = g_MessageBroker->Send(EventName, BodyWriter);

    if (IsErrorMessage(Response)) {
        LOG_ERROR_1("S3IUtils", "Failed to send error, RESPONSE",
                    Response.c_str());
        return;
    }

    LOG_INFO_1("S3IUtils", "Status sent! RESPONSE", Response.c_str());
}

void RhizotronCam::S3IUtils::SendImageAsEvent(const char* ImagePath) {
    const char* EventName =
        g_Config.Get("s3i", "image_event_name").as<const char*>();

    // clang-format off
    String Part1 = String("{") +
        "\"sender\": \""         + g_Config.Get("s3i", "id").as<const char*>() + "\", "
        "\"identifier\": \""     + S3I::CreateMessageIdentifier() + "\", "
        "\"timestamp\": "        + String(TimeTools::GetTimestamp()) + ", "
        "\"topic\": \""          + EventName + "\", "
        "\"messageType\": \"eventMessage\", "
        "\"content\": {"
            "\"type\": \"image/jpeg; encoding=base64url\", "
            "\"path\": \""       + String(ImagePath) + "\", "
            "\"takenAt\": "      + String(Camera::ExtractTimestampFromImage(ImagePath)) + ", "
            "\"image\": \"";
    // clang-format on

    const char* Part1Char = Part1.c_str();
    const char* Part2Char = R"("}})";

    DEBUG_PRINTF("Message Content: %s <<<IMAGE>>> %s\n", Part1Char, Part2Char);

    SendImage(Part1Char, ImagePath, Part2Char, EventName);
}

void RhizotronCam::S3IUtils::SendImage(const char* Part1, const char* ImagePath,
                                       const char* Part2,
                                       const char* Endpoint) {
    LOG_INFO_1("S3IUtils", "Sending image at", ImagePath);

    LOG_DEBUG("S3IUtils", "Creating message writers");
    S3I::Writers::CharWiFiWriter BodyPart1(Part1);
    S3I::Writers::B64WiFiWriter BodyPart2(SD_MMC, ImagePath);
    S3I::Writers::CharWiFiWriter BodyPart3(Part2);

    S3I::Writers::ComposedWiFiWriter BodyWriter(
        std::vector<S3I::Writers::WiFiWriter*>{&BodyPart1, &BodyPart2,
                                               &BodyPart3});
    LOG_DEBUG("S3IUtils", "Message writers created");

    LOG_DEBUG("S3IUtils", "Sending message");
    String Response = g_MessageBroker->Send(Endpoint, BodyWriter);

    if (IsErrorMessage(Response)) {
        LOG_ERROR_1("S3IUtils", "Failed to send image, RESPONSE",
                    Response.c_str());
        String Detail = String("{\"response\": \"") + Response +
                        "\", \"path\": \"" + ImagePath + "\"}";
        SendErrorAsEvent("Failed to send image", Detail.c_str(), "S3I");
        return;
    }

    // TODO: Check the response for errors
    LOG_INFO_1("S3IUtils", "Image sent! RESPONSE", Response.c_str());
}

static bool IsImageRequest(const JsonDocument& doc) {
    return strcmp(doc["messageType"], "getValueRequest") == 0 &&
           String(doc["attributePath"].as<const char*>()).startsWith("image/");
}

static bool IsConfigurationRequest(const JsonDocument& doc) {
    return strcmp(doc["messageType"], "getValueRequest") == 0 &&
           String(doc["attributePath"].as<const char*>())
               .startsWith("configuration/");
}

static bool IsConfigurationUpdate(const JsonDocument& doc) {
    return strcmp(doc["messageType"], "setValueRequest") == 0 &&
           String(doc["attributePath"].as<const char*>())
               .startsWith("configuration/");
}

/**
 * @brief Checks if a configuration value is allowed to be read or written
 * @param Category
 * @param Key
 * @return The error message if the value is not allowed, or an empty string if
 * it is allowed
 */
String IsAllowedConfig(const char* Category, const char* Key) {
    if (Category == "s3i" && Key == "id") {
        return "The ID cannot be changed nor read";
    }
    if (Category == "s3i" && Key == "secret") {
        return "The secret cannot be changed nor read";
    }

    if (Category == "wifi" && Key == "ssid") {
        return "The SSID cannot be changed nor read";
    }
    if (Category == "wifi" && Key == "password") {
        return "The password cannot be changed nor read";
    }

    return "";
}

void RhizotronCam::S3IUtils::FetchAndProcessMessages() {
    LOG_INFO("S3IUtils", "Fetching messages");

    // Fetch one message
    String Response = g_MessageBroker->Receive();
    // Check if the response was an error message
    if (IsErrorMessage(Response)) {
        LOG_ERROR_1("S3IUtils", "Failed to fetch messages, RESPONSE",
                    Response.c_str());
        SendErrorAsEvent("Failed to fetch messages", Response.c_str(), "S3I");
        return;
    }

    if (Response.length() == 1 && Response.c_str()[0] == 255) {
        LOG_INFO("S3IUtils", "No messages to fetch");
        return;
    }
    LOG_INFO_1("S3IUtils", "Messages fetched! RESPONSE", Response.c_str());

    // Parse the response
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, Response);
    if (error) {
        LOG_ERROR_1("S3IUtils", "Failed to parse JSON", error.c_str());
        return;
    }
    LOG_INFO("S3IUtils", "JSON parsed");

    // Determine the action to take based on the message type

    // 1. Request for an image
    if (IsImageRequest(doc)) {
        LOG_INFO("S3IUtils", "Image request received");

        // 1. Get the image path
        String ImageIdentifier =
            String(doc["attributePath"].as<const char*>()).substring(6);
        String ImagePath = "";

        if (ImageIdentifier == "latest") {
            ImagePath = RhizotronCam::Camera::GenerateImagePathWithOffset(-1);
        } else if (!ImageIdentifier.toInt() == 0) {
            ImagePath = RhizotronCam::Camera::GenerateImagePathForNumber(
                ImageIdentifier.toInt());
        } else {
            LOG_ERROR_1("S3IUtils", "Invalid image identifier",
                        ImageIdentifier.c_str());

            return;
        }

        // 2. Send the image
        // clang-format off
        String Part1 = String("{\"sender\": \"") + g_Config.Get("s3i", "id").as<const char*>()
            + "\", \"identifier\": \"" + S3I::CreateMessageIdentifier()
            + "\", \"receivers\": [\"" + doc["replyToEndpoint"].as<const char*>()
            + "\"], \"messageType\": \"getValueReply\", \"replyingToMessage\": \"" + doc["identifier"].as<const char*>()
            + "\", \"value\": {\"type\": \"image/jpeg; encoding=base64url\", \"path\": \"" + String(ImagePath)
            + "\", \"takenAt\": " + String(Camera::ExtractTimestampFromImage(ImagePath.c_str()))
            + ", \"image\": \"";
        // clang-format on

        const char* Part1Char = Part1.c_str();
        const char* Part2Char = "\"}}";

        SendImage(
            Part1Char, ImagePath.c_str(), Part2Char,
            (String("s3ibs://") + doc["replyToEndpoint"].as<const char*>())
                .c_str());
        return;
    }

    // 2. Request for the current configuration
    if (IsConfigurationRequest(doc)) {
        LOG_INFO("S3IUtils", "Configuration request received");

        String ValueContent = "";

        // 1. Get the wanted value
        String AttributePath =
            String(doc["attributePath"].as<const char*>()).substring(14);

        const char* Category =
            AttributePath.substring(0, AttributePath.indexOf('/')).c_str();
        const char* Key =
            AttributePath.substring(AttributePath.indexOf('/') + 1).c_str();

        // 1.1 Check if the value is allowed to be read
        String Error = IsAllowedConfig(Category, Key);
        if (!Error.isEmpty()) {
            ValueContent = String("{\"success\": false, \"error\": \"") +
                           Error + "\", \"category\": \"" + Category +
                           "\", \"key\": \"" + Key + "\"}";
        } else if (!g_Config.Exists(Category, Key)) {
            // 2. Get the value
            LOG_ERROR_2("S3IUtils", "Configuration value does not exist",
                        Category, Key);
            ValueContent = String(
                               "{\"success\": false, \"error\": "
                               "\"Configuration value does "
                               "not exist\", \"category\": \"") +
                           Category + "\", \"key\": \"" + Key + "\"}";
        } else {
            const char* Value = g_Config.Get(Category, Key).as<const char*>();
            ValueContent = String("{\"success\": true, \"category\": \"") +
                           Category + "\", \"key\": \"" + Key +
                           "\", \"value\": \"" + Value + "\"}";
        }

        // clang-format off
        String Body = String("{\"sender\": \"") + g_Config.Get("s3i", "id").as<const char*>()
                    + "\", \"identifier\": \"" + S3I::CreateMessageIdentifier()
                    + "\", \"receivers\": [\"" + doc["replyToEndpoint"].as<const char*>()
                    + "\"], \"messageType\": \"getValueReply\", \"replyingToMessage\": \"" + doc["identifier"].as<const char*>()
                    + "\", \"value\": {\"category\": \"" + Category
                    + "\", \"key\": \"" + Key
                    + "\", \"value\": \"" + ValueContent + "\"}}";
        // clang-format on

        // 3. Send the value
        S3I::Writers::CharWiFiWriter BodyWriter(Body.c_str());

        g_MessageBroker->Send(
            (String("s3ibs://") + doc["replyToEndpoint"].as<const char*>())
                .c_str(),
            BodyWriter);
        return;
    }

    // 3. Update to the configuration
    if (IsConfigurationUpdate(doc)) {
        LOG_INFO("S3IUtils", "Configuration update received");

        bool Success = true;
        bool Restart = false;

        // 1. Get the wanted value
        String AttributePath =
            String(doc["attributePath"].as<const char*>()).substring(14);

        if (AttributePath.endsWith("/restart")) {
            Restart = true;
            AttributePath =
                AttributePath.substring(0, AttributePath.length() - 8);
        }

        const char* Category =
            AttributePath.substring(0, AttributePath.indexOf('/')).c_str();
        const char* Key =
            AttributePath.substring(AttributePath.indexOf('/') + 1).c_str();
        const char* Value = doc["value"]["value"].as<const char*>();

        // 1.1 Check if the value is allowed to be written
        String Error = IsAllowedConfig(Category, Key);
        if (!Error.isEmpty()) {
            LOG_ERROR_1("S3IUtils", "Configuration update not allowed",
                        Error.c_str());
            Success = false;
        } else {
            // 2. Update the value
            Success = g_Config.Update(Category, Key, Value);

            // 2.1 If the update was a timers update, update the timers
            if (strcmp(Category, "timers") == 0) {
                if (!g_Timers.timerExists(Key)) {
                    LOG_ERROR_1("S3IUtils", "Timer does not exist", Key);
                    Success = false;
                }

                g_Timers.setInterval(Key, String(Value).toInt() * 1000, false);
            }
        }

        // 3. Send the value
        // clang-format off
        String Body = String("{\"sender\": \"") + g_Config.Get("s3i", "id").as<const char*>()
                + "\", \"identifier\": \"" + S3I::CreateMessageIdentifier()
                + "\", \"receivers\": [\"" + doc["replyToEndpoint"].as<const char*>()
                + "\"], \"messageType\": \"setValueReply\", \"replyingToMessage\": \"" + doc["identifier"].as<const char*>()
                + "\", \"ok\": " + String(Success ? "true" : "false") + "}";
        // clang-format on

        S3I::Writers::CharWiFiWriter BodyWriter(Body.c_str());

        g_MessageBroker->Send(
            (String("s3ibs://") + doc["replyToEndpoint"].as<const char*>())
                .c_str(),
            BodyWriter);

        // 4. Restart if requested
        if (Restart && Success) {
            LOG_INFO("S3IUtils", "Restarting ESP32");
            ESP.restart();
        }

        return;
    }

    // 4. Unknown message type
    LOG_ERROR("S3IUtils", "Unhandled message");
    // TODO: Also send a reply with an error message
    return;
}
