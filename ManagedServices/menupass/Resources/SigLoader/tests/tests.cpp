#include <gtest/gtest.h>
#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>

#include "layer1_loader.h"

TEST(AES_TEST, Encryption_Test)
{
	// "test data" encrypted with AES CBC
	std::vector<unsigned char> aes_data = { 0x91, 0x7f, 0x23, 0x7f, 0xde, 0x75, 0x83, 0xa6, 0x03, 0xc2, 0xa5, 0xd9, 0x1a, 0x22, 0x73, 0xe9 };
	std::string tmp = "test data";
	std::vector<unsigned char> test(tmp.data(), tmp.data() + tmp.length());

	std::vector<unsigned char> result = load_data::AES(aes_data);
	

	EXPECT_EQ(result, test);
}

TEST(XOR_TEST, Encryption_Test)
{
	// "test data" in hex (for XOR)
	std::vector<unsigned char> xor_data = { 0xdd, 0xcc, 0xda, 0xdd, 0x89, 0xcd, 0xc8, 0xdd, 0xc8 };
	std::string tmp = "test data";
	std::vector<unsigned char> test(tmp.data(), tmp.data() + tmp.length());

	std::vector<unsigned char> result = load_data::XOR(xor_data);


	EXPECT_EQ(result, test);
}

TEST(DES_TEST, Encryption_Test)
{
	// "test data" encrypted with DES CBC
	std::vector<unsigned char> des_data = { 0x5c, 0xce, 0xea, 0x58, 0x76, 0xa2, 0xf2, 0x00, 0x40, 0x7e, 0xd7, 0x08, 0xba, 0x8d, 0xf1, 0x74 };
	std::string tmp = "test data";
	std::vector<unsigned char> test(tmp.data(), tmp.data() + tmp.length());

	std::vector<unsigned char> result = load_data::DES(des_data);


	EXPECT_EQ(result, test);
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}