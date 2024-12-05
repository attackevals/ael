#include "string_util.hpp"
#include <sstream>

namespace string_util {

std::wstring string_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string wstring_to_string(const std::wstring& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(str);
}

std::string hresult_to_hex_str(long h) {
    std::ostringstream oss;
    oss << "0x" << std::hex << h;
    return oss.str();
}

} // namespace