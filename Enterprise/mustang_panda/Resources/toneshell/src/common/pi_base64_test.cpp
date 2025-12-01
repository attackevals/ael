#include <tuple>
#include <vector>
#include <gtest/gtest.h>

#include "pi_base64.hpp"

std::vector<unsigned char> StringToUnsignedCharVec(std::string input) {
    return std::vector<unsigned char>(input.begin(), input.end());
}

TEST(PI_EncodingTests, TestBase64) {
    std::vector<unsigned char> buf;
    buf.resize(1024);
    std::vector<std::tuple<std::vector<unsigned char>, std::string>> test_elements{
        std::tuple<std::vector<unsigned char>, std::string>{
            std::vector<unsigned char>{},
            ""
        },
        std::tuple<std::vector<unsigned char>, std::string>{
            std::vector<unsigned char>{0x0},
            "AA=="
        },
        std::tuple<std::vector<unsigned char>, std::string>{
            std::vector<unsigned char>{0x0, 0x1},
            "AAE="
        },
        std::tuple<std::vector<unsigned char>, std::string>{
            std::vector<unsigned char>{0x0, 0x1, 0x2},
            "AAEC"
        },
        std::tuple<std::vector<unsigned char>, std::string>{
            std::vector<unsigned char>{0x0, 0x1, 0x2, 0x3},
            "AAECAw=="
        },
        std::tuple<std::vector<unsigned char>, std::string>{
            std::vector<unsigned char>{
                0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
                0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
                0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
                0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
                0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
                0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
                0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
                0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf
            },
            "AAECAwQFBgcICQoLDA0ODwABAgMEBQYHCAkKCwwNDg8AAQIDBAUGBwgJCgsMDQ4PAAECAwQFBgcICQoLDA0ODwABAgMEBQYHCAkKCwwNDg8AAQIDBAUGBwgJCgsMDQ4PAAECAwQFBgcICQoLDA0ODwABAgMEBQYHCAkKCwwNDg8="
        },
        std::tuple<std::vector<unsigned char>, std::string>{
            StringToUnsignedCharVec("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."),
            "TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2NpbmcgZWxpdCwgc2VkIGRvIGVpdXNtb2QgdGVtcG9yIGluY2lkaWR1bnQgdXQgbGFib3JlIGV0IGRvbG9yZSBtYWduYSBhbGlxdWEuIFV0IGVuaW0gYWQgbWluaW0gdmVuaWFtLCBxdWlzIG5vc3RydWQgZXhlcmNpdGF0aW9uIHVsbGFtY28gbGFib3JpcyBuaXNpIHV0IGFsaXF1aXAgZXggZWEgY29tbW9kbyBjb25zZXF1YXQuIER1aXMgYXV0ZSBpcnVyZSBkb2xvciBpbiByZXByZWhlbmRlcml0IGluIHZvbHVwdGF0ZSB2ZWxpdCBlc3NlIGNpbGx1bSBkb2xvcmUgZXUgZnVnaWF0IG51bGxhIHBhcmlhdHVyLiBFeGNlcHRldXIgc2ludCBvY2NhZWNhdCBjdXBpZGF0YXQgbm9uIHByb2lkZW50LCBzdW50IGluIGN1bHBhIHF1aSBvZmZpY2lhIGRlc2VydW50IG1vbGxpdCBhbmltIGlkIGVzdCBsYWJvcnVtLg=="
        }
    };

    // Test bad arguments
    size_t dst_size = 0;
    ASSERT_FALSE(PI_Base64Encode((unsigned char*)"test", 4, NULL, NULL));
    ASSERT_FALSE(PI_Base64Encode((unsigned char*)"test", 4, buf.data(), NULL));
    ASSERT_FALSE(PI_Base64Encode((unsigned char*)"test", 4, buf.data(), &dst_size));

    for (auto test_suite : test_elements) {
        dst_size = buf.size();
        unsigned char* input = std::get<0>(test_suite).data();
        size_t input_size = std::get<0>(test_suite).size();

        ASSERT_TRUE(PI_Base64Encode(input, input_size, buf.data(), &dst_size));
        std::string want = std::get<1>(test_suite);
        ASSERT_EQ(dst_size, want.size());
        std::string result((char*)(buf.data()), dst_size);
        ASSERT_EQ(result, want);
    }
}
