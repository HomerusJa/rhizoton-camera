#include "Flash.h"

RhizotronCam::Camera::Flash* RhizotronCam::Camera::CreateFlash(
    JsonObject Config) {
    // FIXME: This function causes a memory leak
    if (Config["type"] == "ShellyPlugS") {
        const char* ip = Config["ip"].as<const char*>();
        LOG_INFO_1("flash", "Creating ShellyPlugS flash with IP", ip);
        return new FlashShellyPlugS(new Shelly::ShellyPlugS_REST_API(ip));
    } else if (Config["type"] == "LED") {
        int pin = Config["pin"].as<int>();
        LOG_INFO_1("flash", "Creating LED flash on PIN", String(pin).c_str());
        return new FlashLED(pin);
    } else {
        LOG_INFO("flash", "No flash specified, using FlashOff");
        return new FlashOff();
    }
}
