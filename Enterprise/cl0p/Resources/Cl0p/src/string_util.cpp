#include "string_util.hpp"

namespace string_util {

std::string wstring_to_string(const std::wstring& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(str);
}

} // namespace