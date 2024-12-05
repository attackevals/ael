#include "networking.hpp"
#include "HTTPRequest.hpp"
#include "hash_string.hpp"
#include <fstream>
#include <filesystem>
#include <gtest/gtest.h>

TEST(NetworkUtilities, CreateDataBlobFromStringSizeRecoverSize) {

    std::string test_data = "This is some test data.";
    std::vector<unsigned char> bytes = network::utilities::CreateDataBlobFromString(test_data);

    uint32_t length = 0;
    length |= (bytes[0] << 24);
    length |= (bytes[1] << 16);
    length |= (bytes[2] << 8);
    length |= bytes[3];

    EXPECT_EQ(length, test_data.size());
}

TEST(NetworkUtilities, CreateDataBlobFromStringRecoverString) {

    std::string test_data = "This is some test data.";
    std::vector<unsigned char> bytes = network::utilities::CreateDataBlobFromString(test_data);
    std::string recovered(bytes.begin() + 4, bytes.end());

    EXPECT_STREQ(recovered.c_str(), test_data.c_str());
}

TEST(NetworkUtilities, CreateDataBlobFromVectorRecoverSize) {

    std::vector<unsigned char> test_data = { 0xff, 0x01, 0x02, 0xfc, 0x90, 0x01, 0xdf };
    std::vector<unsigned char> bytes = network::utilities::CreateDataBlobFromVector(test_data);

    uint32_t length = 0;
    length |= (bytes[0] << 24);
    length |= (bytes[1] << 16);
    length |= (bytes[2] << 8);
    length |= bytes[3];


    EXPECT_EQ(length, test_data.size());
}

TEST(NetworkUtilities, CreateDataBlobFromVectorRecoverData) {

    std::vector<unsigned char> test_data = { 0xff, 0x01, 0x02, 0xfc, 0x90, 0x01, 0xdf };
    std::vector<unsigned char> bytes = network::utilities::CreateDataBlobFromVector(test_data);


    size_t test_data_size = test_data.size();
    unsigned char test_data_arr[7];

    for (size_t i = 0; i < test_data_size; ++i) {
        test_data_arr[i] = bytes[i + 4];
    }

    EXPECT_EQ(test_data_arr[0], test_data.data()[0]);
}