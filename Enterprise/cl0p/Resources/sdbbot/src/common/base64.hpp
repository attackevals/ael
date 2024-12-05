#ifndef _BASE64_H_
#define _BASE64_H_

#include <vector>
#include <string>
typedef unsigned char BYTE;

// Base64 encode for BYTE buffer
std::string base64_encode(BYTE const* buf, unsigned int bufLen);

// Base64 to vector BYTE
std::vector<BYTE> base64_decode(std::string const&);

// Base64 encode string
std::string base64_encode_string(const std::string data);

// Base64 decode string
std::string base64_decode_string(std::string const& encoded_string);



// Added for/Used by the XORLogger
namespace common {
    const static unsigned char base64_table[65] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // Base64 encodes the given data and returns a std::string representing the base64-encoded output
    inline std::string Base64Encode(std::vector<unsigned char> buf) {
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        for (auto bytes : buf) {
            char_array_3[i++] = bytes;
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; (i < 4); i++)
                    ret += base64_table[char_array_4[i]];
                i = 0;
            }
        }
        if (i)
        {
            for (j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; (j < i + 1); j++)
                ret += base64_table[char_array_4[j]];

            while ((i++ < 3))
                ret += '=';
        }
        return ret;

    }
}

#endif