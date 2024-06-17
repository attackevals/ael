#ifndef __LAYER2_LOADER__
#define __LAYER2_LOADER__



#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <intrin.h>
#include <stdexcept>
#include <memory>
#include <windows.h>

#include "cryptopp\osrng.h"
#include "cryptopp\aes.h"
#include "cryptopp\des.h"
#include "cryptopp\filters.h"
#include "cryptopp\modes.h"
#include "cryptopp\hex.h"
#include "cryptopp\cryptlib.h"

#include "encryption_def.h"

#ifndef ENCRYPTION_ORDER
#define DEFAULT_ENCRYPTION_ORDER {DES,AES,XOR}
#else
#define DEFAULT_ENCRYPTION_ORDER ENCRYPTION_ORDER
#endif

#ifndef FILEPATH_2
#define DEFAULT_FILEPATH ".\\sodamaster"
#else
#define DEFAULT_FILEPATH2 FILEPATH_2
#endif



extern std::string filepath;

extern "C" __declspec (dllexport) int LoadData();
void run_code(std::vector<unsigned char>);
std::vector<unsigned char> read_file(std::string);
std::vector<unsigned char> AES(std::vector<unsigned char>);
std::vector<unsigned char> DES(std::vector<unsigned char>);
std::vector<unsigned char> XOR(std::vector<unsigned char>);
std::vector<unsigned char> RC4(std::vector<unsigned char>);
std::vector<unsigned char> decrypt(std::vector<unsigned char>);
bool check_signature(std::vector<unsigned char>, std::vector<unsigned char>);

// Change the oreder of the values to change what encryption algorithm is used in what order
//extern std::vector<unsigned char>encryption_order;


#endif