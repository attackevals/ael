#pragma once

#include "shellcode.hpp"
#include "util.hpp"

#include <al/string.hpp>

#define _XOR_MACRO_HELPER(x) x ## _xor
#define XOR_MACRO(x) _XOR_MACRO_HELPER(x)

void pi_widen_str(const char* src, size_t src_size, wchar_t* dst, size_t dst_size);

void pi_concat_wstrn(wchar_t* dst, size_t dst_size, LPCWSTR src);

// For detections, implemented in shellcode_util_id_d.cpp
// For protections, implemented in shellcode_util_id_p.cpp
DWORD GenerateNewVictimID(sh_context* ctx);

DWORD SaveVictimID(sh_context* ctx);

DWORD GetHostname(sh_context* ctx);

DWORD FetchFunctions(func_pointers* fp);
