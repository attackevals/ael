#include "utilities.hpp"
#include <gtest/gtest.h>


TEST(RatUtilityTests, GetFileNameFromPathExists) {
    std::string filename = Utilities::GetFileNameFromPath("C:\\Windows\\System32\\ntdll.dll");
    std::string value = "ntdll.dll";

    EXPECT_STREQ(filename.c_str(), value.c_str());
}

TEST(RatUtilityTests, GetFileNameFromPathNoExist) {
    std::string filename = Utilities::GetFileNameFromPath("C:\\Windows\\System32\\ntdll-asdfasdf.dll");
    std::string value = "";

    EXPECT_STREQ(filename.c_str(), value.c_str());
}

TEST(RatUtilityTests, XorStringData) {
    std::string test_data = "XORTHIS";
    std::string xord_test_data = Utilities::XorStringData(test_data);

    std::string xor_decoded = Utilities::XorStringData(xord_test_data);

    EXPECT_STREQ(test_data.c_str(), xor_decoded.c_str());
}