#include <format>
#include "xor_obfuscation.hpp"
#include "logger.hpp"
#include "os_util.hpp"
#include "string_util.hpp"

namespace os_util {

// Get short form of absolute path to current executable
std::wstring GetCurrExecutableShortPath() {
    std::vector<wchar_t> buffer(MAX_PATH);
    DWORD ret = GetModuleFileNameW(NULL, buffer.data(), buffer.size());
    if (ret == 0) {
        throw std::invalid_argument(std::format("{} {}", XOR_LIT("GetModuleFileName failed with error code"), GetLastError()));
    }
    std::wstring abs_path(buffer.data());
    XorLogger::LogDebug(std::format(
        "{}: {}", 
        XOR_LIT("Found absolute path to current executable"),
        string_util::wstring_to_string(abs_path)
    ));
    ret = GetShortPathNameW(abs_path.c_str(), buffer.data(), buffer.size());
    if (ret == 0) {
        throw std::invalid_argument(std::format("{} {}", XOR_LIT("GetShortPathNameW failed with error code"), GetLastError()));
    } else if (ret > buffer.size()) {
        buffer.resize(ret);
        XorLogger::LogDebug(std::format("{} {}", XOR_LIT("Resizing GetShortPathNameW buffer to"), ret));
        ret = GetShortPathNameW(abs_path.c_str(), buffer.data(), buffer.size());
        if (ret == 0) {
            throw std::invalid_argument(std::format("{} {}", XOR_LIT("GetShortPathNameW failed after resize with error code"), GetLastError()));
        }
    }
    std::wstring short_path(buffer.data());
    XorLogger::LogDebug(std::format(
        "{}: {}", 
        XOR_LIT("Found short path to current executable"),
        string_util::wstring_to_string(short_path)
    ));
    return short_path;
}

} // namespace util
