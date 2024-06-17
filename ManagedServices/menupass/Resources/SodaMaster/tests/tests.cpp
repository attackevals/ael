#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include "EncUtils.h"

std::string key_string = "8BD4091D416F6D5D";

CryptoPP::SecByteBlock key(reinterpret_cast<const byte*>(&key_string[0]), key_string.size());
std::string plaintext_data = "test data";
std::string encrypted_data = "w3IhxU/NdhiJhTAGTQ==";

TEST(RC4_ENCRYPTION_TEST, Encryption_Test)
{
	std::string result = RC4EncryptString(plaintext_data, key);

	EXPECT_STREQ(result.c_str(), encrypted_data.c_str());
}

TEST(RC4_DECRYPTION_TEST, Encryption_Test)
{
	std::string result = RC4DecryptString(encrypted_data, key);

	EXPECT_STREQ(plaintext_data.c_str(), result.c_str());
}