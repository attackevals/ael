#include "core.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <gtest/gtest.h>


class FullhouseCoreTests : public testing::Test {
    protected:
        void SetUp() override {
            std::ofstream testConfig;
            testConfig.open ("com.docker.sock.lock");
            testConfig << "http://1.2.3.4:80" << std::endl;
            testConfig << "TESTUUID";
            testConfig.close();

            std::ofstream testFile;
            testFile.open("/tmp/testfile");
            testFile << "test data";
            testFile.close();
        }

        void TearDown() override {
            std::remove("com.docker.sock.lock");
            std::remove("/tmp/testfile");
        }
};

class FullhouseCoreNoSetupTests : public testing::Test {

};

TEST_F(FullhouseCoreTests, SetUpTest) {
    const std::filesystem::path config{"com.docker.sock.lock"};
    EXPECT_TRUE(std::filesystem::exists(config));
}

TEST_F(FullhouseCoreTests, LoadConfigSuccessTest) {
    bool loadedConfig = LoadConfig();
    EXPECT_TRUE(loadedConfig);
}

TEST_F(FullhouseCoreNoSetupTests, LoadConfigFailureTest) {
    bool loadedConfig = LoadConfig();
    EXPECT_FALSE(loadedConfig);
}

TEST_F(FullhouseCoreTests, LoadConfigEmptyTest) {
    std::ofstream testConfig;
    testConfig.open ("com.docker.sock.lock");
    testConfig << "";
    testConfig.close();
    bool loadedConfig = LoadConfig();
    EXPECT_FALSE(loadedConfig);
}

TEST_F(FullhouseCoreTests, LoadConfigNotTwoLinesTest) {
    std::ofstream testConfig;
    testConfig.open ("com.docker.sock.lock");
    testConfig << "http://1.2.3.4:80";
    testConfig.close();
    bool loadedConfig = LoadConfig();
    EXPECT_FALSE(loadedConfig);
}

TEST_F(FullhouseCoreNoSetupTests, GetUserNameTest) {
    std::string result(GetUserName());
    ASSERT_NE("", result);
}

TEST_F(FullhouseCoreNoSetupTests, GetHostNameTest) {
    std::string result(GetHostName());
    ASSERT_NE("", result);
}

TEST_F(FullhouseCoreNoSetupTests, GetPIDTest) {
    ASSERT_LT(0, GetPID());
}

TEST_F(FullhouseCoreTests, ExecuteCmdSTDOUTTest) {
    std::string cmd = "cat com.docker.sock.lock";
    std::string expect_str = "http://1.2.3.4:80\nTESTUUID";
    const char * expect = expect_str.c_str();
    ASSERT_STREQ(expect, ExecuteCmd(cmd.c_str()));
}

TEST_F(FullhouseCoreNoSetupTests, ExecuteCmdSTDERRTest) {
    std::string cmd = "cat com.docker.sock.lock";
    std::string expect_str = "cat: com.docker.sock.lock: No such file or directory\n";
    const char * expect = expect_str.c_str();
    ASSERT_STREQ(expect, ExecuteCmd(cmd.c_str()));
}

TEST_F(FullhouseCoreNoSetupTests, ExecuteCmdEmptySTDOUTTest) {
    std::string cmd = "echo \"hello\" > testfile";
    std::string expect_str = "";
    const char * expect = expect_str.c_str();
    ASSERT_STREQ(expect, ExecuteCmd(cmd.c_str()));

    std::remove("testfile");
}

TEST_F(FullhouseCoreNoSetupTests, DownloadFileSuccessTest) {
    std::vector<unsigned char> download_file_data = { 0x74,0x65,0x73,0x74,0x20,0x66,0x69,0x6c,0x65,0x20,0x63,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x0a };
    std::string test_path = "testdownloadfile.txt";

    EXPECT_TRUE(DownloadFile(&download_file_data[0], download_file_data.size(), test_path.c_str()));

    std::ifstream download_file(test_path, std::ifstream::in);
    std::stringstream buffer;
    buffer << download_file.rdbuf();
    download_file.close();

    EXPECT_EQ("test file content\n", buffer.str());

    std::remove("testdownloadfile.txt");

}

TEST_F(FullhouseCoreNoSetupTests, DownloadFileFailTest) {
    std::vector<unsigned char> download_file_data = { 0x74,0x65,0x73,0x74,0x20,0x66,0x69,0x6c,0x65,0x20,0x63,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x0a };
    std::string test_path = "/nonexistent_path/testdownloadfile.txt";

    EXPECT_FALSE(DownloadFile(&download_file_data[0], download_file_data.size(), test_path.c_str()));
}

TEST_F(FullhouseCoreTests, UploadFileSuccessTest) {
    std::string expected = "dGVzdCBkYXRh";      // "test data"

    std::string actual(UploadFile("/tmp/testfile"));

    EXPECT_EQ(actual, expected);
}

TEST_F(FullhouseCoreNoSetupTests, UploadFileNotExistsTest) {
    char * actual = UploadFile("/tmp/testfile");

    EXPECT_TRUE(!actual);
}

TEST_F(FullhouseCoreNoSetupTests, UploadFilePermissionDeniedTest) {
    char * actual = UploadFile("/var/db/dslocal/nodes/Default/users/root.plist");       //should exist on every MacOS

    EXPECT_TRUE(!actual);
}