#pragma once

#include "shared_func.hpp"

#define _XOR_MACRO_HELPER(x) x ## _xor
#define XOR_MACRO(x) _XOR_MACRO_HELPER(x)

#define _WIDENHELPER(x) L ## x
#define WIDEN(x) _WIDENHELPER(x)

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS (NTSTATUS)0x00000000
#endif

DWORD WriteFileBytes(shared_func_pointers* fp, HANDLE h_file, char* buffer, DWORD buffer_len);

void pi_memcpy(void* dst, const void* src, size_t n);

void pi_memset(void* dst, unsigned char a, size_t n);
