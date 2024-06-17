#ifndef _HELPER_
#define _HELPER_

#pragma comment(lib, "dbghelp.lib") 

#include <stdio.h>
#include <windows.h>
#include <DbgHelp.h>
#include <time.h>
#include <iostream>
#include <softpub.h> 

#include "cryptopp\osrng.h"
#include "cryptopp\aes.h"
#include "cryptopp\des.h"
#include "cryptopp\filters.h"
#include "cryptopp\modes.h"
#include "cryptopp\hex.h"
#include "cryptopp\cryptlib.h"

#define SHA1LEN  20

#ifndef ENCRYPTION_ORDER
#define DEFAULT_ENCRYPTION_ORDER {DES,AES,XOR}
#else
#define DEFAULT_ENCRYPTION_ORDER ENCRYPTION_ORDER
#endif

extern unsigned int PEChecksum(void *FileBase, unsigned int FileSize);
extern char* genRandomBytes(size_t length);
extern char* genKey();
extern void *memcopy(void *const dest, void const *const src, size_t bytes);
extern int crypt(unsigned char* data, long dataLen, unsigned char* result);
extern std::vector<unsigned char> rc4(std::vector<unsigned char>);
extern std::vector<unsigned char> AES(std::vector<unsigned char>);
extern std::vector<unsigned char> DES(std::vector<unsigned char>);
extern std::vector<unsigned char> XOR(std::vector<unsigned char>);
extern BOOL IsWow64(HANDLE pHandle);
extern BOOL Sha1(BYTE* peBlob, char* sha1Buf, DWORD dwBufferLen);
extern DWORD VerifyPESignature(PCWSTR FileName, HANDLE FileHandle);
extern void toMultiByte(DWORD strLen, CHAR* _Str, LPWSTR _wStr);
extern char* getFName(char* _fPath);

#endif 