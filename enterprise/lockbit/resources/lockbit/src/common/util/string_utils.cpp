#include "util/string_util.hpp"
#include <algorithm>
#include <cctype>

namespace string_util {

// Converts the given narrow string to a wide string
std::wstring string_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

// Converts the given wide string to a narrow string
std::string wstring_to_string(const std::wstring& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(str);
}

// Removes leading whitespace from the string
// Reference: https://stackoverflow.com/a/217605
std::string ltrim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char c) { return !std::isspace(c); }));
    return s;
}

// Removes trailing whitespace from the string
// Reference: https://stackoverflow.com/a/217605
std::string rtrim(std::string s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](char c) { return !std::isspace(c); }).base(), s.end());
    return s;
}

// Converts the string to lowercase
std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

// Removes the given suffix from the string
std::wstring trim_suffix(const std::wstring& s, const std::wstring& suffix) {
    if (suffix.length() > 0) {
        auto trim_pos = s.rfind(suffix);
        if (trim_pos != s.npos) {
            return s.substr(0, trim_pos);
        }
    }
    return std::wstring(s);
}

} // namespace
