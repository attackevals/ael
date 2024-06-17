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

#endif