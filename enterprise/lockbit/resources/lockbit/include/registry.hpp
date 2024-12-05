#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <winreg.h>
#include <format>
#include <stdexcept>
#include <vector>

using FP_RegCloseKey = decltype(&RegCloseKey);
using FP_RegCreateKeyExW = decltype(&RegCreateKeyExW);
using FP_RegSetValueExW = decltype(&RegSetValueExW);

// Defined in src/common/util/registry.cpp
namespace registry {

// Opens or creates a registry key - wrapper for RegCreateKeyEx.
bool CreateRegistrykey(HKEY h_base_key, const std::wstring& sub_key, REGSAM sam, PHKEY p_h_key_result);

// Writes a string value to the specified registry key, creating it if it doesn't exist.
bool WriteRegistry(HKEY h_base_key, const std::wstring& sub_key, const std::wstring& value, const std::wstring& data);

// Writes a DWORD value to the specified registry key, creating it if it doesn't exist.
bool WriteRegistry(HKEY h_base_key, const std::wstring& sub_key, const std::wstring& value, DWORD data);

// Writes a binary blob to the specified registry key, creating it if it doesn't exist.
bool WriteRegistry(HKEY h_base_key, const std::wstring& sub_key, const std::wstring& value, const unsigned char* data, size_t data_len);

// Reads binary blob from the specified registry key and value. Throws exception on failure.
std::vector<unsigned char> ReadBinaryRegistry(HKEY h_base_key, const std::wstring& sub_key, const std::wstring& value);

} // namespace