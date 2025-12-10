#include "base64.hpp"

void base64_encode(const unsigned char* input, size_t in_len, char* out, size_t* out_len) {
    size_t i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    size_t pos = 0;
    char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    while (in_len--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++) {
                out[pos++] = base64_table[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; j < i + 1; j++) {
            out[pos++] = base64_table[char_array_4[j]];
        }

        while (i++ < 3) {
            out[pos++] = '=';
        }
    }

    out[pos] = '\0';
    if (out_len) *out_len = pos;
}

void base64_decode(const char* input, unsigned char* output, size_t* out_len) {
    size_t len = 0;
    unsigned int accum = 0;
    int bits_collected = 0;
    size_t out_pos = 0;

    unsigned char base64_reverse_table[256];
    for (int i = 0; i < 256; ++i)
        base64_reverse_table[i] = 0xFF;

    const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (int i = 0; i < 64; ++i)
        base64_reverse_table[(unsigned char)chars[i]] = i;

    for (const char* p = input; *p; p++) {
        char ch = *p;
        if (ch == '=' || ch == '\0') break;
        if (ch > 127 || base64_reverse_table[(unsigned char)ch] == 0 && ch != 'A') continue;

        accum = (accum << 6) | base64_reverse_table[(unsigned char)ch];
        bits_collected += 6;

        if (bits_collected >= 8) {
            bits_collected -= 8;
            output[out_pos++] = (accum >> bits_collected) & 0xFF;
        }
    }

    if (out_len) *out_len = out_pos;
}
