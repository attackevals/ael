#include <gtest/gtest.h>
#include <filesystem>

#include "guardrails.hpp"

TEST(CoreTests, TestMutexCreated) {
    bool result = Guardrails::CheckCreateMutex();
    EXPECT_TRUE(result);
    EXPECT_TRUE(Guardrails::h_mutex != NULL);

    Guardrails::ReleaseMutex();
}

TEST(CoreTests, TestMutexCreatedAlreadyExists) {
    Guardrails::CheckCreateMutex();
    bool result = Guardrails::CheckCreateMutex();
    EXPECT_FALSE(result);

    Guardrails::ReleaseMutex();
}