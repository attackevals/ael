#include <fstream>
#include "execute.hpp"
#include "test_utils.hpp"
#include "util/string_util.hpp"

namespace test_utils {

void ExecuteProcessAndVerifyOutput(LPWSTR command_line, const std::string& want, bool partial_comparison=false) {
    DWORD error_code;
    DWORD exit_code;
    std::vector<char> output = execute::ExecuteProcess(command_line, 5, &error_code, &exit_code); 
    ASSERT_EQ(error_code, ERROR_SUCCESS);
    std::string result(output.begin(), output.end());
    std::cout << "Command: " << string_util::wstring_to_string(command_line) << std::endl;
    std::cout << "Output: " << result << std::endl;
    ASSERT_EQ(exit_code, 0);
    if (partial_comparison) {
        ASSERT_NE(std::string::npos, result.find(want));
    } else {
        ASSERT_EQ(string_util::rtrim(result), want);
    }
}

void CompareFileHash(const std::wstring& path, const std::string& want) {
    wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
    swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"powershell.exe -command \"(get-filehash '%s').hash\"", path.c_str());
    ExecuteProcessAndVerifyOutput(command_line, want);
}

void CompareRegContentsDefaultValue(const std::wstring& key_path, const std::string& expected) {
    wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
    swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"reg.exe query %s /ve /t REG_SZ", key_path.c_str());
    ExecuteProcessAndVerifyOutput(command_line, expected, true);
}

void CompareRegContents(const std::wstring& key_path, const std::wstring& value, const std::string& expected) {
    wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
    swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"powershell.exe -command \"(Get-ItemProperty '%s' -Name %s).%s\"", key_path.c_str(), value.c_str(), value.c_str());
    ExecuteProcessAndVerifyOutput(command_line, expected);
}

void CompareRegBinaryContents(const std::wstring& key_path, const std::wstring& value, const std::string& expected_hex_str) {
    wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
    swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"powershell.exe -command \"$ret = ''; (get-itemproperty '%s' -Name %s).%s | %%{ $ret = $ret + ('{0:X2}' -f $_)}; $ret\"", key_path.c_str(), value.c_str(), value.c_str());
    ExecuteProcessAndVerifyOutput(command_line, expected_hex_str);
}

void CreateRegKey(const std::wstring& key_path) {
    wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
    swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"reg.exe add %s /f", key_path.c_str());
    DWORD error_code;
    DWORD exit_code;
    std::vector<char> output = execute::ExecuteProcess(command_line, 5, &error_code, &exit_code); 
    ASSERT_EQ(error_code, ERROR_SUCCESS);
    std::string result(output.begin(), output.end());
    std::cout << "Output for deleting reg key " << string_util::wstring_to_string(key_path) << ": " << result << std::endl;
    ASSERT_EQ(exit_code, 0);
}

void DeleteRegKey(const std::wstring& key_path) {
    wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
    swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"reg.exe delete %s /f", key_path.c_str());
    DWORD error_code;
    DWORD exit_code;
    std::vector<char> output = execute::ExecuteProcess(command_line, 5, &error_code, &exit_code); 
    ASSERT_EQ(error_code, ERROR_SUCCESS);
    std::string result(output.begin(), output.end());
    std::cout << "Output for deleting reg key " << string_util::wstring_to_string(key_path) << ": " << result << std::endl;
}

void CreateDummyFile(const std::string& path, const std::string& contents) {
    std::ofstream ofs(path);
    for (int i = 0; i < 20; i++) {
        ofs << contents;
    }
    ofs.close();
}

void CompareFileContents(const std::wstring& path, const std::vector<char>& want) {
    std::ifstream in_file(path, std::ios::binary | std::ios::ate);
    size_t len = in_file.tellg();
    in_file.seekg(0, in_file.beg);
    EXPECT_TRUE(len > 0);
    EXPECT_EQ(want.size(), len);
    std::vector<char> buffer(len);
    in_file.read(buffer.data(), len);
    EXPECT_EQ(in_file.gcount(), len);
    EXPECT_EQ(buffer, want);
}

int GetNumProcesses(LPCWSTR proc_name) {
    wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
    swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"powershell.exe -command \"(get-process %s -erroraction silentlycontinue).count\"", proc_name);
    DWORD error_code;
    DWORD exit_code;
    std::vector<char> output = execute::ExecuteProcess(command_line, 5, &error_code, &exit_code); 
    EXPECT_EQ(error_code, ERROR_SUCCESS);
    EXPECT_EQ(exit_code, 0);
    if (error_code != ERROR_SUCCESS || exit_code != 0) {
        return -1;
    }
    return std::stoi(std::string(output.begin(), output.end()));
}

void KillProcess(LPCWSTR proc_name) {
    wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
    swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"powershell.exe -command \"$proc = get-process %s -erroraction silentlycontinue; if ($proc) { $proc | stop-process -force } else { exit 0; }\"", proc_name);
    DWORD error_code;
    DWORD exit_code;
    std::vector<char> output = execute::ExecuteProcess(command_line, 5, &error_code, &exit_code); 
    ASSERT_EQ(error_code, ERROR_SUCCESS);
    ASSERT_EQ(exit_code, 0);
    ASSERT_EQ(GetNumProcesses(proc_name), 0);
}

} // namespace