#pragma once

#include <stdint.h>
#include <stddef.h>

#ifndef PI_CTR
  #define PI_CTR 1
#endif

#define PI_AES256 1

#define PI_AES_BLOCKLEN 16 // Block length in bytes - AES is 128b block only

#define PI_AES_KEYLEN 32
#define PI_AES_keyExpSize 240

struct PI_AES_ctx
{
  uint8_t RoundKey[PI_AES_keyExpSize];
  uint8_t Iv[PI_AES_BLOCKLEN];
};

void PI_AES_init_ctx(struct PI_AES_ctx* ctx, const uint8_t* key);
void PI_AES_init_ctx_iv(struct PI_AES_ctx* ctx, const uint8_t* key, const uint8_t* iv);
void PI_AES_ctx_set_iv(struct PI_AES_ctx* ctx, const uint8_t* iv);

// Same function for encrypting as for decrypting.
// IV is incremented for every block, and used after encryption as XOR-compliment for output
// Suggesting https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for padding scheme
// NOTES: you need to set IV in ctx with PI_AES_init_ctx_iv() or PI_AES_ctx_set_iv()
//        no IV should ever be reused with the same key
void PI_AES_CTR_xcrypt_buffer(struct PI_AES_ctx* ctx, uint8_t* buf, size_t length);

void PI_AesCtrCrypt(unsigned char* key, unsigned char* iv, unsigned char* buffer, size_t buf_len);
