#include <gtest/gtest.h>
#include <filesystem>

#include "core.hpp"

const char* filename = "Test_Favbug.txt";

TEST(CoreTests, TestFileDropped) {
    bool result = Core::DropFile(filename);
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(filename));

    std::remove(filename);
}

TEST(CoreTests, TestFileDroppedAlreadyExists) {
    Core::DropFile(filename);
    bool result = Core::DropFile(filename);
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(filename));

    std::remove(filename);
}

TEST(CoreTests, TestFileDroppedBadFilename) {
    const char* badFilename = "test<>.txt";
    bool result = Core::DropFile(badFilename);

    EXPECT_FALSE(result);
    EXPECT_FALSE(std::filesystem::exists(badFilename));

    std::remove(badFilename);
}

TEST(CoreTests, TestCmdSuccess) {
    std::string output_data = Core::cmdExecuteCommand("echo test_command");
    std::string test_string = "test_command\r\n";

    EXPECT_STREQ(output_data.c_str(), test_string.c_str());
}

TEST(CoreTests, TestCmdVariableExpansionSuccess) {
    std::string output = Core::cmdExecuteCommand("echo %cd%");

    EXPECT_TRUE(output.find("Resources\\Cl0p") != std::string::npos);
}
