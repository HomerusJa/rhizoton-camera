#pragma once

#include <HTTPClient.h>

namespace Shelly {
struct APICallSuccess {
    bool success; /**< True if the API call was successful, false otherwise. */
    bool was_on;  /**< True if the switch was on before the API call, false
                     otherwise. Only used  sometimes! */
    String payload; /**< The payload of the API call. */
};

/**
 * @brief Helper function to print the result of an API call.
 *
 * @param result The result of the API call.
 */
void helper_printAPICallSuccess(APICallSuccess& result);

/**
 * @brief The ShellyPlugS_REST_API class provides an interface for controlling a
 * Shelly Plug S device over its REST API.
 */
class ShellyPlugS_REST_API {
   public:
    /**
     * @brief Constructs a ShellyPlugS_REST_API object with the specified IP
     * address and ID which interacts with the device over its REST API.
     *
     * @param ip The IP address of the Shelly Plug S device.
     * @param id The ID of the Shelly Plug S device (optional, default is 0).
     */
    ShellyPlugS_REST_API(const char* ip, uint8_t id = 0);

    /**
     * @brief Gets the status of the Shelly Plug S device.
     *
     * @return The status of the Shelly Plug S device, packed in a
     * APICallSuccess struct. It doesnt use the was_on field.
     */
    APICallSuccess getStatus();

    /**
     * @brief Sets the state of the Shelly Plug S device.
     *
     * @param state The desired state (true for ON, false for OFF).
     * @param toggle_after Flip-back timer in seconds (optional, default is 0).
     *
     * @return True if the switch was on before the method was executed, false
     * otherwise.
     */
    APICallSuccess set(bool state, uint8_t toggle_after = 0);

    /**
     * @brief Toggles the state of the Shelly Plug S device.
     *
     * @return True if the switch was on before the method was executed, false
     * otherwise.
     */
    APICallSuccess toggle();

   private:
    const char* ip; /**< The IP address of the Shelly Plug S device. */
    uint8_t id;     /**< The ID of the Shelly Plug S device. */
};
}  // namespace Shelly