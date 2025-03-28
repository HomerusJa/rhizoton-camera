//#error "test"
#include "ShellyPlugS.h"

void Shelly::helper_printAPICallSuccess(APICallSuccess& result) {
    Serial.println("Success: " + String(result.success));
    Serial.println("Was on: " + String(result.was_on));
    Serial.println("Payload: " + result.payload);
    Serial.println();
}

Shelly::ShellyPlugS_REST_API::ShellyPlugS_REST_API(const char* ip, uint8_t id)
    : ip(ip), id(id) {}

Shelly::APICallSuccess Shelly::ShellyPlugS_REST_API::getStatus() {
    String url =
        "http://" + String(ip) + "/rpc/Switch.GetStatus?id=" + String(id);
    Serial.println("Getting the Status. url=" + url);
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    String payload = http.getString();

    if (httpCode != 200) {
        Serial.println("Error getting status, HTTP code: " + String(httpCode));
        return {false, false, ""};
    }
    http.end();
    return {true, false, payload};
}

Shelly::APICallSuccess Shelly::ShellyPlugS_REST_API::set(bool state,
                                                         uint8_t toggle_after) {
    String url = "http://" + String(ip) + "/rpc/Switch.Set?id=" + String(id) +
                 "&on=" + (state ? "true" : "false");
    if (toggle_after > 0) {
        url += "&toggle_after=" + String(toggle_after);
    }
    Serial.println("Setting the state. url=" + url);
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    String payload = http.getString();

    if (httpCode != 200) {
        Serial.println("Error setting state, HTTP code: " + String(httpCode));
        return {false, false, ""};
    }

    http.end();
    bool was_on_true = payload == "{\"was_on\":true}";
    bool was_on_false = payload == "{\"was_on\":false}";
    if (!was_on_true && !was_on_false) {
        Serial.println("Error setting state, unexpected response: " + payload);
        return {false, false, payload};
    }
    return {true, was_on_true, payload};
}

Shelly::APICallSuccess Shelly::ShellyPlugS_REST_API::toggle() {
    String url = "http://" + String(ip) + "/rpc/Switch.Toggle?id=" + String(id);
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    String payload = http.getString();

    if (httpCode != 200) {
        Serial.println("Error toggling state, HTTP code: " + String(httpCode));
        return {false, false, ""};
    }

    http.end();
    bool was_on_true = payload == "{\"was_on\":true}";
    bool was_on_false = payload == "{\"was_on\":false}";
    if (!was_on_true && !was_on_false) {
        Serial.println("Error toggling state, unexpected response: " + payload);
        return {false, false, payload};
    }
    return {true, was_on_true, payload};
}
