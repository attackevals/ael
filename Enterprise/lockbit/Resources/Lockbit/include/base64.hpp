/**
 * Standalone C++ implementation of base64 encoding, based on code from 
 * https://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.c
 * 
 * Reference:
 *      https://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.c
 *      License and copyright info: https://web.mit.edu/freebsd/head/contrib/wpa/
*/

#ifndef BASE64_ENCODER_H_
#define BASE64_ENCODER_H_

#include <string>
#include <vector>

namespace Base64Encoder {
    const static unsigned char base64_table[65] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // Base64 encodes the given data and returns a std::string representing the base64-encoded output
    inline std::string Base64Encode(const unsigned char* plaintext, size_t plaintext_len) {
        char* pos;
        const unsigned char *end, *in;
        size_t encoded_len;
        if (plaintext_len == 0) {
            return std::string("");
        }
        encoded_len = (plaintext_len / 3) * 4;
        if (plaintext_len % 3 != 0) {
            encoded_len += 4; // include padding at end if needed
        }
        encoded_len += 1; // null terminator
        std::vector<char> encoded;
        encoded.resize(encoded_len);

        end = plaintext + plaintext_len;
        in = plaintext;
        pos = &encoded[0];
        while (end - in >= 3) {
            *pos++ = base64_table[in[0] >> 2];
            *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
            *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
            *pos++ = base64_table[in[2] & 0x3f];
            in += 3;
        }

        if (end - in) {
            *pos++ = base64_table[in[0] >> 2];
            if (end - in == 1) {
                *pos++ = base64_table[(in[0] & 0x03) << 4];
                *pos++ = '=';
            } else {
                *pos++ = base64_table[((in[0] & 0x03) << 4) |
                            (in[1] >> 4)];
                *pos++ = base64_table[(in[1] & 0x0f) << 2];
            }
            *pos++ = '=';
        }
        encoded[encoded_len - 1] = '\0';
        std::string encoded_str(&encoded[0]);
        return encoded_str;
    }
}

#endif