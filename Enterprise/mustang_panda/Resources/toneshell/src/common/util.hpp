#pragma once

#include "shared_func.hpp"

#define _WIDENHELPER(x) L ## x
#define WIDEN(x) _WIDENHELPER(x)

#define STATUS_SUCCESS (NTSTATUS)0x00000000

DWORD WriteFileBytes(shared_func_pointers* fp, HANDLE h_file, char* buffer, DWORD buffer_len);

void pi_memcpy(void* dst, const void* src, size_t n);

void pi_memset(void* dst, unsigned char a, size_t n);
