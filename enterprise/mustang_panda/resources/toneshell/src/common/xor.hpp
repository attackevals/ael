#pragma once

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
void XorInPlace(unsigned char* input, size_t input_len, unsigned char* key, size_t key_len);
void XorInPlace(unsigned char* input, size_t input_len, unsigned char key);
void XorInPlace(unsigned char* input, size_t input_len, unsigned char* key, size_t key_len, size_t offset);

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
void TripleXorInPlace(unsigned char* input, size_t input_len, unsigned char* key, size_t key_len);

} // namespace
