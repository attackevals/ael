#include "registry.hpp"
#include "test_utils.hpp"
#include <gtest/gtest.h>

class RegistryTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        test_utils::CreateRegKey(L"HKCU\\lockbitunittestkey");
    }

    virtual void TearDown() {
        test_utils::DeleteRegKey(L"HKCU\\lockbitunittestkey");
    }
};

TEST_F(RegistryTest, TestWrites) {
    unsigned char data[50] = {
        0x78, 0x88, 0xbf, 0x48, 0x8b, 0x2d, 0x4d, 0xf6, 
        0xf5, 0xb6, 0x7e, 0x61, 0xba, 0x63, 0x1f, 0xd6,
        0x2d, 0xc1, 0x6d, 0x17, 0xcf, 0x8c, 0x9c, 0xca, 
        0x1f, 0x81, 0xb1, 0x54, 0x5f, 0xbe, 0x84, 0x38, 
        0xbb, 0x81, 0x9b, 0x6d, 0xe2, 0x96, 0x75, 0xca, 
        0x77, 0x47, 0x55, 0xf6, 0x68, 0x81, 0x0d, 0x20, 
        0x61, 0x69
    };
    EXPECT_TRUE(registry::WriteRegistry(HKEY_CURRENT_USER, L"lockbitunittestkey\\test", L"stringval", L"my string data"));
    EXPECT_TRUE(registry::WriteRegistry(HKEY_CURRENT_USER, L"lockbitunittestkey\\test", L"dwordval", 20));
    EXPECT_TRUE(registry::WriteRegistry(HKEY_CURRENT_USER, L"lockbitunittestkey\\test", L"binaryval", data, sizeof(data)));
    test_utils::CompareRegContents(L"HKCU:\\lockbitunittestkey\\test", L"stringval", "my string data");
    test_utils::CompareRegContents(L"HKCU:\\lockbitunittestkey\\test", L"dwordval", "20");
    test_utils::CompareRegBinaryContents(L"HKCU:\\lockbitunittestkey\\test", L"binaryval", "7888BF488B2D4DF6F5B67E61BA631FD62DC16D17CF8C9CCA1F81B1545FBE8438BB819B6DE29675CA774755F668810D206169");
}

TEST_F(RegistryTest, TestBinaryRead) {
    unsigned char data[50] = {
        0x78, 0x88, 0xbf, 0x48, 0x8b, 0x2d, 0x4d, 0xf6, 
        0xf5, 0xb6, 0x7e, 0x61, 0xba, 0x63, 0x1f, 0xd6,
        0x2d, 0xc1, 0x6d, 0x17, 0xcf, 0x8c, 0x9c, 0xca, 
        0x1f, 0x81, 0xb1, 0x54, 0x5f, 0xbe, 0x84, 0x38, 
        0xbb, 0x81, 0x9b, 0x6d, 0xe2, 0x96, 0x75, 0xca, 
        0x77, 0x47, 0x55, 0xf6, 0x68, 0x81, 0x0d, 0x20, 
        0x61, 0x69
    };
    EXPECT_TRUE(registry::WriteRegistry(HKEY_CURRENT_USER, L"lockbitunittestkey\\test", L"binaryvalread", data, sizeof(data)));
    std::vector<unsigned char> read_data = registry::ReadBinaryRegistry(HKEY_CURRENT_USER, L"lockbitunittestkey\\test", L"binaryvalread");
    EXPECT_EQ(read_data.size(), sizeof(data));
    EXPECT_EQ(0, memcmp(read_data.data(), data, sizeof(data)));
}