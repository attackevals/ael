#include "core.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <gtest/gtest.h>
#include <thread>

class StratofearCoreTests : public testing::Test {
    protected:
        void SetUp() override {
            std::string path = CONFIG_PATH + "config";
            std::ofstream testConfig;
            testConfig.open (path);
            //insert values into the file at offsets
            testConfig.seekp(410, std::ios::beg);
            testConfig.write("AB", sizeof("AB"));
            testConfig.seekp(526, std::ios::beg);
            testConfig.write("https://1.2.3.4:80", strlen("https://1.2.3.4:80"));
            testConfig.close();
            // Encrypt file so it can be decrypted by LoadConfig()
            std::string key = CONFIG_ENC_KEY;
            std::string encryptedPath = CONFIG_PATH + CONFIG_FILENAME;
            std::string command = "openssl enc -aes-128-cbc -pass pass:" + key + " -in " + path + " -out " + encryptedPath;
            int result = system(command.c_str());
        }

        void TearDown() override {
            std::string pathEncrypted = CONFIG_PATH + CONFIG_FILENAME;
            std::string pathDecrypted = CONFIG_PATH + CONFIG_FILENAME + ".lock";

            std::filesystem::remove(pathEncrypted);
            std::filesystem::remove(pathDecrypted);
        }
};

class StratofearCoreNoSetupTests : public testing::Test {

};

TEST_F(StratofearCoreTests, SetUpTest) {
    std::string path = CONFIG_PATH + CONFIG_FILENAME;
    const std::filesystem::path config{path};
    EXPECT_TRUE(std::filesystem::exists(config));
}

TEST_F(StratofearCoreTests, LoadConfigSuccessTest) {
    // Create the file
    std::string path = CONFIG_PATH + CONFIG_FILENAME;
    const std::filesystem::path config{path};
    Configuration stratofearConfig = LoadConfig();
    EXPECT_TRUE(stratofearConfig.configured);
}

TEST_F(StratofearCoreNoSetupTests, LoadConfigFailureTest) {
    Configuration stratofearConfig = LoadConfig();
    EXPECT_FALSE(stratofearConfig.configured);
}

TEST_F(StratofearCoreNoSetupTests, LoadConfigEmptyTest) {
    std::ofstream testConfig;
    std::string path = CONFIG_PATH + "config";
    testConfig.open (path);
    testConfig << "";
    testConfig.close();
    Configuration stratofearConfig = LoadConfig();
    EXPECT_FALSE(stratofearConfig.configured);
}

TEST_F(StratofearCoreNoSetupTests, LoadConfigMissingC2DomainTest) {
    std::ofstream testConfig;
    std::string path = CONFIG_PATH + "config";
    testConfig.open (path);
    testConfig.seekp(410, std::ios::beg);
    testConfig.write("TES", sizeof("TES"));
    testConfig.seekp(416, std::ios::beg);
    testConfig.write("TD", sizeof("TD"));
    testConfig.close();
    // Encrypt file so it can be decrypted by LoadConfig()
    std::string key = CONFIG_ENC_KEY;
    std::string encryptedPath = CONFIG_PATH + CONFIG_FILENAME;
    std::string command = "openssl enc -aes-128-cbc -pass pass:" + key + " -in " + path + " -out " + encryptedPath;
    int result = system(command.c_str());
    Configuration stratofearConfig = LoadConfig();
    EXPECT_FALSE(stratofearConfig.configured);
}

TEST_F(StratofearCoreNoSetupTests, LoadConfigMissingC2UUIDTest) {
    std::ofstream testConfig;
    std::string path = CONFIG_PATH + "config";
    testConfig.open (path);
    testConfig.seekp(526, std::ios::beg);
    testConfig.write("https://1.2.3.4:80", strlen("https://1.2.3.4:80"));
    testConfig.close();
    // Encrypt file so it can be decrypted by LoadConfig()
    std::string key = CONFIG_ENC_KEY;
    std::string encryptedPath = CONFIG_PATH + CONFIG_FILENAME;
    std::string command = "openssl enc -aes-128-cbc -pass pass:" + key + " -in " + path + " -out " + encryptedPath;
    int result = system(command.c_str());
    Configuration stratofearConfig = LoadConfig();
    EXPECT_FALSE(stratofearConfig.configured);
}

