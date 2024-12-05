#include <gtest/gtest.h>
#include <base64_loader.hpp>

TEST(Base64Tests, Base64) {
	char* lorem = const_cast<char*>("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
	const char* lorem_want = "TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2NpbmcgZWxpdCwgc2VkIGRvIGVpdXNtb2QgdGVtcG9yIGluY2lkaWR1bnQgdXQgbGFib3JlIGV0IGRvbG9yZSBtYWduYSBhbGlxdWEuIFV0IGVuaW0gYWQgbWluaW0gdmVuaWFtLCBxdWlzIG5vc3RydWQgZXhlcmNpdGF0aW9uIHVsbGFtY28gbGFib3JpcyBuaXNpIHV0IGFsaXF1aXAgZXggZWEgY29tbW9kbyBjb25zZXF1YXQuIER1aXMgYXV0ZSBpcnVyZSBkb2xvciBpbiByZXByZWhlbmRlcml0IGluIHZvbHVwdGF0ZSB2ZWxpdCBlc3NlIGNpbGx1bSBkb2xvcmUgZXUgZnVnaWF0IG51bGxhIHBhcmlhdHVyLiBFeGNlcHRldXIgc2ludCBvY2NhZWNhdCBjdXBpZGF0YXQgbm9uIHByb2lkZW50LCBzdW50IGluIGN1bHBhIHF1aSBvZmZpY2lhIGRlc2VydW50IG1vbGxpdCBhbmltIGlkIGVzdCBsYWJvcnVtLg==";
	
	// checking len % 3 == 0 and +/-1 char
	char* lorem_25 = const_cast<char*>("Lorem ipsum dolor sit ame");
	const char* lorem_25_want = "TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZQ==";
	char* lorem_24 = const_cast<char*>("Lorem ipsum dolor sit am");
	const char* lorem_24_want = "TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFt";
	char* lorem_23 = const_cast<char*>("Lorem ipsum dolor sit a");
	const char* lorem_23_want = "TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGE=";

	auto encoded = common::base64_encode(std::span<std::byte>(reinterpret_cast<std::byte*>(lorem), strlen(lorem)));
	EXPECT_STREQ(reinterpret_cast<char*>(encoded.data()), lorem_want);
	auto encoded_25 = common::base64_encode(std::span<std::byte>(reinterpret_cast<std::byte*>(lorem_25), strlen(lorem_25)));
	EXPECT_STREQ(reinterpret_cast<char*>(encoded_25.data()), lorem_25_want);
	auto encoded_24 = common::base64_encode(std::span<std::byte>(reinterpret_cast<std::byte*>(lorem_24), strlen(lorem_24)));
	EXPECT_STREQ(reinterpret_cast<char*>(encoded_24.data()), lorem_24_want);
	auto encoded_23 = common::base64_encode(std::span<std::byte>(reinterpret_cast<std::byte*>(lorem_23), strlen(lorem_23)));
	EXPECT_STREQ(reinterpret_cast<char*>(encoded_23.data()), lorem_23_want);
}