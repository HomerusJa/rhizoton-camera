#include "Camera.h"

#include <TimeTools.h>

#include "S3IUtils.h"

#define CAPTURE_RETRY_COUNT 5

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    .xclk_freq_hz = 20000000,  // EXPERIMENTAL: Set to 16MHz on ESP32-S2 or
                               // ESP32-S3 to enable EDMA mode
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,  // YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size =
        FRAMESIZE_UXGA,  // QQVGA-UXGA, For ESP32, do not use sizes above QVGA
                         // when not JPEG. The performance of the ESP32-S series
                         // has improved a lot, but JPEG mode always gives
                         // better frame rates.

    .jpeg_quality = 12,  // 0-63, for OV series camera sensors, lower number
                         // means higher quality
    .fb_count = 1,  // When jpeg mode is used, if fb_count more than one, the
                    // driver will work in continuous mode.
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY  // CAMERA_GRAB_LATEST. Sets when
                                         // buffers should be filled
};

bool RhizotronCam::Camera::Init() {
    LOG_INFO("Camera", "Initializing Camera");
    if (CAM_PIN_PWDN != -1) {
        pinMode(CAM_PIN_PWDN, OUTPUT);
        digitalWrite(CAM_PIN_PWDN, LOW);
    }

    LOG_DEBUG("Camera", "Camera Power Up Sequence succeeded");

    // initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        LOG_ERROR_1("Camera", "Camera Init Failed with error",
                    esp_err_to_name(err));
        return false;
    }

    LOG_INFO("Camera", "Camera Init Succeeded");
    return true;
}

bool RhizotronCam::Camera::Capture(const char* ImagePath, Flash* Flash) {
    LOG_INFO("Camera", "Capturing Image");

    // Unix timestamp in seconds
    uint32_t Timestamp = TimeTools::GetTimestamp();

    if (!Flash->TurnOn()) {
        LOG_WARN("Camera", "Failed to turn on flash");
    } else {
        LOG_DEBUG("Camera", "Flash turned on successfully");
    }
    delay(1000);  // Delay for 1 second to let the flash turn on

    LOG_INFO("Camera", "Acquiring Camera Frame Buffer");
    camera_fb_t* fb = nullptr;
    int retries = CAPTURE_RETRY_COUNT;
    while (retries > 0) {
        fb = esp_camera_fb_get();
        if (fb) {
            break;
        }
        LOG_WARN_1("Camera", "Camera Capture Failed. Retries left",
                   String(retries).c_str());
        delay(1000);  // Delay for 1 second before retrying
        retries--;
    }

    if (!Flash->TurnOff()) {
        LOG_WARN("Camera", "Failed to turn off flash");
    } else {
        LOG_DEBUG("Camera", "Flash turned off successfully");
    }

    if (!fb) {
        LOG_ERROR("Camera", "Camera Capture Failed");
        RhizotronCam::S3IUtils::SendErrorAsEvent(
            "Camera Capture Failed",
            ("number of retries: " + String(CAPTURE_RETRY_COUNT - retries))
                .c_str(),
            "Camera");
        RhizotronCam::Utils::Restart();
        return false;  // Never reached
    } else {
        LOG_DEBUG("Camera", "Camera Capture Succeeded");
    }

    // Append a comment with the timestamp to the image
    const char* Comment = (String("Timestamp: ") + Timestamp).c_str();
    if (AppendCommentJPEG(fb, Comment) != ESP_OK) {
        LOG_ERROR("Camera", "Failed to append comment to image");
    } else {
        LOG_DEBUG("Camera", "Comment appended to image successfully");
    }

    // Save the image to the SD card
    if (SaveImgToSD(fb, ImagePath) != ESP_OK) {
        LOG_ERROR("Camera", "Failed to save image to SD card");
    } else {
        LOG_DEBUG("Camera", "Image saved to SD card successfully");
    }

    // Analyze the image if needed
    // AnalyzeImage(fb);

    return true;
}

int RhizotronCam::Camera::g_ImageNameCounter = 0;

void RhizotronCam::Camera::InitializeImageNameCounter() {
    g_ImageNameCounter = 0;
    File root = SD_MMC.open("/image");
    while (true) {
        File entry = root.openNextFile();
        if (!entry) {
            break;
        }
        if (entry.isDirectory()) {
            continue;
        }

        String FileName = entry.name();
        int Number = FileName.substring(0, FileName.length() - 4).toInt();
        if (Number > g_ImageNameCounter) {
            g_ImageNameCounter = Number;
        }
        if (Number < 0) LOG_ERROR("Camera", "Image number is negative");
    }

    LOG_INFO_1("Camera", "Initialized Image Name Counter",
               String(g_ImageNameCounter).c_str());
}

