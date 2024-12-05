#include "util.hpp"
#include <filesystem>
#include <gtest/gtest.h>

class ConfigFileTest : public ::testing::Test {

protected:

    void CreateConfigFile() {
        std::ofstream ofs("sb.conf");
        ofs << "OTMxPzsxPzcuPS4wIzE6OTQ0OTsxPzcuPS4wIzE6NTAwPQ==";
        ofs.close();
    }

    virtual void SetUp() {
        settings::config_path = "sb.conf";
        CreateConfigFile();
    }

    virtual void TearDown() {
        std::filesystem::remove_all("sb.conf");
    }
};


class ConfigFileMalformedTest : public ::testing::Test {

protected:

    void CreateConfigFile() {
        std::ofstream ofs("sb.conf");
        ofs << "OTMyPDs=";
        ofs.close();
    }

    virtual void SetUp() {
        CreateConfigFile();
    }

    virtual void TearDown() {
        std::filesystem::remove_all("sb.conf");
    }
};

TEST_F(ConfigFileTest, ParseConfigMalId) {

    util::ReadParseConfig();

    std::string expected = "4312";
    EXPECT_STREQ(settings::malware_id.c_str(), expected.c_str());
}

TEST_F(ConfigFileTest, ParseConfigC2) {

    util::ReadParseConfig();

    std::string expected = "127.0.0.1:4444";
    EXPECT_STREQ(settings::C2_SERVERS.at(0).c_str(), expected.c_str());
}

TEST(UtilTests, ParseConfigNX) {

    EXPECT_FALSE(util::ReadParseConfig());
}

TEST_F(ConfigFileMalformedTest, ParseMalformedConfig) {

    EXPECT_FALSE(util::ReadParseConfig());
}

TEST(UtilTests, GetSystemComputerNameSuccess) {

    std::string computerName = recon::GetSystemComputerName();
    std::string noteq = "Unable to retrieve computer name";

    EXPECT_NE(computerName.c_str(), noteq.c_str());
}


TEST(UtilTests, GetDomainName) {

    std::string domainName = recon::GetSystemDomainName();
    std::string noteq = "Unable to get domain name";

    EXPECT_NE(domainName.c_str(), noteq.c_str());
}