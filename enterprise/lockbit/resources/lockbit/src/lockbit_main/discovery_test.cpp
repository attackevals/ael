#include "discovery.hpp"
#include "execute.hpp"
#include "util/string_util.hpp"
#include <gtest/gtest.h>


TEST(DiscoveryTests, TestGetUsername) {
    DWORD error_code = ERROR_SUCCESS;
    DWORD exit_code;
    wchar_t command[] = L"whoami.exe";
    std::vector<char> output = execute::ExecuteProcess(command, 10, &error_code, &exit_code);
    ASSERT_EQ(error_code, ERROR_SUCCESS);
    ASSERT_EQ(exit_code, ERROR_SUCCESS);
    std::string want(output.begin(), output.end());
    std::string result = string_util::to_lower(discovery::GetUsername());
    ASSERT_EQ(result, string_util::to_lower(string_util::rtrim(want)));
}

TEST(DiscoveryTests, TestGetHostname) {
    DWORD error_code = ERROR_SUCCESS;
    DWORD exit_code;
    wchar_t command[] = L"hostname.exe";
    std::vector<char> output = execute::ExecuteProcess(command, 10, &error_code, &exit_code);
    ASSERT_EQ(error_code, ERROR_SUCCESS);
    ASSERT_EQ(exit_code, ERROR_SUCCESS);
    std::string want(output.begin(), output.end());
    ASSERT_EQ(string_util::to_lower(discovery::GetHostname()), string_util::to_lower(string_util::rtrim(want)));
}

TEST(DiscoveryTests, TestGetDomain) {
    DWORD error_code = ERROR_SUCCESS;
    DWORD exit_code;
    wchar_t command[] = L"powershell.exe -command \"$output=systeminfo.exe; $output | Select-String -Pattern '^Domain:\\s+(\\S+)$' | %{ $_.Matches.Groups[1].Value };\"";
    std::vector<char> output = execute::ExecuteProcess(command, 10, &error_code, &exit_code);
    ASSERT_EQ(error_code, ERROR_SUCCESS);
    ASSERT_EQ(exit_code, ERROR_SUCCESS);
    std::string want(output.begin(), output.end());
    if (string_util::rtrim(want) == discovery::GetHostname()) {
        want = "No domain";
    }
    ASSERT_EQ(string_util::to_lower(discovery::GetDomain()), string_util::to_lower(string_util::rtrim(want)));
}

TEST(DiscoveryTests, TestGetOsArch) {
    DWORD error_code = ERROR_SUCCESS;
    DWORD exit_code;
    wchar_t command[] = L"cmd.exe /c echo %PROCESSOR_ARCHITECTURE%";
    std::vector<char> output = execute::ExecuteProcess(command, 10, &error_code, &exit_code);
    ASSERT_EQ(error_code, ERROR_SUCCESS);
    ASSERT_EQ(exit_code, ERROR_SUCCESS);
    std::string want(output.begin(), output.end());
    ASSERT_EQ(discovery::GetOsArchitecture(), string_util::rtrim(want));
}

TEST(DiscoveryTests, TestGetOsVersion) {
    DWORD error_code = ERROR_SUCCESS;
    DWORD exit_code;
    wchar_t command[] = L"powershell.exe -command \"$version = [System.Environment]::OSVersion.Version; [string]::Format('{0}.{1}.{2}', $version.Major, $version.Minor,$version.Build)\"";
    std::vector<char> output = execute::ExecuteProcess(command, 10, &error_code, &exit_code);
    ASSERT_EQ(error_code, ERROR_SUCCESS);
    ASSERT_EQ(exit_code, ERROR_SUCCESS);
    std::string want(output.begin(), output.end());
    ASSERT_EQ(discovery::GetOsVersionString(), string_util::rtrim(want));
}
