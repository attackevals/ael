#include "comms_encryption.hpp"
#include "rc4.hpp"

// Encrypt C2 comms using RC4 encryption
void CommsCryptInPlace(unsigned char* input, size_t input_len, unsigned char* key, size_t key_len) {
    RC4CryptInPlace(input, input_len, key, key_len);
}
