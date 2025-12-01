#include "comms_encryption.hpp"
#include "xor.hpp"

// Encrypt C2 comms using XOR encryption
void CommsCryptInPlace(unsigned char* input, size_t input_len, unsigned char* key, size_t key_len) {
    encryption::XorInPlace(input, input_len, key, key_len);
}
