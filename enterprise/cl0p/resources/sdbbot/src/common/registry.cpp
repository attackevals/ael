#include "registry.hpp"

namespace common {

/*
 * create_registry_key:
 *      About:
 *          Creates or opens the specified registry key in the specified hive using the
 *          RegCreateKeyTransactedW API call.
 *      Result:
 *          Registry key created or opened
 *      MITRE ATT&CK Techniques:
 *          T1112: Modify Registry
 *          T1106: Native API
 *      CTI:
 *          https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
 */
std::expected<common::unique_hkey, common::windows_error>
create_registry_key(
    HANDLE transaction,
    std::wstring_view key,
    REGSAM access,
    HKEY hive
) {
    common::unique_hkey key_handle;

    LSTATUS status = RegCreateKeyTransactedW(
        hive,
        key.data(),
        0,
        nullptr,
        REG_OPTION_NON_VOLATILE,
        access,
        nullptr,
        key_handle.addressof(),
        nullptr,
        transaction,
        nullptr
    );
    if (ERROR_SUCCESS != status) {
        return std::unexpected(common::windows_error(
            status,
            std::system_category()
        ));
    }
    return key_handle;
}

/*
 * read_registry_binary:
 *      About:
 *          Reads the specified binary value from the specified registry key using the
 *          RegGetValueW API call.
 *      Result:
 *          vector of std::bytes containing the binary registry data
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 *      CTI:
 *          https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
 */
std::expected<std::vector<std::byte>, common::windows_error>
read_registry_binary(HKEY key, std::wstring_view value) {
    std::vector<std::byte> buf{};
    unsigned long size = 0;

    // Get the size of the string
    LSTATUS status = RegGetValueW(
        key,
        nullptr,
        value.data(),
        RRF_RT_REG_BINARY,
        nullptr,
        nullptr,
        &size
    );
    if (ERROR_SUCCESS != status) {
        return std::unexpected(common::windows_error(
            status,
            std::system_category()
        ));
    }

    // Resize the buffer to the string size. The value returned is in bytes
    // and accounts for a null byte which we don't need. Since this will hold
    // shellcode occasionally we simply allow the padding.
    buf.resize(size);
    status = RegGetValueW(
        key,
        nullptr,
        value.data(),
        RRF_RT_REG_BINARY,
        nullptr,
        buf.data(),
        &size
    );
    if (ERROR_SUCCESS != status) {
        return std::unexpected(common::windows_error(
            status,
            std::system_category()
        ));
    }
    return buf;
}

/*
 * read_registry_string:
 *      About:
 *          Reads the specified string value from the specified registry key using the
 *          RegGetValueW API call.
 *      Result:
 *          std::wstring containing the registry data
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 */
std::expected<std::wstring, common::windows_error>
read_registry_string(HKEY key, std::wstring_view value) {
    std::wstring str{};
    unsigned long size = 0;

    // Get the size of the string
    LSTATUS status = RegGetValueW(
        key,
        nullptr,
        value.data(),
        RRF_RT_REG_SZ,
        nullptr,
        nullptr,
        &size
    );
    if (ERROR_SUCCESS != status) {
        return std::unexpected(common::windows_error(
            status,
            std::system_category()
        ));
    }

    // Resize the buffer to the string size. The value returned is in bytes
    // and accounts for a null byte which we don't need. Since this will hold
    // shellcode occasionally we simply allow the padding.
    str.resize(size / sizeof(wchar_t));
    status = RegGetValueW(
        key,
        nullptr,
        value.data(),
        RRF_RT_REG_SZ,
        nullptr,
        str.data(),
        &size
    );
    if (ERROR_SUCCESS != status) {
        return std::unexpected(common::windows_error(
            status,
            std::system_category()
        ));
    }
    return str;
}

/*
 * read_registry_dword:
 *      About:
 *          Reads the specified DWORD value from the specified registry key using the
 *          RegGetValueW API call.
 *      Result:
 *          unsigned long containing the registry data
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 */
std::expected<unsigned long, common::windows_error>
read_registry_dword(HKEY key, std::wstring_view value) {
    unsigned long data{};
    unsigned long size = sizeof(unsigned long);

    LSTATUS status = RegGetValueW(
        key,
        nullptr,
        value.data(),
        RRF_RT_REG_DWORD,
        nullptr,
        &data,
        &size
    );
    if (ERROR_SUCCESS != status) {
        return std::unexpected(common::windows_error(
            status,
            std::system_category()
        ));
    }
    return data;
}

/*
 * write_registry:
 *      About:
 *          Writes the specified string data to the specified registry key and value,
 *          using the RegSetKeyValueW API call.
 *      Result:
 *          Successful write or error
 *      MITRE ATT&CK Techniques:
 *          T1112: Modify Registry
 *          T1106: Native API
 *      CTI:
 *          https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
 */
std::expected<void, common::windows_error>
write_registry(HKEY key, std::wstring_view value, std::wstring_view str) {
    LSTATUS status = RegSetKeyValueW(
        key,
        nullptr,
        value.data(),
        REG_SZ,
        str.data(),
        str.size() * sizeof(wchar_t)
    );
    if (ERROR_SUCCESS != status) {
        return std::unexpected(common::windows_error(
            status,
            std::system_category()
        ));
    }
    return {};
}

/*
 * write_registry:
 *      About:
 *          Writes the specified binary data to the specified registry key and value,
 *          using the RegSetKeyValueW API call.
 *      Result:
 *          Successful write or error
 *      MITRE ATT&CK Techniques:
 *          T1112: Modify Registry
 *          T1106: Native API
 *      CTI:
 *          https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
 */
std::expected<void, common::windows_error>
write_registry(HKEY key, std::wstring_view value, std::span<std::byte> buf) {
    LSTATUS status = RegSetKeyValueW(
        key,
        nullptr,
        value.data(),
        REG_BINARY,
        buf.data(),
        buf.size_bytes()
    );
    if (ERROR_SUCCESS != status) {
        return std::unexpected(common::windows_error(
            status,
            std::system_category()
        ));
    }
    return {};
}

/*
 * write_registry:
 *      About:
 *          Writes the specified DWORD data to the specified registry key and value,
 *          using the RegSetKeyValueW API call.
 *      Result:
 *          Successful write or error
 *      MITRE ATT&CK Techniques:
 *          T1112: Modify Registry
 *          T1106: Native API
 *      CTI:
 *          https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
 */
std::expected<void, common::windows_error>
write_registry(HKEY key, std::wstring_view value, unsigned long dword) {
    LSTATUS status = RegSetKeyValueW(
        key,
        nullptr,
        value.data(),
        REG_DWORD,
        &dword,
        sizeof(unsigned long)
    );
    if (ERROR_SUCCESS != status) {
        return std::unexpected(common::windows_error(
            status,
            std::system_category()
        ));
    }
    return {};
}

/*
 * early_read_registry_binary:
 *      About:
 *          Reads the specified binary value from the specified registry key using the
 *          RegGetValueW API call. Intended to be called at verifier DLL runtime.
 *      Result:
 *          span of std::bytes containing the binary registry data
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 *      CTI:
 *          https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
 */
std::expected<std::span<std::byte>, const wchar_t*>
early_read_registry_binary(
    HKEY key,
    std::wstring_view subkey,
    std::wstring_view value
) {
    unsigned long size = 0;
    LSTATUS status = RegGetValueW(
        key,
        subkey.data(),
        value.data(),
        RRF_RT_REG_BINARY,
        nullptr,
        nullptr,
        &size
    );
    if (ERROR_SUCCESS != status) {
        return std::unexpected(get_error_message_wchar(status));
    }

    void* buffer = VirtualAlloc(
        nullptr,
        size,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_EXECUTE_READWRITE
    );

    status = RegGetValueW(
        key,
        subkey.data(),
        value.data(),
        RRF_RT_REG_BINARY,
        nullptr,
        buffer,
        &size
    );

    if (status != ERROR_SUCCESS) {
        return std::unexpected(get_error_message_wchar(status));
    }

    return std::span<std::byte>{reinterpret_cast<std::byte*>(buffer), size};
}

} // namespace common
