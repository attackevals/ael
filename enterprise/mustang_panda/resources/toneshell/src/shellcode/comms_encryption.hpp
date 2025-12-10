#pragma once

// This will be satisfied by different source files depending on
// which shellcode is being built
void CommsCryptInPlace(unsigned char* input, size_t input_len, unsigned char* key, size_t key_len);
