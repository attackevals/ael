#include "nt.hpp"
#include <gtest/gtest.h>


TEST(NTFunctionTests, NtSetInformationProcessHarderrorMode) {
    errorhandling::SetHardErrorMode();

    ASSERT_EQ(GetErrorMode(), 6);
}