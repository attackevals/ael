#include <codecvt>
#include <string_view>

// Defined in src/common/util/string_utils.cpp
namespace string_util {

// Converts the given narrow string to a wide string
std::wstring string_to_wstring(const std::string& str);

// Converts the given wide string to a narrow string
std::string wstring_to_string(const std::wstring& str);

// Removes leading whitespace from the string
std::string ltrim(std::string s);

// Removes trailing whitespace from the string
std::string rtrim(std::string s);

// Converts the string to lowercase
std::string to_lower(std::string s);

// Removes the given suffix from the string
std::wstring trim_suffix(const std::wstring& s, const std::wstring& suffix);

} // namespace
