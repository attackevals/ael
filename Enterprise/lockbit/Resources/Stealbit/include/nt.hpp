
#pragma once
#include <Windows.h>
#include <winternl.h>
#include "logger.hpp"
#include "xor_string.hpp"

#define ProcessDefaultHardErrorMode (PROCESSINFOCLASS)12


typedef NTSTATUS(NTAPI* pNtSetInformationProcess) (
    _In_ HANDLE ProcessHandle,
    _In_ PROCESSINFOCLASS ProcessInformationClass,
    _In_reads_bytes_(ProcessInformationLength) PVOID ProcessInformation,
    _In_ ULONG ProcessInformationLength
);




namespace errorhandling {

    /*
     * SetHardErrorMode:
     *      About:
     *          Uses NtSetInformationProcess to adjust the process hard error mode (PPROCESSINFOCLASS)12
     *      Result:
     *          Process errors usually shown to the user are suppressed.
     *      Returns:
     *          void
     *      MITRE ATT&CK Techniques:
     *          n/a
     *      CTI:
     *          https://www.cybereason.com/blog/research/threat-analysis-report-inside-the-lockbit-arsenal-the-stealbit-exfiltration-tool
     *      Other References:
     *          https://chuongdong.com/reverse%20engineering/2022/03/19/LockbitRansomware/#default-error--privilege-setting
     *          https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-seterrormode
     */
    void SetHardErrorMode();
}