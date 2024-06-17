#include <iostream>
#include <string>
#include "Utilities.h"
#include "XorString.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/secblock.h"
#include "cryptopp/osrng.h"
#include "cryptopp/files.h"
#include "cryptopp/hex.h"
#include "cryptopp/base64.h"
#include "cryptopp/modes.h"
#include "cryptopp/rsa.h"
#include "base64.h"

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "cryptopp/arc4.h"



CryptoPP::SecByteBlock GenerateRC4Key();

std::string RC4EncryptString(std::string plain, CryptoPP::SecByteBlock key);

std::string RC4DecryptString(std::string cipher, CryptoPP::SecByteBlock key);

std::string RC4KeyToString(CryptoPP::SecByteBlock key);

std::vector<char> RSAEncrypt(std::string packet_data);