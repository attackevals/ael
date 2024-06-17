#ifndef __ENCRYPTION_DEF__
#define __ENCRYPTION_DEF__

#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>

#include "XorString.h"

#include "cryptopp\osrng.h"
#include "cryptopp\aes.h"
#include "cryptopp\des.h"
#include "cryptopp\filters.h"
#include "cryptopp\modes.h"
#include "cryptopp\hex.h"
#include "cryptopp\cryptlib.h"

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "cryptopp/arc4.h"


// Set up default values that are set in the CMake file

// Size of encrypted data
#ifndef DATA_SIZE
#define DEFAULT_DATA_SIZE 32
#else
#define DEFAULT_DATA_SIZE (DATA_SIZE)
#endif

// RC4 Key
#ifndef RC4_KEY
#define DEFAULT_RC4_KEY "abcdabcdabcdabcd"
#else
#define DEFAULT_RC4_KEY (RC4_KEY)
#endif

// RC4 Key Length
#ifndef RC4_KEY_LEN
#define DEFAULT_RC4_KEY_LEN 16
#else
#define DEFAULT_RC4_KEY_LEN (RC4_KEY_LEN)
#endif

// Use RC4 or not
#ifndef RC4_USE
#define DEFAULT_RC4_USE false
#else
#define DEFAULT_RC4_USE (RC4_USE)
#endif

// AES Key
#ifndef AES_KEY
#define DEFAULT_AES_KEY "1234567890ABCDEF1234567890ABCDEF"
#else
#define DEFAULT_AES_KEY (AES_KEY)
#endif

// AES IV
#ifndef AES_IV
#define DEFAULT_AES_IV "1234567890ABCDEF"
#else
#define DEFAULT_AES_IV (AES_IV)
#endif

// DES Key
#ifndef DES_KEY
#define DEFAULT_DES_KEY "1234567890ABCDEF"
#else
#define DEFAULT_DES_KEY (DES_KEY)
#endif

// DES IV
#ifndef DES_IV
#define DEFAULT_DES_IV "12345678"
#else
#define DEFAULT_DES_IV (DES_IV)
#endif

// XOR Key
#ifndef XOR_KEY
#define DEFAULT_XOR_KEY 0xA9
#else
#define DEFAULT_XOR_KEY (XOR_KEY)
#endif

// Signature
#ifndef SIGNATURE
#define DEFAULT_SIGNATURE "tempsig3"
#else
#define DEFAULT_SIGNATURE (SIGNATURE)
#endif

// Debug Mode
#ifndef DEBUG
#define DEFAULT_DEBUG 0
#else
#define DEFAULT_DEBUG (DEBUG)
#endif


extern std::string rc4_string_key;
extern CryptoPP::SecByteBlock rc4_key;
extern DWORD rc4_keyLength;
extern bool use_rc4;
extern std::string aes_string_key;
extern CryptoPP::SecByteBlock aes_key;
extern CryptoPP::byte aes_iv[];
extern std::string des_string_key;
extern CryptoPP::SecByteBlock des_key;
extern CryptoPP::byte des_iv[];
extern std::string sig;
extern std::vector<unsigned char> signature;
extern long int data_size;

extern unsigned char xor_key;

extern int debug;
std::string currentDateTime();
void debug_message(std::string, std::string);


#endif