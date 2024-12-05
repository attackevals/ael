#include "crypto/xor.hpp"
#include "crypto/xor_obfuscation.hpp"
#include <stdexcept>

namespace encryption {

std::vector<unsigned char> XorKeyBytes = GetXorKeyBytes(RESOURCE_XOR_KEY_HEX_STR);

/*
 * GetXorKeyBytes:
 *      About:
 *          Parses the given hex string into a byte vector
 *      Result:
 *          XOR key vector from the given hex string
 */
std::vector<unsigned char> GetXorKeyBytes(const std::string& hex_str) {
    std::vector<unsigned char> key_bytes;
    size_t str_len = hex_str.size();
    for (size_t i = 0; i < str_len; i += 2) {
        std::string byte_str = hex_str.substr(i, 2);
        unsigned char byte_val = static_cast<unsigned char>(std::stoi(byte_str, nullptr, 16));
        key_bytes.push_back(byte_val);
    }
    return key_bytes;
}

/*
 * XorInPlace:
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
void XorInPlace(unsigned char* input, size_t input_len, unsigned char* key, size_t key_len) {
    if (key_len == 0) {
        throw std::invalid_argument(XOR_LIT("Cannot perform XOR operations with a 0-length key."));
    }
    for (size_t i = 0; i < input_len; i++) {
        input[i] = input[i] ^ key[i % key_len];
    }
}

} // namespace