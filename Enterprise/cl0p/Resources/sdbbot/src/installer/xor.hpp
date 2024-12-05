#pragma once
#include <span>
#include <string>
#include <vector>

namespace xor_crypt {

extern std::vector<unsigned char> XorKeyBytes;

/*
 * GetXorKeyBytes:
 *      About:
 *          Parses the given hex string into a byte vector
 *      Result:
 *          XOR key vector from the given hex string
 */
std::vector<unsigned char> GetXorKeyBytes(const std::string& hex_str);

/*
 * XorInPlace:
 *      About:
 *          XOR encrypts/decrypts the provided input using the provided key.
 *      Result:
 *          XOR encrypted/decrypted buffer.
 *      MITRE ATT&CK Techniques:
 *          T1027: Obfuscated Files or Information
 *          T1140: Deobfuscate/Decode Files or Information
 */
void XorInPlace(unsigned char* input, size_t input_len, const std::vector<unsigned char>& key);
void XorInPlace(std::byte* input, size_t input_len, const std::vector<unsigned char>& key);

} // namespace