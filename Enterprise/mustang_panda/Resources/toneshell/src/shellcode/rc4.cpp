// Reference: https://en.wikipedia.org/wiki/RC4

#include "rc4.hpp"
#include "xor.hpp"

#define STATE_LEN 256

// swap values between 2 pointers
void swap(unsigned char* x, unsigned char* y) {
    unsigned char tmp = *x;
    *x = *y;
    *y = tmp;
}

// Key-scheduling algorithm for RC4
void KSA(unsigned char* key, size_t key_len, unsigned char* state) {
    // Initialize state array
    for (int i = 0; i < STATE_LEN; i++) {
        state[i] = i;
    }

    // Perform permutations
    int j = 0;
    for (int i = 0; i < STATE_LEN; i++) {
        j = (j + state[i] + key[i % key_len]) % STATE_LEN;
        swap(&(state[i]), &(state[j]));
    }
}

// Pseudo-random generation algorithm to generate keystream and XOR input
void PRGA(unsigned char* input, size_t input_len, unsigned char* state) {
    int i = 0;
    int j = 0;
    unsigned char k = 0;
    for (int input_i = 0; input_i < input_len; input_i++) {
        i = (i + 1) % 256;
        j = (j + state[i]) % 256;
        swap(&(state[i]), &(state[j]));
        k = state[(state[i] + state[j]) % 256];
        input[input_i] ^= k;
    }
}

/*
 * RC4CryptInPlace:
 *      About:
 *          Perform in-place RC4 encryption/decryption using the provided input and key.
 *          The same function is used for both encryption and decryption.
 *      Result:
 *          Encrypted/decrypted input buffer.
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */
void RC4CryptInPlace(unsigned char* input, size_t input_len, unsigned char* key, size_t key_len) {
    unsigned char state[STATE_LEN];
    KSA(key, key_len, state);
    PRGA(input, input_len, state);
}