String RhizotronCam::Camera::GenerateImagePathForNumber(int Number) {
    if (Number < 0) {
        LOG_ERROR("Camera", "Image number is negative");
        return String();
    }

    String ImagePath = String("/image/") + Number + ".jpg";

    LOG_INFO_1("Camera", "Generated Image Path", ImagePath.c_str());
    return ImagePath;
}

String RhizotronCam::Camera::GenerateImagePathWithOffset(int Offset) {
    return GenerateImagePathForNumber(g_ImageNameCounter + Offset);
}

String RhizotronCam::Camera::GenerateNextImagePath() {
    String ImagePath = GenerateImagePathForNumber(g_ImageNameCounter);
    g_ImageNameCounter++;
    while (SD_MMC.exists(ImagePath)) {
        g_ImageNameCounter++;
        ImagePath = GenerateImagePathForNumber(g_ImageNameCounter);
    }
    return ImagePath;
}

void RhizotronCam::Camera::AnalyzeImage(camera_fb_t* fb) {
    // Output basic information about the image
    Serial.printf("Picture taken! Its size was: %zu bytes\n", fb->len);
    Serial.printf("Image resolution: %dx%d\n", fb->width, fb->height);
    Serial.printf("Image format: %d\n\n", fb->format);

    // Check some stuff about the image format
    if (fb->format != PIXFORMAT_JPEG) {
        Serial.println("This example only works with JPEG images");
        return;
    }

    // Search for the JPEG markers
    static uint8_t JPEG_MARKERS[][2] = {
        {0xFF, 0xD8},  // SOI: Start Of Image
        {0xFF, 0xD9},  // EOI: End Of Image
        {0xFF, 0xE0},  // APP0: JFIF (JPEG File Interchange Format)
        {0xFF, 0xE1},  // APP1: EXIF (Exchangeable Image File Format)
        {0xFF, 0xE2},  // APP2: FlashPix
        {0xFF, 0xE3},  // APP3: Kodak
        {0xFF, 0xE4},  // APP4: Reserved
        {0xFF, 0xE5},  // APP5: Reserved
        {0xFF, 0xE6},  // APP6: Reserved
        {0xFF, 0xE7},  // APP7: Reserved
        {0xFF, 0xE8},  // APP8: Application-specific
        {0xFF, 0xE9},  // APP9: Application-specific
        {0xFF, 0xEA},  // APP10: Application-specific
        {0xFF, 0xEB},  // APP11: Application-specific
        {0xFF, 0xEC},  // APP12: Application-specific
        {0xFF, 0xED},  // APP13: Application-specific
        {0xFF, 0xEE},  // APP14: Application-specific
        {0xFF, 0xEF},  // APP15: Application-specific
        {0xFF, 0xFE},  // COM: Comment
        {0xFF, 0xC0},  // SOF0: Start Of Frame (Baseline DCT)
        {0xFF, 0xC1},  // SOF1: Start Of Frame (Extended Sequential DCT)
        {0xFF, 0xC2},  // SOF2: Start Of Frame (Progressive DCT)
        {0xFF, 0xC4},  // DHT: Define Huffman Table(s)
        {0xFF, 0xDB},  // DQT: Define Quantization Table(s)
        {0xFF, 0xDA},  // SOS: Start Of Scan
        {0xFF, 0xD0},  // RST0: Restart marker 0
        {0xFF, 0xD1},  // RST1: Restart marker 1
        {0xFF, 0xD2},  // RST2: Restart marker 2
        {0xFF, 0xD3},  // RST3: Restart marker 3
        {0xFF, 0xD4},  // RST4: Restart marker 4
        {0xFF, 0xD5},  // RST5: Restart marker 5
        {0xFF, 0xD6},  // RST6: Restart marker 6
        {0xFF, 0xD7},  // RST7: Restart marker 7
        {0xFF, 0xDD}   // DRI: Define Restart Interval
    };

    for (int i = 0; i < fb->len - 1; i++) {
        for (int j = 0; j < 4; j++) {
            if (fb->buf[i] == JPEG_MARKERS[j][0] &&
                fb->buf[i + 1] == JPEG_MARKERS[j][1]) {
                Serial.printf("Found JPEG Marker: %02X %02X at position %d\n",
                              JPEG_MARKERS[j][0], JPEG_MARKERS[j][1], i);
            }
        }
    }
}

