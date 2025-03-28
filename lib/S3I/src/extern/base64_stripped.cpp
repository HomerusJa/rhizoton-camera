#include "base64_stripped.hpp"

unsigned char binary_to_base64(unsigned char v) {
    // Capital letters - 'A' is ascii 65 and base64 0
    if (v < 26) return v + 'A';

    // Lowercase letters - 'a' is ascii 97 and base64 26
    if (v < 52) return v + 71;

    // Digits - '0' is ascii 48 and base64 52
    if (v < 62) return v - 4;

#ifdef BASE64_URL
    // '-' is ascii 45 and base64 62
    if (v == 62) return '-';
#else
    // '+' is ascii 43 and base64 62
    if (v == 62) return '+';
#endif

#ifdef BASE64_URL
    // '_' is ascii 95 and base64 62
    if (v == 63) return '_';
#else
    // '/' is ascii 47 and base64 63
    if (v == 63) return '/';
#endif

    return 64;
}

unsigned int encode_base64_length(unsigned int input_length) { return (input_length + 2) / 3 * 4; }

unsigned int encode_base64(const unsigned char input[], unsigned int input_length,
                           unsigned char output[]) {
    unsigned int full_sets = input_length / 3;

    // While there are still full sets of 24 bits...
    for (unsigned int i = 0; i < full_sets; ++i) {
        output[0] = binary_to_base64(input[0] >> 2);
        output[1] = binary_to_base64((input[0] & 0x03) << 4 | input[1] >> 4);
        output[2] = binary_to_base64((input[1] & 0x0F) << 2 | input[2] >> 6);
        output[3] = binary_to_base64(input[2] & 0x3F);

        input += 3;
        output += 4;
    }

    switch (input_length % 3) {
        case 0:
            output[0] = '\0';
            break;
        case 1:
            output[0] = binary_to_base64(input[0] >> 2);
            output[1] = binary_to_base64((input[0] & 0x03) << 4);
            output[2] = '=';
            output[3] = '=';
            output[4] = '\0';
            break;
        case 2:
            output[0] = binary_to_base64(input[0] >> 2);
            output[1] = binary_to_base64((input[0] & 0x03) << 4 | input[1] >> 4);
            output[2] = binary_to_base64((input[1] & 0x0F) << 2);
            output[3] = '=';
            output[4] = '\0';
            break;
    }

    return encode_base64_length(input_length);
}
