#pragma once
#include "handles.hpp"
#include "errors.hpp"
#include <string_view>
#include <vector>
#include <span>
#include <expected>
#include <strsafe.h>

namespace common {

// Opens or creates a registry key to be used transactionally.
std::expected<common::unique_hkey, common::windows_error>
create_registry_key(
    HANDLE transaction,
    std::wstring_view key,
    REGSAM access,
    HKEY hive = HKEY_LOCAL_MACHINE
);

// Reads binary data from a registry key value.
std::expected<std::vector<std::byte>, common::windows_error>
read_registry_binary(HKEY key, std::wstring_view value);

// Reads a string value from a registry key.
std::expected<std::wstring, common::windows_error>
read_registry_string(HKEY key, std::wstring_view value);

// Reads a DWORD value from a registry key.
std::expected<unsigned long, common::windows_error>
read_registry_dword(HKEY key, std::wstring_view value);

// Writes a string value to the specified registry key.
std::expected<void, common::windows_error>
write_registry(HKEY key, std::wstring_view value, std::wstring_view str);

// Writes binary data as a value to the specified registry key.
std::expected<void, common::windows_error>
write_registry(HKEY key, std::wstring_view value, std::span<std::byte> buf);

// Writes a DWORD value to the specified registry key.
std::expected<void, common::windows_error>
write_registry(HKEY key, std::wstring_view value, unsigned long dword);

// Reads binary data from a registry key value into an array.
std::expected<std::span<std::byte>, const wchar_t*>
early_read_registry_binary(
    HKEY key,
    std::wstring_view subkey,
    std::wstring_view value
);

} // namespace common
