#pragma once

void base64_decode(const char* input, unsigned char* output, size_t* out_len);

void base64_encode(const unsigned char* input, size_t in_len, char* out, size_t* out_len);
