#include "guardrails.hpp"
#include <gtest/gtest.h>

TEST(MutexTests, TestGeneratedMutexName) {
    // Generated using https://www.convertcase.com/hashing/djb-hash-calculator
    EXPECT_EQ(guardrails::GenerateMutexName("myhostname"), L"Global\\6cf03e6a");
    EXPECT_EQ(guardrails::GenerateMutexName("K"), L"Global\\0002b5f0");
}

TEST(MutexTests, TestCheckCreateMutex) {
    EXPECT_TRUE(guardrails::CheckCreateMutex("lockbittesthostname"));
    EXPECT_FALSE(guardrails::CheckCreateMutex("lockbittesthostname"));
    guardrails::ReleaseMutex();
}