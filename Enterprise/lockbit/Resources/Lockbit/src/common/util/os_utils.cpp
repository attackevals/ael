#include <format>
#include <sstream>
#include <thread>
#include "crypto/xor_obfuscation.hpp"
#include "logger.hpp"
#include "util/os_util.hpp"
#include "util/string_util.hpp"

namespace os_util {

// Gets the ID of the calling thread
std::string GetThreadIdStr() {
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

// Get absolute path to current executable
std::wstring GetCurrExecutablePath() {
    wchar_t buffer[MAX_PATH];
    DWORD num_written = GetModuleFileNameW(NULL, buffer, MAX_PATH);
    if (num_written == 0) {
        throw std::invalid_argument(std::format("{} {}", XOR_LIT("GetModuleFileName failed with error code"), GetLastError()));
    }
    return std::wstring(buffer);
}

// Reads given file into buffer
DWORD ReadFileBytes(HANDLE h_file, unsigned char* buffer, DWORD bytes_to_read) {
    unsigned char* p_seek_buffer = buffer;
    DWORD remaining_bytes = bytes_to_read;
    DWORD bytes_read;
    while (remaining_bytes > 0) {
        if (!ReadFile(h_file, p_seek_buffer, remaining_bytes, &bytes_read, NULL)) {
            return GetLastError();
        }
        p_seek_buffer += bytes_read;
        remaining_bytes -= bytes_read;
    }
    return ERROR_SUCCESS;
}

// Writes buffer to specified file
DWORD WriteFileBytes(HANDLE h_file, char* buffer, DWORD buffer_len) {
    char* p_seek_buffer = buffer;
    DWORD remaining_bytes = buffer_len;
    DWORD bytes_written;
    while (remaining_bytes > 0) {
        if (!WriteFile(h_file, p_seek_buffer, remaining_bytes, &bytes_written, NULL)) {
            return GetLastError();
        }
        p_seek_buffer += bytes_written;
        remaining_bytes -= bytes_written;
    }
    return ERROR_SUCCESS;
}

// Writes buffer to specified file on disk
DWORD WriteFileToDisk(LPCWSTR path, char* buffer, DWORD buffer_len) {
    HANDLE h_dest_file = CreateFileW(
        path,
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS, // always create new file, overwriting if already exists
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (h_dest_file == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }
    DWORD result = WriteFileBytes(h_dest_file, buffer, buffer_len);
    if (h_dest_file != INVALID_HANDLE_VALUE) {
        CloseHandle(h_dest_file);
    }
    return result;
}

// Gets the value of the specified environment variable
std::string GetEnvVariable(LPCSTR var) {
    char* result = std::getenv(var);
    if (result == NULL) {
        return std::string("");
    } else {
        return std::string(result);
    }
}

// Gets the path to the PROGRAMDATA folder
std::filesystem::path GetProgramDataPath() {
    std::string path_str = GetEnvVariable(XOR_LIT("PROGRAMDATA"));
    if (path_str.empty()) {
        XorLogger::LogError(XOR_LIT("Failed to get PROGRAMDATA environment variable. Defaulting to C:\\ProgramData"));
        path_str = std::string(XOR_LIT("C:\\ProgramData"));
    }
    return std::filesystem::path(path_str);
}

} // namespace util
