#include "logger.hpp"
#include "util.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#define TEST_LOG_PATH L"plugx_test_log_file.log"

// Sets mock system time value: 2025-01-02T13:14:15.678 (Thursday)
void mock_GetSystemTime(LPSYSTEMTIME lpSystemTime) {
    lpSystemTime->wYear = 2025;
    lpSystemTime->wMonth = 1;
    lpSystemTime->wDayOfWeek = 4;
    lpSystemTime->wDay = 2;
    lpSystemTime->wHour = 13;
    lpSystemTime->wMinute = 14;
    lpSystemTime->wSecond = 15;
    lpSystemTime->wMilliseconds = 678;
}

// Fills buffer with 0x2
NTSTATUS mock_BCryptGenRandom(
    BCRYPT_ALG_HANDLE hAlgorithm,
    PUCHAR            pbBuffer,
    ULONG             cbBuffer,
    ULONG             dwFlags
) {
    for (ULONG i = 0; i < cbBuffer; i++) {
        pbBuffer[i] = 0x2;
    }
    return STATUS_SUCCESS;
}

class LoggerTest : public ::testing::Test {
protected:
    logger_ctx mock_ctx;
    shared_func_pointers mock_logger_fp = {
        GetLastError,
        CreateFileW,
        CloseHandle,
        _vsnprintf,
        _snprintf,
        mock_GetSystemTime,
        SetFilePointer,
        HeapAlloc,
        HeapCreate,
        HeapDestroy,
        HeapFree,
        WriteFile,
        mock_BCryptGenRandom
    };

    std::string GetLogFileContents() {
        std::ifstream log_file(TEST_LOG_PATH);
        std::stringstream buf;
        buf << log_file.rdbuf();
        return buf.str();
    }

    virtual void SetUp() {
        // Clear test log file if it exists
        if (std::filesystem::exists(TEST_LOG_PATH)) {
            ASSERT_TRUE(std::filesystem::remove(TEST_LOG_PATH));
        }
    }

    virtual void TearDown() {
        AesLogger::CloseLogger(&mock_ctx);
        ASSERT_TRUE(std::filesystem::remove(TEST_LOG_PATH));
    }
};

TEST_F(LoggerTest, TestLogMessagesEncSingle) {
    ASSERT_EQ(AesLogger::InitializeLogger(&mock_logger_fp, TEST_LOG_PATH, TRUE, &mock_ctx), ERROR_SUCCESS);

    // Verify encrypted logger key was copied correctly
    unsigned char want_encrypted_key[] = {
        0xda,0x67,0x0c,0xed,0x29,0xa8,0x71,0xb6,
        0xd8,0xdc,0x7e,0x40,0xda,0x88,0xc5,0x77,
        0xc1,0x86,0x73,0xcb,0xf4,0xc0,0x8e,0xc4,
        0x9a,0x7d,0x50,0x68,0x55,0x87,0x94,0xfb
    };
    ASSERT_EQ(memcmp(mock_ctx.encryption_key, want_encrypted_key, PI_AES_KEYLEN), 0);

    // Verify log messages
    ASSERT_EQ(AesLogger::LogDebug(&mock_ctx, "Test message: %d %s", 1, "hi"), ERROR_SUCCESS);

    // Verify correct IV placement
    unsigned char want_iv[PI_AES_BLOCKLEN] = {
        0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2,
        0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2
    };
    EXPECT_EQ(memcmp(mock_ctx.formatting_buf, want_iv, PI_AES_BLOCKLEN), 0);

    // Verify correct formatting and encryption
    LPCSTR want_partial = "Test message: 1 hi";
    EXPECT_STREQ(mock_ctx.formatting_buf + LOG_MSG_MAX_LEN + 1 + PI_AES_BLOCKLEN, want_partial);
    LPCSTR want_plaintext = "[DEBUG] [2025-01-02T13:14:15.678] Test message: 1 hi";
    PI_AesCtrCrypt(mock_ctx.encryption_key, want_iv, (unsigned char*)(mock_ctx.formatting_buf) + PI_AES_BLOCKLEN, strlen(want_plaintext));
    EXPECT_STREQ(mock_ctx.formatting_buf + PI_AES_BLOCKLEN, want_plaintext);

    // Verify written to file
    LPCSTR want_encoded = "AgICAgICAgICAgICAgICAlfyYGYY/QG6a1lUkNYQh6sRLTJbpulRa6rFlRngkp9j01yUWTDfSJ05p17ksQjTxCYdLuI=\n";
    EXPECT_STREQ(GetLogFileContents().c_str(), want_encoded);
}

