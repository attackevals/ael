#include "gflagsui_util.hpp"
#include <gtest/gtest.h>

TEST(GflagsuiUtilTest, TestEnsureSingleton) {
    wchar_t mutex_name[] = L"Global\\TestProt4Mutex129389315";

    // Mutex shouldn't exist yet
    ASSERT_EQ(EnsureSingleton(mutex_name, false), ERROR_SUCCESS);
    ASSERT_EQ(EnsureSingleton(mutex_name, false), ERROR_SUCCESS);
    ASSERT_EQ(EnsureSingleton(mutex_name, false), ERROR_SUCCESS);

    // Try creating mutex
    ASSERT_EQ(EnsureSingleton(mutex_name, true), ERROR_SUCCESS);

    // Subsequent checks should fail
    ASSERT_EQ(EnsureSingleton(mutex_name, true), ERROR_ALREADY_EXISTS);
    ASSERT_EQ(EnsureSingleton(mutex_name, false), ERROR_ALREADY_EXISTS);
    ASSERT_EQ(EnsureSingleton(mutex_name, true), ERROR_ALREADY_EXISTS);
    ASSERT_EQ(EnsureSingleton(mutex_name, false), ERROR_ALREADY_EXISTS);
}
