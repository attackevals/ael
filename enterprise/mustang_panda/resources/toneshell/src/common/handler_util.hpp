#pragma once

#include "api_helper.hpp"
#include "shared_func.hpp"
#include "obfuscation.hpp"

#define FAIL_GET_VIRTUALALLOC 0x56021
#define FAIL_GET_VIRTUALPROTECT 0x56022
#define FAIL_GET_VIRTUALFREE 0x56023
#define FAIL_GET_CREATEPROCESSW 0x56024
#define FAIL_GET_GETMODULEFILENAMEW 0x56025
#define FAIL_GET_CREATETHREAD 0x56026
#define FAIL_GET_GETFOREGROUNDWINDOW 0x56027
#define FAIL_GET_SLEEP 0x56028

using VirtualAlloc_t = decltype(&VirtualAlloc);
using VirtualProtect_t = decltype(&VirtualProtect);
using VirtualFree_t = decltype(&VirtualFree);
using CreateProcessW_t = decltype(&CreateProcessW);
using GetModuleFileNameA_t = decltype(&GetModuleFileNameA);
using GetModuleFileNameW_t = decltype(&GetModuleFileNameW);
using CreateThread_t = decltype(&CreateThread);
using GetForegroundWindow_t = decltype(&GetForegroundWindow);
using Sleep_t = decltype(&Sleep);

DWORD ResolveSharedApis(shared_func_pointers* fp);

BOOL FilePathHasLeaf(LPCWSTR path, LPCWSTR target);