TEST_F(LoggerTest, TestLogMessagesEncMultiple) {
    ASSERT_EQ(AesLogger::InitializeLogger(&mock_logger_fp, TEST_LOG_PATH, TRUE, &mock_ctx), ERROR_SUCCESS);

    ASSERT_EQ(AesLogger::LogDebug(&mock_ctx, "Test debug."), ERROR_SUCCESS);
    ASSERT_EQ(AesLogger::LogInfo(&mock_ctx, "Test info: %s", "msg"), ERROR_SUCCESS);
    ASSERT_EQ(AesLogger::LogWarn(&mock_ctx, "Test warning: %d %d", 1, 2), ERROR_SUCCESS);
    ASSERT_EQ(AesLogger::LogError(&mock_ctx, "Test ERROR"), ERROR_SUCCESS);
    ASSERT_EQ(AesLogger::LogSuccess(&mock_ctx, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."), ERROR_SUCCESS);

    LPCSTR want_contents = R"(AgICAgICAgICAgICAgICAlfyYGYY/QG6a1lUkNYQh6sRLTJbpulRa6rFlRngkp9j01yUWTDfSJQ5tlji+A==
AgICAgICAgICAgICAgICAlf/a2IC53zBAltWl84NhrcML1Q+pOBabqTOkQL4k5AGriilTzeLAZ46uxelux6O
AgICAgICAgICAgICAgICAlfhZHYD53zBAltWl84NhrcML1Q+pOBabqTOkQL4k5AGriilTzeLH5EuukTrsVfJ1TcP
AgICAgICAgICAgICAgICAlfzd3YC6AG6a1lUkNYQh6sRLTJbpulRa6rFlRngkp9j01yUWTDfSLUOhmLX
AgICAgICAgICAgICAgICAlflcGcO/w/JbUs/kNMPgrcMLC0/pY5aaaTOkBb/kYZtuUSdHA/EGpUx9ET1pRiExHNSKuQdS1JI/V4fM6FqxW1n04vFjDQ2jUGbFyQcW2WnyL6qox+R7xY2SxcA4eXnPdAjezICR2ZXUHfpXcITrJnKE2lGcxPfe2VjPDTYzSe5tl0gWgoaRM6vmIwdp2YAgnf/SHmWEHOjOhmpIraFecwqIdYTSGw892LhnggaoDmrSj+TDRjvaoM+KimFBb6krWJw01Ca4CAnb682+jGT8j8bDg4LCawo5PRtIb6qm0esTH3BIUTYzseBY2Joyy0j/N806IdgD67sLhwUNkH/GaecHK5JvCao88itneW6JDKuhcQnCOuugiPMjuKnC4KWSCHC+hCb6/dlnzeDhL+Kvzb6yIFmzvvZkDnrvtKelEr1081n9tvVIVi/OwzeR5iwyvk2t+23nhkflxlDDNFGgUPTju+JZ1zcoQehJIaUmd9h17SFKw0r5kiJYcvdns+Qz53Y86/9WIAM7e+1GPJS6CiCroQsddDs7Dr/yntUqBAA6mTWuxgSJq5dKkGn5UsMLp6etRs1H2RqoKlm55B5TLbdB9KaXLklgFws7nz5v+ODUJT1TPDwa7dlJ/9wRr8g5uM=
)";
    EXPECT_STREQ(GetLogFileContents().c_str(), want_contents);
}


TEST_F(LoggerTest, TestLogMessagesPlaintextSingle) {
    ASSERT_EQ(AesLogger::InitializeLogger(&mock_logger_fp, TEST_LOG_PATH, FALSE, &mock_ctx), ERROR_SUCCESS);

    // Verify log messages
    ASSERT_EQ(AesLogger::LogDebug(&mock_ctx, "Test message: %d %s", 1, "hi"), ERROR_SUCCESS);

    // Verify correct formatting
    LPCSTR want_plaintext = "[DEBUG] [2025-01-02T13:14:15.678] Test message: 1 hi\n";
    EXPECT_STREQ(mock_ctx.formatting_buf + PI_AES_BLOCKLEN, want_plaintext);

    // Verify written to file
    EXPECT_STREQ(GetLogFileContents().c_str(), want_plaintext);
}

TEST_F(LoggerTest, TestLogMessagesPlaintextMultiple) {
    ASSERT_EQ(AesLogger::InitializeLogger(&mock_logger_fp, TEST_LOG_PATH, FALSE, &mock_ctx), ERROR_SUCCESS);

    ASSERT_EQ(AesLogger::LogDebug(&mock_ctx, "Test debug."), ERROR_SUCCESS);
    ASSERT_EQ(AesLogger::LogInfo(&mock_ctx, "Test info: %s", "msg"), ERROR_SUCCESS);
    ASSERT_EQ(AesLogger::LogWarn(&mock_ctx, "Test warning: %d %d", 1, 2), ERROR_SUCCESS);
    ASSERT_EQ(AesLogger::LogError(&mock_ctx, "Test ERROR"), ERROR_SUCCESS);
    ASSERT_EQ(AesLogger::LogSuccess(&mock_ctx, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."), ERROR_SUCCESS);

    LPCSTR want_contents = R"([DEBUG] [2025-01-02T13:14:15.678] Test debug.
[INFO] [2025-01-02T13:14:15.678] Test info: msg
[WARN] [2025-01-02T13:14:15.678] Test warning: 1 2
[ERROR] [2025-01-02T13:14:15.678] Test ERROR
[SUCCESS] [2025-01-02T13:14:15.678] Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.
)";

    EXPECT_STREQ(GetLogFileContents().c_str(), want_contents);
}