esp_err_t RhizotronCam::Camera::AppendCommentJPEG(camera_fb_t* fb,
                                                  const char* comment) {
    // All kinds of error checking
    // Check if the image is a JPEG
    if (fb->format != PIXFORMAT_JPEG) {
        Serial.println("This example only works with JPEG images");
        return ESP_FAIL;
    }

    // Check if the comment is not empty
    if (strlen(comment) == 0) {
        Serial.println("The comment cannot be empty");
        return ESP_ERR_INVALID_ARG;
    }

    // Check if the comment is not too long
    // The maximum size of a comment is 65533 bytes (excluding the
    // null-terminator) because the length field is 2 bytes
    if (strlen(comment) > 65533) {
        Serial.println("The comment is too long");
        return ESP_ERR_INVALID_ARG;
    }

    // Check if the comment is null-terminated
    if (comment[strlen(comment)] != '\0') {
        Serial.println("The comment must be null-terminated");
        return ESP_ERR_INVALID_ARG;
    }

    // Check if the last marker is EOI
    if (fb->buf[fb->len - 2] != 0xFF || fb->buf[fb->len - 1] != 0xD9) {
        Serial.println("The image is not properly terminated");
        return ESP_ERR_INVALID_ARG;
    }

    // Change the EOI marker to a COM marker
    fb->buf[fb->len - 2] = 0xFF;
    fb->buf[fb->len - 1] = 0xFE;

    // Calculate the size of the comment
    // 2 bytes for the length field and include the null-terminator (1 byte)
    size_t comment_size = strlen(comment) + 3;

    // Write the comment size
    fb->buf[fb->len++] = (comment_size >> 8) & 0xFF;
    fb->buf[fb->len++] = comment_size & 0xFF;

    // Write the comment
    for (size_t i = 0; i < comment_size; i++) {
        fb->buf[fb->len++] = comment[i];
    }

    // Write the EOI marker
    fb->buf[fb->len++] = 0xFF;
    fb->buf[fb->len++] = 0xD9;

    return ESP_OK;
}

esp_err_t RhizotronCam::Camera::SaveImgToSD(camera_fb_t* fb, const char* path) {
    // Delete the file if it already exists
    if (SD_MMC.exists(path)) SD_MMC.remove(path);

    // Open the file for writing
    File file = SD_MMC.open(path, FILE_WRITE);

    if (!file) {
        Serial.printf("Failed to open file %s for writing\n", path);
        return ESP_FAIL;
    }

    // Write the image to the file
    file.write(fb->buf, fb->len);

    // Close the file
    file.close();

    return ESP_OK;
}

std::vector<std::string> RhizotronCam::Camera::ListAllComments(
    const char* ImagePath) {
    std::vector<std::string> Comments;
    uint8_t Buffer[2];
    static const uint8_t CommentMarker[2] = {0xFF, 0xFE};

    // Open the file for reading
    File file = SD_MMC.open(ImagePath, FILE_READ);

    if (!file) {
        LOG_ERROR_1("Camera", "Failed to open file", ImagePath);
        return Comments;
    }

    // Read the file byte by byte
    file.read(Buffer, 2);

    while (file.available()) {
        // Check if the current bytes are the comment marker
        if (Buffer[0] == CommentMarker[0] && Buffer[1] == CommentMarker[1]) {
            // Read the length of the comment
            uint16_t Length;
            file.read(reinterpret_cast<uint8_t*>(&Length), 2);

            // Read the comment
            char Comment[Length];
            file.read(reinterpret_cast<uint8_t*>(Comment), Length);

            // Add the comment to the vector
            Comments.push_back(std::string(Comment));
        }

        // Move the second byte to the first byte
        Buffer[0] = Buffer[1];

        // Read the next byte
        file.read(Buffer + 1, 1);
    }

    return Comments;
}

uint32_t RhizotronCam::Camera::ExtractTimestampFromImage(
    const char* ImagePath) {
    std::vector<std::string> Comments = ListAllComments(ImagePath);

    for (const std::string& Comment : Comments) {
        if (Comment.find("Timestamp: ") != std::string::npos) {
            LOG_INFO_1("Camera", "Extracted timestamp from image",
                       Comment.c_str());
            return std::stoul(Comment.substr(11));
        } else {
            LOG_DEBUG_1("Camera", "Found comment", Comment.c_str());
        }
    }

    LOG_ERROR("Camera", "Failed to extract timestamp from image");
    return 0;
}
