#pragma once

#include <ArduinoJson.h>

#include "ShellyPlugS.h"
#include "Utils.h"

namespace RhizotronCam {
namespace Camera {

class Flash {
   public:
    virtual bool TurnOn() = 0;
    virtual bool TurnOff() = 0;
    virtual bool Switch() = 0;
};

class FlashShellyPlugS : public Flash {
   public:
    FlashShellyPlugS(Shelly::ShellyPlugS_REST_API* ShellyPlugS)
        : m_ShellyPlugS(ShellyPlugS) {}

    bool TurnOn() override { return m_ShellyPlugS->set(true).success; }
    bool TurnOff() override { return m_ShellyPlugS->set(false).success; }
    bool Switch() override { return m_ShellyPlugS->toggle().success; }

   private:
    Shelly::ShellyPlugS_REST_API* m_ShellyPlugS;
};

class FlashLED : public Flash {
   public:
    FlashLED(int Pin) : m_Pin(Pin) {
        pinMode(m_Pin, OUTPUT);
        digitalWrite(m_Pin, LOW);
    }

    bool TurnOn() override {
        digitalWrite(m_Pin, HIGH);
        return true;
    }

    bool TurnOff() override {
        digitalWrite(m_Pin, LOW);
        return true;
    }

    bool Switch() override {
        digitalWrite(m_Pin, !digitalRead(m_Pin));
        return true;
    }

   private:
    int m_Pin;
};

class FlashOff : public Flash {
   public:
    bool TurnOn() override { return true; }
    bool TurnOff() override { return true; }
    bool Switch() override { return true; }
};

Flash* CreateFlash(JsonObject Config);

}  // namespace Camera
}  // namespace RhizotronCam
