#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <stdio.h>
#include <bcrypt.h>

#define FAIL_LOAD_BCRYPT 0x50005

#define FAIL_GET_GETLASTERROR 0x51001
#define FAIL_GET_CREATEFILEW 0x51002
#define FAIL_GET_CLOSEHANDLE 0x51003
#define FAIL_GET_SNPRINTF 0x51004
#define FAIL_GET_VSNPRINTF 0x51005
#define FAIL_GET_GETSYSTEMTIME 0x51006
#define FAIL_GET_SETFILEPOINTER 0x51007
#define FAIL_GET_HEAPALLOC 0x5101C
#define FAIL_GET_HEAPCREATE 0x5101E
#define FAIL_GET_HEAPDESTROY 0x5101F
#define FAIL_GET_HEAPFREE 0x51020
#define FAIL_GET_WRITEFILE 0x51021
#define FAIL_GET_BCRYPTGENRANDOM 0x51022

#define FAIL_HEAP_ALLOCATION 0x52001

using GetLastError_t = decltype(&GetLastError);
using CreateFileW_t = decltype(&CreateFileW);
using CloseHandle_t = decltype(&CloseHandle);
using vsnprintf_t = decltype(&_vsnprintf);
using snprintf_t = decltype(&_snprintf);
using GetSystemTime_t = decltype(&GetSystemTime);
using SetFilePointer_t = decltype(&SetFilePointer);
using HeapAlloc_t = decltype(&HeapAlloc);
using HeapCreate_t = decltype(&HeapCreate);
using HeapDestroy_t = decltype(&HeapDestroy);
using HeapFree_t = decltype(&HeapFree);
using WriteFile_t = decltype(&WriteFile);
using BCryptGenRandom_t = decltype(&BCryptGenRandom);

struct shared_func_pointers {
    GetLastError_t fp_GetLastError;
    CreateFileW_t fp_CreateFileW;
    CloseHandle_t fp_CloseHandle;
    vsnprintf_t fp_vsnprintf;
    snprintf_t fp_snprintf;
    GetSystemTime_t fp_GetSystemTime;
    SetFilePointer_t fp_SetFilePointer;
    HeapAlloc_t fp_HeapAlloc;
    HeapCreate_t fp_HeapCreate;
    HeapDestroy_t fp_HeapDestroy;
    HeapFree_t fp_HeapFree;
    WriteFile_t fp_WriteFile;
    BCryptGenRandom_t fp_BCryptGenRandom;
};
