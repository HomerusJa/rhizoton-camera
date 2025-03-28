#pragma once

#include <Arduino.h>
#include <SD_MMC.h>
#include <esp_camera.h>

#include "Config.h"
#include "Flash.h"
#include "Utils.h"

// AI Thinker ESP32-CAM pin map
#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1  // software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

namespace RhizotronCam {
namespace Camera {

bool Init();
bool Capture(const char* ImagePath, Flash* Flash);

extern int g_ImageNameCounter;

// This function sets the global counter to the next number of an image based on
// the directory contents
void InitializeImageNameCounter();

// Note: This function does not even use the global counter
String GenerateImagePathForNumber(int Number);
// Note: This function does not modify the global counter, it only uses it
String GenerateImagePathWithOffset(int Offset);
// Note: This function modifies the global counter
String GenerateNextImagePath();

/**
 * @brief Analyzes the given image.
 *
 * This function takes a camera frame buffer as input and performs analysis on
 * the image. It prints all the detected markers and their positions to the
 * serial console.
 *
 * @param fb A pointer to the camera frame buffer.
 *
 * @return void
 *
 * @note This function is used for debugging purposes only.
 */
void AnalyzeImage(camera_fb_t* fb);

/**
 * @brief Appends a comment to a JPEG image.
 *
 * This function checks if the image is a JPEG and if the comment is valid
 * before appending the comment to the image. The comment must not be empty,
 * must not exceed the maximum size of 65534 bytes (including the
 * null-terminator), and must be null-terminated. The function also checks if
 * the image is properly terminated before modifying it.
 *
 * @param fb Pointer to the camera_fb_t structure representing the image.
 * @param comment Pointer to the comment string to be appended.
 *
 * @return esp_err_t
 */
esp_err_t AppendCommentJPEG(camera_fb_t* fb, const char* comment);

/**
 * @brief Saves an image to the SD card.
 *
 * This function saves the given image to the SD card.
 *
 * @param fb Pointer to the camera_fb_t structure representing the image.
 * @param path Pointer to the path and name of the image.
 *
 * @return esp_err_t
 */
esp_err_t SaveImgToSD(camera_fb_t* fb, const char* path);

std::vector<std::string> ListAllComments(const char* ImagePath);

uint32_t ExtractTimestampFromImage(const char* ImagePath);

}  // namespace Camera

}  // namespace RhizotronCam
