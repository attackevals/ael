#include "xor.hpp"

namespace encryption {

/*
 * XorInPlace:
 *      About:
 *          XOR encrypts/decrypts the provided input using the provided key.
 *          The offset parameter will rotate the key by the provided value (specify 0
 *          to use the key as is).
 *      Result:
 *          XOR encrypted/decrypted buffer.
 *      MITRE ATT&CK Techniques:
 *          T1027: Obfuscated Files or Information
 *          T1140: Deobfuscate/Decode Files or Information
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */
void XorInPlace(unsigned char* input, size_t input_len, unsigned char* key, size_t key_len, size_t offset) {
    if (key_len == 0) {
        return;
    }
    offset %= key_len;
    for (size_t i = 0; i < input_len; i++) {
        input[i] = input[i] ^ key[(i + offset) % key_len];
    }
}

// Standard XOR encryption
void XorInPlace(unsigned char* input, size_t input_len, unsigned char* key, size_t key_len) {
    XorInPlace(input, input_len, key, key_len, 0);
}

// XOR encryption with a single-byte key
void XorInPlace(unsigned char* input, size_t input_len, unsigned char key) {
    unsigned char key_array[1] = {key};
    XorInPlace(input, input_len, key_array, 1);
}

/*
 * TripleXorInPlace:
 *      About:
 *          Triple-XOR encrypts/decrypts the provided input using the provided key.
 *          The first round of XOR encryption uses the key as provided.
 *          The second round rotates the key by 1 byte,
 *          and the third round rotates it by 7 bytes.
 *      Result:
 *          XOR encrypted/decrypted buffer.
 *      MITRE ATT&CK Techniques:
 *          T1027: Obfuscated Files or Information
 *          T1140: Deobfuscate/Decode Files or Information
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */
void TripleXorInPlace(unsigned char* input, size_t input_len, unsigned char* key, size_t key_len) {
    // Round 1
    XorInPlace(input, input_len, key, key_len, 0);

    // Round 2
    XorInPlace(input, input_len, key, key_len, 1);

    // Round 3
    XorInPlace(input, input_len, key, key_len, 7);
}

} // namespace
