#include "nt.hpp"
#include <iostream>
namespace errorhandling {

    void SetHardErrorMode() {
        int hard_error_mode = SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOALIGNMENTFAULTEXCEPT;

        HMODULE hNTDLL = GetModuleHandle(XOR_LIT("ntdll.dll"));
        if (!hNTDLL) {
            XorLogger::LogError(XOR_LIT("Failed to load NTDLL"));
            return;
        }
        pNtSetInformationProcess NtSetInformationProcess = (pNtSetInformationProcess)GetProcAddress(hNTDLL, XOR_LIT("NtSetInformationProcess"));
        if (!NtSetInformationProcess) {
            XorLogger::LogError(XOR_LIT("Failed to get address for NtSetInformationProcess"));
            return;
        }
        NTSTATUS status = NtSetInformationProcess(GetCurrentProcess(), ProcessDefaultHardErrorMode, &hard_error_mode, 4);
        if (!status) {
            XorLogger::LogDebug(XOR_LIT("Suppressed process errors"));
        }
        else {
            XorLogger::LogDebug(XOR_LIT("Failed to suppress process errors"));
        }
        return;
    }
}

