#include "xor.hpp"

namespace xor_crypt {

std::vector<unsigned char> XorKeyBytes = GetXorKeyBytes(RESOURCE_XOR_KEY_STR);

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
 */
void XorInPlace(unsigned char* input, size_t input_len, const std::vector<unsigned char>& key) {
    if (key.size() == 0) {
        return;
    }
    for (size_t i = 0; i < input_len; i++) {
        input[i] = input[i] ^ key[i % key.size()];
    }
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
 */
void XorInPlace(std::byte* input, size_t input_len, const std::vector<unsigned char>& key) {
    if (key.size() == 0) {
        return;
    }
    for (size_t i = 0; i < input_len; i++) {
        input[i] = input[i] ^ (std::byte)(key[i % key.size()]);
    }
}

} // namespace