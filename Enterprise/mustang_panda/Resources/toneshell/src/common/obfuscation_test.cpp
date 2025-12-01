#include "obfuscation.hpp"
#include <gtest/gtest.h>

TEST(ObfuscationTest, TestXorStrings) {
    ASSERT_STREQ(XorString("Hello world! 123"), "Hello world! 123");
    ASSERT_STREQ(XorStringW(L"Hello world! 123"), L"Hello world! 123");
    ASSERT_STREQ(XorString("Hello world!\t\n 123"), "Hello world!\t\n 123");
    ASSERT_STREQ(XorStringW(L"Hello world!\t\n 123"), L"Hello world!\t\n 123");
    ASSERT_STRNE(XorString("Hello world! 123"), "123 Hello world!");
    ASSERT_STRNE(XorStringW(L"Hello world! 123"), L"123 Hello world!");
}
