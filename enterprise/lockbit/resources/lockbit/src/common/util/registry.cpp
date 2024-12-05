#include "crypto/xor_obfuscation.hpp"
#include "registry.hpp"
#include "logger.hpp"
#include "util/string_util.hpp"

#ifndef NO_WIN_API_HASHING
    #include "util/winapihelper.hpp"
#endif

namespace registry {

// Opens or creates a registry key - wrapper for RegCreateKeyEx.
bool CreateRegistrykey(HKEY h_base_key, const std::wstring& sub_key, REGSAM sam, PHKEY p_h_key_result) {
#ifndef NO_WIN_API_HASHING
    DWORD error_code;
    FP_RegCreateKeyExW regCreateKeyExW = (FP_RegCreateKeyExW)winapi_helper::GetAPI(0x46ceb3b4, XOR_WIDE_LIT(L"Advapi32.dll"), &error_code);
    if (regCreateKeyExW == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for RegCreateKeyExW. Error code"), error_code));
        return false;
    }
#endif
    
    DWORD disposition;
    std::string key_str = string_util::wstring_to_string(sub_key);
#ifndef NO_WIN_API_HASHING
    LSTATUS result = regCreateKeyExW(
#else
    LSTATUS result = RegCreateKeyExW(
#endif
        h_base_key, 
        sub_key.c_str(), 
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        sam,
        NULL,
        p_h_key_result,
        &disposition
    );
    if (result != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{} {}. LSTATUS: {}", XOR_LIT("Failed to create registry key"), key_str, result));
        return false;
    }
    if (disposition == REG_CREATED_NEW_KEY) {
        XorLogger::LogDebug(std::format("{} {}.", XOR_LIT("Successfully created new registry key"), key_str));
    } else if (disposition == REG_OPENED_EXISTING_KEY) {
        XorLogger::LogDebug(std::format("{} {}.", XOR_LIT("Opened existing registry key"), key_str));
    }
    return true;
}

bool writeRegistryHelper(HKEY h_base_key, const std::wstring& sub_key, const std::wstring& value, DWORD data_type, const unsigned char* data, size_t data_len) {
#ifndef NO_WIN_API_HASHING
    DWORD error_code;
    FP_RegCloseKey regCloseKey = (FP_RegCloseKey)winapi_helper::GetAPI(0x736b3702, XOR_WIDE_LIT(L"Advapi32.dll"), &error_code);
    if (regCloseKey == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for RegCloseKey. Error code"), error_code));
        return false;
    }
    FP_RegSetValueExW regSetValueExW = (FP_RegSetValueExW)winapi_helper::GetAPI(0x34587300, XOR_WIDE_LIT(L"Advapi32.dll"), &error_code);
    if (regSetValueExW == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for RegSetValueExW. Error code"), error_code));
        return false;
    }
#endif
    
    HKEY h_key = NULL;
    std::string key_str = string_util::wstring_to_string(sub_key);
    std::string value_str = string_util::wstring_to_string(value);
    if (value_str.empty()) {
        value_str = "(default)";
    }
    if (!CreateRegistrykey(h_base_key, sub_key, KEY_WRITE, &h_key) || h_key == NULL) {
        XorLogger::LogError(std::format("{} {}", XOR_LIT("Failed to write to registry key"), key_str));
        return false;
    }
#ifndef NO_WIN_API_HASHING
    LSTATUS result = regSetValueExW(h_key, value.c_str(), 0, data_type, data, data_len);
    regCloseKey(h_key);
#else
    LSTATUS result = RegSetValueExW(h_key, value.c_str(), 0, data_type, data, data_len);
    RegCloseKey(h_key);
#endif
    if (result != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{} {} at value {}. LSTATUS: {}", XOR_LIT("Failed to write data to registry key"), key_str, value_str, result));
        return false;
    } else {
        XorLogger::LogDebug(std::format("{} {} at value {}.", XOR_LIT("Successfully wrote data to registry key"), key_str, value_str));
        return true;
    }
}

// Writes a string value to the specified registry key. Will create the key if it does not exist.
bool WriteRegistry(HKEY h_base_key, const std::wstring& sub_key, const std::wstring& value, const std::wstring& data) {
    size_t data_size = (data.length() + 1) * sizeof(wchar_t);
    return writeRegistryHelper(h_base_key, sub_key, value, REG_SZ, (const BYTE *)(data.c_str()), data_size);
}

// Writes a DWORD value to the specified registry key. Will create the key if it does not exist.
bool WriteRegistry(HKEY h_base_key, const std::wstring& sub_key, const std::wstring& value, DWORD data) {
    DWORD to_write = data;
    return writeRegistryHelper(h_base_key, sub_key, value, REG_DWORD, (const BYTE *)(&to_write), sizeof(to_write));
}

// Writes a binary blob to the specified registry key. Will create the key if it does not exist.
bool WriteRegistry(HKEY h_base_key, const std::wstring& sub_key, const std::wstring& value, const unsigned char* data, size_t data_len) {
    return writeRegistryHelper(h_base_key, sub_key, value, REG_BINARY, data, data_len);
}

// Reads binary blob from the specified registry key and value. Throws exception on failure.
std::vector<unsigned char> ReadBinaryRegistry(HKEY h_base_key, const std::wstring& sub_key, const std::wstring& value) {    
    std::string key_str = string_util::wstring_to_string(sub_key);
    std::string value_str = string_util::wstring_to_string(value);
    if (value_str.empty()) {
        value_str = "(default)";
    }
    DWORD size;
    LSTATUS result = RegGetValueW(h_base_key, sub_key.c_str(), value.c_str(), RRF_RT_REG_BINARY, NULL, NULL, &size);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("First RegGetValueW call failed with LSTATUS: {}", result));
    }
    std::vector<unsigned char> buffer(size);
    result = RegGetValueW(h_base_key, sub_key.c_str(), value.c_str(), RRF_RT_REG_BINARY, NULL, buffer.data(), &size);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("Second RegGetValueW call failed with LSTATUS: {}", result));
    } else {
        XorLogger::LogDebug(std::format(
            "Successfully read {} bytes from registry key {} at value {}.",
            size,
            key_str, 
            value_str
        ));
    }
    return buffer;
}

} // namespace