TEST_F(StratofearCoreNoSetupTests, DlopenDlsymSuccessTest) {
    std::filesystem::path filepath = std::filesystem::current_path();
    filepath += "/../module/test_module/Debug/libtest_module.dylib";
    if(!std::filesystem::exists(filepath)){
        filepath = std::filesystem::current_path();
        filepath += "/../module/test_module/Release/libtest_module.dylib";
    }
    const char* testArray = executeModule(filepath);
    std::string testString(testArray);
    EXPECT_TRUE(testString.contains("Success"));
}

TEST_F(StratofearCoreNoSetupTests, DlopenDlsymFailureTest) {
    std::filesystem::path filepath = std::filesystem::current_path();
    filepath += "/../module/test_module/Debug/nofile.dylib";
    if(!std::filesystem::exists(filepath)){
        filepath = std::filesystem::current_path();
        filepath += "/../module/test_module/Release/nofile.dylib";
    }
    const char* testArray = executeModule(filepath);
    std::string testString(testArray);
    EXPECT_FALSE(testString.contains("Success"));
}

TEST_F(StratofearCoreNoSetupTests, executeModuleFirstEnvVarSuccess) {
    // Check & remove environment variables
    if (getenv("fn") != nullptr) {
        unsetenv("fn");
    }  
    std::filesystem::path filepath = std::filesystem::current_path();
    filepath += "/../module/test_module/Debug/libtest_module.dylib";
    if(!std::filesystem::exists(filepath)){
        filepath = std::filesystem::current_path();
        filepath += "/../module/test_module/Release/libtest_module.dylib";
    }
    const char* result = executeModule(filepath);
    char* envFN = std::getenv("fn");
    std::string testString(envFN);
    EXPECT_TRUE(testString.contains("test/to/path"));
}

TEST_F(StratofearCoreNoSetupTests, executeModuleNoDylibFile) {
    // Check & remove environment variables
    if (getenv("fn") != nullptr) {
        unsetenv("fn");
    }  
    std::filesystem::path filepath = std::filesystem::current_path();
    filepath += "/../module/test_module/Debug/nofile.dylib";
    if(!std::filesystem::exists(filepath)){
        filepath = std::filesystem::current_path();
        filepath += "/../module/test_module/Release/nofile.dylib";
    }
    const char* result = executeModule(filepath);
    char* envFN = std::getenv("fn");
    EXPECT_EQ(envFN, nullptr);
}

TEST_F(StratofearCoreNoSetupTests, executeCollectSystemInfo) {
    std::string result = collectSystemInfo();
    EXPECT_NE(result.c_str(), nullptr);
}

TEST_F(StratofearCoreNoSetupTests, executeMonitorDevicesSuccess) {
    // Start the monitor
    // `/Library/Fonts/AppleSDGothicNeo.ttc.1` is hardcoded in core.cpp _logDeviceEvent
    std::thread monitorThread(monitorDevices, "/Library/Fonts");
    monitorThread.detach();
    // Create a file to trigger event
    const char* testFile = "/Library/Fonts/test_file.txt";
    std::ofstream outfile(testFile);
    outfile << "Test content\n";
    outfile.close();
    // Allow time to create log file and detect the event
    sleep(2);
    const char* logFile = "/Library/Fonts/AppleSDGothicNeo.ttc.1";
    std::ifstream infile(logFile);
    bool LogFileExists = infile.good();
    infile.close();
    std::remove(testFile);
    std::remove(logFile);
    ASSERT_TRUE(LogFileExists);
}