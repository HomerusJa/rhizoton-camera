/* binary_to_base64:
 *   Description:
 *     Converts a single byte from a binary value to the corresponding base64 character
 *   Parameters:
 *     v - Byte to convert
 *   Returns:
 *     ascii code of base64 character. If byte is >= 64, then there is not corresponding base64
 * character and 255 is returned
 */
unsigned char binary_to_base64(unsigned char v);

/* encode_base64_length:
 *   Description:
 *     Calculates length of base64 string needed for a given number of binary bytes
 *   Parameters:
 *     input_length - Amount of binary data in bytes
 *   Returns:
 *     Number of base64 characters needed to encode input_length bytes of binary data
 */
unsigned int encode_base64_length(unsigned int input_length);

/* encode_base64:
 *   Description:
 *     Converts an array of bytes to a base64 null-terminated string
 *   Parameters:
 *     input - Pointer to input data
 *     input_length - Number of bytes to read from input pointer
 *     output - Pointer to output string. Null terminator will be added automatically
 *   Returns:
 *     Length of encoded string in bytes (not including null terminator)
 */
unsigned int encode_base64(const unsigned char input[], unsigned int input_length,
                           unsigned char output[]);


