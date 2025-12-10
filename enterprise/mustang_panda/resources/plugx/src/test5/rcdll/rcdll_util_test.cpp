#include "rcdll_util.hpp"
#include <gtest/gtest.h>

TEST(StubUtilTests, TestGetExecutableNameLength) {
    EXPECT_EQ(GetExecutableNameLength(std::wstring(L"executablenospaces.exe")), 22);
    EXPECT_EQ(GetExecutableNameLength(std::wstring(L"executablenospaces.exe -with -flags and args")), 22);
    EXPECT_EQ(GetExecutableNameLength(std::wstring(L"'C:\\single quotes with spaces\\binary.exe'")), 41);
    EXPECT_EQ(GetExecutableNameLength(std::wstring(L"'C:\\single quotes with spaces\\binary.exe' -with -flags and args")), 41);
    EXPECT_EQ(GetExecutableNameLength(std::wstring(L"\"C:\\double quotes with spaces\\binary.exe\"")), 41);
    EXPECT_EQ(GetExecutableNameLength(std::wstring(L"\"C:\\double quotes with spaces\\binary.exe\" -with -flags and args")), 41);
}

TEST(StubUtilTests, TestCommandLineHasArg) {
    EXPECT_TRUE(CommandLineHasArg(std::wstring(L"gup.exe test"), L"test"));
    EXPECT_TRUE(CommandLineHasArg(std::wstring(L"gup.exe test a"), L"test"));
    EXPECT_TRUE(CommandLineHasArg(std::wstring(L"gup.exe a test"), L"test"));
    EXPECT_TRUE(CommandLineHasArg(std::wstring(L"gup.exe -i"), L"-i"));
    EXPECT_TRUE(CommandLineHasArg(std::wstring(L"gup.exe -a -b -c"), L"-a"));
    EXPECT_TRUE(CommandLineHasArg(std::wstring(L"gup.exe -a -b -c"), L"-b"));
    EXPECT_TRUE(CommandLineHasArg(std::wstring(L"gup.exe -a -b -c"), L"-c"));
    EXPECT_TRUE(CommandLineHasArg(std::wstring(L"gup.exe -a   -b   -c  "), L"-a"));
    EXPECT_TRUE(CommandLineHasArg(std::wstring(L"gup.exe -a   -b   -c  "), L"-b"));
    EXPECT_TRUE(CommandLineHasArg(std::wstring(L"gup.exe -a   -b   -c  "), L"-c"));
    EXPECT_FALSE(CommandLineHasArg(std::wstring(L"gup.exe -a -b -c"), L"-d"));
    EXPECT_FALSE(CommandLineHasArg(std::wstring(L"gup.exe -a -b -c"), L"-i"));
    EXPECT_FALSE(CommandLineHasArg(std::wstring(L"gup.exe -a -b -c"), L"gup.exe"));
    EXPECT_FALSE(CommandLineHasArg(std::wstring(L"gup.exe"), L"-i"));
    EXPECT_FALSE(CommandLineHasArg(std::wstring(L"gup.exe "), L"-i"));
    EXPECT_FALSE(CommandLineHasArg(std::wstring(L"gup.exe -a -b -c"), L"-charlie"));
    EXPECT_FALSE(CommandLineHasArg(std::wstring(L"gup.exe -a -b -charlie"), L"-c"));
}
