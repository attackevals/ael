#include "checks.hpp"
#include <gtest/gtest.h>

TEST(ChecksTests, TestVerifyProcessName) {
    EXPECT_EQ(ERROR_SUCCESS, VerifyProcessName(L"toneshell-test.exe"));
    EXPECT_EQ(ERROR_SUCCESS, VerifyProcessName(L"Toneshell-test.exe"));
    EXPECT_EQ(ERROR_SUCCESS, VerifyProcessName(L"TONESHELL-TEST.EXE"));
    EXPECT_EQ(PROCESS_NAME_MISMATCH, VerifyProcessName(L"toneshell-test1.exe"));
    EXPECT_EQ(PROCESS_NAME_MISMATCH, VerifyProcessName(L"test.exe"));
}
