#include <codecvt>
#include <string_view>

namespace string_util {

std::wstring string_to_wstring(const std::string& str);

std::string wstring_to_string(const std::wstring& str);

std::string hresult_to_hex_str(long h);

} // namespace