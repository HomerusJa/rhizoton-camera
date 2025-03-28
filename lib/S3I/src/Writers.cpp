#include "Writers.h"

#include <memory>

using namespace S3I::Writers;

size_t CharWiFiWriter::CharWiFiWriter::Length() const { return strlen(m_Data); }

void CharWiFiWriter::CharWiFiWriter::Write(WiFiClient *client,
                                           size_t chunk_size) {
    client->print(m_Data);
}

size_t B64WiFiWriter::B64WiFiWriter::Length() const {
    File file = m_FS.open(m_Filename, "r");
    if (!file) {
        return 0;
    }

    size_t file_length = file.size();

    return encode_base64_length(file_length);
}

void B64WiFiWriter::Write(WiFiClient *client, size_t chunk_size) {
    // Open the file
    File file = m_FS.open(m_Filename, "r");
    if (!file) {
        Serial.println("Failed to open file.");
        return;
    }

    size_t file_length = file.size();
    size_t b64_length = encode_base64_length(chunk_size);

    // Buffers on the stack
    uint8_t file_buffer[chunk_size];
    uint8_t b64_buffer[b64_length];

    size_t bytes_read = 0;

    Serial.printf("Sending file of %d bytes, encoded length: %d\n", file_length,
                  encode_base64_length(file_length));

    // Loop through the file, read, encode, and send
    while (bytes_read < file_length) {
        // Read a chunk from the file
        size_t bytes_to_read = std::min(chunk_size, file_length - bytes_read);
        size_t bytes_read_now = file.read(file_buffer, bytes_to_read);

        // Base64 encode the chunk
        size_t b64_bytes_written =
            encode_base64(file_buffer, bytes_read_now, b64_buffer);

        // Send the encoded chunk over WiFi
        if (client->write(b64_buffer, b64_bytes_written) != b64_bytes_written) {
            Serial.println("Error: Not all bytes were sent.");
            break;
        }

        bytes_read += bytes_read_now;

        // Log progress
        Serial.printf("Progress: %d/%d bytes sent\n", bytes_read, file_length);

        delay(10);  // Optional delay to prevent flooding
    }

    file.close();  // Close the file after sending
    Serial.println("File transmission completed.");
}

// void B64WiFiWriter::Write(WiFiClient *client, size_t chunk_size) {
//     // Open the file
//     File file = m_FS.open(m_Filename, "r");
//     if (!file) {
//         return;
//     }

//     size_t file_length = file.size();
//     size_t b64_length = encode_base64_length(chunk_size);

//     // Allocate buffers once
//     std::unique_ptr<uint8_t[]> file_buffer(new uint8_t[chunk_size]);
//     std::unique_ptr<uint8_t[]> b64_buffer(new uint8_t[b64_length]);

//     if (!file_buffer || !b64_buffer) {
//         Serial.println("Memory allocation failed");
//         file.close();
//         return;
//     }

//     size_t bytes_read = 0;
//     size_t bytes_written = 0;

//     size_t output_every = 1000;
//     size_t last_output = 0;

//     Serial.printf(
//         "Sending B64-encoded file\nFile length: %d\nEncoded length: %d\n",
//         file_length, encode_base64_length(file_length));

//     while (bytes_read < file_length) {
//         size_t bytes_to_read = std::min(chunk_size, file_length -
//         bytes_read);

//         size_t bytes_read_now = file.read(file_buffer.get(), bytes_to_read);

//         size_t b64_bytes_written =
//             encode_base64(file_buffer.get(), bytes_read_now,
//             b64_buffer.get());

//         Serial.print(".");
//         size_t bytes_sent = client->write(b64_buffer.get(),
//         b64_bytes_written);

//         Serial.printf("Read %d bytes, encoded to %d bytes, sent %d bytes\n",
//                       bytes_read_now, b64_bytes_written, bytes_sent);

//         if (bytes_sent != b64_bytes_written) {
//             Serial.println("Did not write all bytes");
//             Serial.printf("Wrote %d bytes, expected %d bytes\n", bytes_sent,
//                           b64_bytes_written);
//             break;
//         }

//         bytes_read += bytes_read_now;
//         bytes_written += b64_bytes_written;

//         if (bytes_written - last_output > output_every) {
//             Serial.printf("Read %d bytes, wrote %d bytes\n", bytes_read,
//                           bytes_written);
//             last_output = bytes_written;
//         }

//         Serial.printf("Free heap: %d\n", ESP.getFreeHeap());

//         delay(10);  // Maybe this is needed?
//     }

//     // Clean up
//     file.close();
// }

void ComposedWiFiWriter::Write(WiFiClient *client, size_t chunk_size) {
    for (const auto &writer : m_Writers) {
        writer->Write(client, chunk_size);
    }
}

size_t ComposedWiFiWriter::Length() const {
    size_t total_length = 0;
    for (const auto &writer : m_Writers) {
        total_length += writer->Length();
    }

    Serial.printf("Total content length: %d\n", total_length);

    return total_length;
}