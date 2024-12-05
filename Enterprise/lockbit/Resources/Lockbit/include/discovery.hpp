#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif

#include <string>
#include <Windows.h>
#include <security.h>
#include <secext.h>
#include "util/winapihelper.hpp"

using FP_GetComputerNameW = decltype(&GetComputerNameW);
using FP_GetComputerNameExW = decltype(&GetComputerNameExW);
using FP_GetUserNameExW = decltype(&GetUserNameExW);
typedef NTSTATUS (WINAPI* FP_RtlGetVersion)(PRTL_OSVERSIONINFOW);

// Defined in src/lockbit_main/discovery.cpp
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
std::string GetUsername();

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
std::string GetHostname();

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
std::string GetDomain();

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
std::string GetOsArchitecture();

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
std::string GetOsVersionString();

}