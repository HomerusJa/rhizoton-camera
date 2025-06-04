#include <Arduino.h>
#include <CustomLogging.h>
#include <HTTPClient.h>
#include <S3I.h>
#include <ShellyPlugS.h>
#include <TimeTools.h>
#include <Timers.h>

#include <memory>

#include "Camera.h"
#include "Config.h"
#include "S3IUtils.h"
#include "Utils.h"

Timers::Timers g_Timers(SD_MMC, "/timer_state.json", false);

S3I::Authenticator* g_Authenticator;
S3I::MessageBroker* g_MessageBroker;

// Create a placeholder for the flash functions
RhizotronCam::Camera::Flash* g_Flash = new RhizotronCam::Camera::FlashOff();

#define BLINK_PIN 4

void setup() {
    pinMode(BLINK_PIN, OUTPUT);
    RhizotronCam::Utils::morse_blink(BLINK_PIN, "...");

    Serial.begin(115200);
    CustomLogging::SerialLog::InitSerialLogger(true);

    RhizotronCam::Utils::InitSD();

    RhizotronCam::Utils::morse_blink(BLINK_PIN, "---");

    g_Config.Load();

    if (!RhizotronCam::Utils::ConnectToWiFi()) RhizotronCam::Utils::Restart();
    RhizotronCam::Utils::morse_blink(BLINK_PIN, ".");
    bool AllowUDP = g_Config.Get("logging", "allow_udp").as<bool>();

    TimeTools::InitTime();

    SD_MMC.mkdir("/image");
    RhizotronCam::Camera::InitializeImageNameCounter();
    if (!RhizotronCam::Camera::Init()) {
        LOG_ERROR("Setup", "Failed to initialize camera");
        RhizotronCam::Utils::morse_blink(BLINK_PIN, "... --- ...");
        RhizotronCam::Utils::Restart();
    }

    // Setup all the timers
    // g_Timers.load();
    // FIXME: This introduces a bug where updated times are not recognized
    g_Timers.addTimer("Capture",
                      g_Config.Get("timers", "capture").as<uint32_t>() * 1000,
                      true);
    g_Timers.addTimer(
        "Fetch", g_Config.Get("timers", "fetch_interval").as<uint32_t>() * 1000,
        true);
    // Dont want to start this one immediately, because of obvious reasons
    g_Timers.addTimer(
        "Restart",
        g_Config.Get("timers", "restart_interval").as<uint32_t>() * 1000);

    // Initialize the S3I classes
    S3I::IDPServer IDPServer = {
        g_Config.Get("s3i", "idp_server").as<const char*>(),
        g_Config.Get("s3i", "idp_path").as<const char*>()};
    S3I::Credentials Credentials = {
        g_Config.Get("s3i", "id").as<const char*>(),
        g_Config.Get("s3i", "secret").as<const char*>()};

    g_Authenticator = new S3I::Authenticator(IDPServer, Credentials);

    g_MessageBroker = new S3I::MessageBroker(
        g_Config.Get("s3i", "broker_server").as<const char*>(),
        g_Config.Get("s3i", "endpoint").as<const char*>(), g_Authenticator);

    // Create the flash object
    JsonObject FlashConfig = g_Config.Get("flash");
    DEBUG_PRINTLN("Got flash config");
    g_Flash = RhizotronCam::Camera::CreateFlash(FlashConfig);

    LOG_INFO("Setup", "Setup complete");

    RhizotronCam::Utils::morse_blink(BLINK_PIN, "--- ---");

    // Send the status to the S3I broker
    RhizotronCam::S3IUtils::SendStatusAsEvent("startup");
}

void loop() {
    DEBUG_PRINT(".");

    bool IsRestartTime = g_Timers.isTime("Restart");
    bool IsCaptureTime = g_Timers.isTime("Capture");
    bool IsFetchTime = g_Timers.isTime("Fetch");

    // Read any commands from Serial
    RhizotronCam::Utils::ReadSerialCommands(IsRestartTime, IsCaptureTime,
                                            IsFetchTime, g_Timers);

    if (IsRestartTime) {
        RhizotronCam::S3IUtils::SendStatusAsEvent("scheduled-restart");
        RhizotronCam::Utils::Restart();
    }
    if (IsCaptureTime | IsRestartTime) RhizotronCam::Utils::ConnectToWiFi();

    if (IsCaptureTime) {
        LOG_INFO("Main", "Capture timer triggered");
        RhizotronCam::S3IUtils::SendStatusAsEvent("capture");

        String imagePath = RhizotronCam::Camera::GenerateNextImagePath();
        if (!RhizotronCam::Camera::Capture(imagePath.c_str(), g_Flash)) {
            LOG_ERROR("Main", "Failed to capture image");
        } else {
            LOG_INFO_1("Main", "Image captured and saved at PATH",
                       imagePath.c_str());
            // Send the image to the S3I broker
            RhizotronCam::S3IUtils::SendImageAsEvent(imagePath.c_str());
        }
    }

    if (IsFetchTime) {
        LOG_INFO("Main", "Fetch timer triggered");
        RhizotronCam::S3IUtils::SendStatusAsEvent("fetch");
        RhizotronCam::S3IUtils::FetchAndProcessMessages();
    }

    delay(1000);
}
