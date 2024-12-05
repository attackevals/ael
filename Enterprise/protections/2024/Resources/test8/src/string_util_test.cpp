#include "string_util.hpp"
#include <gtest/gtest.h>

TEST(UtilTests, TestConverstion) {
    EXPECT_EQ(string_util::wstring_to_string(L"hello world"), std::string("hello world"));
    EXPECT_EQ(string_util::string_to_wstring("hello world"), std::wstring(L"hello world"));
}

TEST(UtilTests, TestToHexString) {
    EXPECT_EQ(string_util::hresult_to_hex_str(0x357137), std::string("0x357137"));
    EXPECT_EQ(string_util::hresult_to_hex_str(0), std::string("0x0"));
}