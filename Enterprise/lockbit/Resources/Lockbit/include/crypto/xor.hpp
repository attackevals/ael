#pragma once

#include <string>
#include <vector>

#ifdef RESOURCE_XOR_KEY_STR
#define RESOURCE_XOR_KEY_HEX_STR std::string(RESOURCE_XOR_KEY_STR)
#else 
#error "RESOURCE_XOR_KEY_STR must be set at compile-time"
#endif

namespace encryption {

// defined in src/common/crypto/xor.cpp
extern std::vector<unsigned char> XorKeyBytes;

/*
 * GetXorKeyBytes (defined in src/common/crypto/xor.cpp):
 *      About:
 *          Parses the given hex string into a byte vector
 *      Result:
 *          XOR key vector from the given hex string
 */
std::vector<unsigned char> GetXorKeyBytes(const std::string& hex_str);

/*
 * XorInPlace (defined in src/common/crypto/xor.cpp):
 *      About:
 *          XOR encrypts/decrypts the provided input using the provided key.
 *      Result:
 *          XOR encrypted/decrypted buffer.
 *      MITRE ATT&CK Techniques:
 *          T1027: Obfuscated Files or Information
 *          T1140: Deobfuscate/Decode Files or Information
 *      CTI:
 *          https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html
 */
void XorInPlace(unsigned char* input, size_t input_len, unsigned char* key, size_t key_len);

} // namespace