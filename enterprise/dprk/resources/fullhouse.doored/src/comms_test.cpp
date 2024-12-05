#include "comms.hpp"
#include <gmock/gmock.h>

std::string ok_http_header = "HTTP/1.1 200 OK\r\n"
    "Date: Fri, 09 Feb 2024 16:38:58 GMT\r\n"
    "Content-Length: 6\r\n"
    "Content-Type: text/plain; charset=utf-8\r\n"
    "Connection: close\r\n\r\n";

std::string chunked_http_header = "HTTP/1.1 200 OK\r\n"
    "Date: Fri, 09 Feb 2024 16:38:58 GMT\r\n"
    "Content-Length: 6\r\n"
    "Content-Type: text/plain; charset=utf-8\r\n"
    "Connection: close\r\n"
    "Transfer-Encoding: chunked\r\n\r\n"
    "e74\r\n";

std::string get_response_heartbeat_str = ok_http_header + "PWQvImR8djs=";   //{"id":0}
std::string get_response_cmd_str = ok_http_header + "PWQvImR8d2pmZCc0IWR8ZDEuKScrL2Q7";    // {"id":1, "arg":"whoami"}
std::string get_response_download_str = ok_http_header + "PWQvImR8dGpmZCc0IWR8ZGkyKzZpLiMqKikxKTQqImRqZmQ2Jz8qKSciZHxkLiMqKikxKTQqImQ7";   //{"id":2, "arg":"/tmp/helloworld", "payload":"helloworld"}
std::string get_response_bad_json_str = ok_http_header + "PWQvImR8amZkJzQhZHw7";   //{"id":, "arg":}
std::string test_str = "test file content";
std::string get_response_file_str = ok_http_header + "PWQgLyojGTUvPCNkfHdxamZkIC8qIxkkPzIjNWR8ZCIBEDwiBQQrJxE+Kg8BCDAkKBQqJCgXe2Q7";    // {"file_size":17, "file_bytes":"dGVzdCBmaWxlIGNvbnRlbnQ="}
std::string get_response_chunked_str = chunked_http_header + "PWQgLyojGTUvPCNkfHdxamZkIC8qIxkkPzIjNWR8ZCIBEDwiBQQrJxE+Kg8BCDAkKBQqJCgXe2Q7";    // {"file_size":17, "file_bytes":"dGVzdCBmaWxlIGNvbnRlbnQ="}

std::vector<unsigned char> response_vector;


TEST(FullhouseCommsTests, ParseHTTPResponseHeartbeatTest) {
    response_vector = std::vector<unsigned char>(get_response_heartbeat_str.begin(), get_response_heartbeat_str.end());
    Communication::HTTP::Response parsed_response = Communication::HTTP::ParseHTTPResponse(response_vector, false);

    EXPECT_EQ(parsed_response.protocol, "HTTP/1.1");
    EXPECT_EQ(parsed_response.status, "200 OK");
    EXPECT_EQ(parsed_response.id, 0);
}

TEST(FullhouseCommsTests, ParseHTTPResponseCmdTest) {
    response_vector = std::vector<unsigned char>(get_response_cmd_str.begin(), get_response_cmd_str.end());
    Communication::HTTP::Response parsed_response = Communication::HTTP::ParseHTTPResponse(response_vector, false);

    EXPECT_EQ(parsed_response.protocol, "HTTP/1.1");
    EXPECT_EQ(parsed_response.status, "200 OK");
    EXPECT_EQ(parsed_response.id, 1);
    EXPECT_EQ(parsed_response.arg, "whoami");
}

TEST(FullhouseCommsTests, ParseHTTPResponseDownloadTest) {
    response_vector = std::vector<unsigned char>(get_response_download_str.begin(), get_response_download_str.end());
    Communication::HTTP::Response parsed_response = Communication::HTTP::ParseHTTPResponse(response_vector, false);

    EXPECT_EQ(parsed_response.protocol, "HTTP/1.1");
    EXPECT_EQ(parsed_response.status, "200 OK");
    EXPECT_EQ(parsed_response.id, 2);
    EXPECT_EQ(parsed_response.arg, "/tmp/helloworld");
    EXPECT_EQ(parsed_response.payload, "helloworld");
}

TEST(FullhouseCommsTests, ParseHTTPResponseEmptyTest) {
    std::vector<unsigned char> empty_vector;

    EXPECT_THROW(Communication::HTTP::ParseHTTPResponse(empty_vector, false), std::length_error);

}

TEST(FullhouseCommsTests, ParseHTTPResponseBadJSONTest) {
    response_vector = std::vector<unsigned char>(get_response_bad_json_str.begin(), get_response_bad_json_str.end());

    EXPECT_THROW(Communication::HTTP::ParseHTTPResponse(response_vector, false), nlohmann::json::parse_error);

}

TEST(FullhouseCommsTests, ParseHTTPResponseFileTest) {
    std::vector<unsigned char> file_response_vector = std::vector<unsigned char>(get_response_file_str.begin(), get_response_file_str.end());
    Communication::HTTP::Response parsed_file_response = Communication::HTTP::ParseHTTPResponse(file_response_vector, true);

    ASSERT_THAT(parsed_file_response.file_bytes, testing::ElementsAre(0x74,0x65,0x73,0x74,0x20,0x66,0x69,0x6c,0x65,0x20,0x63,0x6f,0x6e,0x74,0x65,0x6e,0x74)); // "test file content"
    EXPECT_EQ(parsed_file_response.file_size, 17);
    EXPECT_EQ(parsed_file_response.id, -1);
}

TEST(FullhouseCommsTests, ParseHTTPResponseChunkedTest) {
    std::vector<unsigned char> file_response_vector = std::vector<unsigned char>(get_response_file_str.begin(), get_response_file_str.end());
    Communication::HTTP::Response parsed_file_response = Communication::HTTP::ParseHTTPResponse(file_response_vector, true);

    ASSERT_THAT(parsed_file_response.file_bytes, testing::ElementsAre(0x74,0x65,0x73,0x74,0x20,0x66,0x69,0x6c,0x65,0x20,0x63,0x6f,0x6e,0x74,0x65,0x6e,0x74)); // "test file content"
    EXPECT_EQ(parsed_file_response.file_size, 17);
    EXPECT_EQ(parsed_file_response.id, -1);
}

TEST(FullhouseCommsTests, XorCryptToVectorTest) {
    std::vector<unsigned char> null_bytes_vector = { 0xfe,0xed,0x00,0xca,0xfe };
    std::string plaintext(null_bytes_vector.begin(), null_bytes_vector.end());
    std::vector<unsigned char> cipher_vector = Communication::XorCryptToVector(plaintext);

    ASSERT_THAT(cipher_vector, testing::ElementsAre(0xb8,0xab,0x46,0x8c,0xb8));
}

TEST(FullhouseCommsTests, XorCryptToStringTest) {
    std::vector<unsigned char> cipher_vector = { 0x03,0x10,0x07,0x0a,0x15 };    // EVALS ^ F
    EXPECT_EQ(Communication::XorCryptToString(cipher_vector), "EVALS");
}
