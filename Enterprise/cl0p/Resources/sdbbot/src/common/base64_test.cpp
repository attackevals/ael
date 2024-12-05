#include <gtest/gtest.h>
#include "base64.hpp"

TEST(RatBase64Tests, Base64Encode) {

    std::string plain_text = "test_data";
    std::string base64_encoded = base64_encode_string(plain_text);

    EXPECT_EQ(base64_encoded, "dGVzdF9kYXRh");
}

TEST(RatBase64Tests, Base64Decode) {

    std::string plain_text = "test_data";
    std::string base64_encoded = base64_decode_string("dGVzdF9kYXRh");

    EXPECT_EQ(base64_encoded, plain_text);
}