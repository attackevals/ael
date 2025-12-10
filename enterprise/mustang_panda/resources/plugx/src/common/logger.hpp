#pragma once

#include "shared_func.hpp"
#include "pi_aes_ctr.hpp"
#include "base64.hpp"

#define FAILED_OPEN_LOG_FILE 0x80001
#define INVALID_LOG_FILE_HANDLE 0x80002
#define FAIL_WRITE_LOG 0x80003
#define FAILED_FETCH_TIMESTAMP 0x80004
#define FAILED_LOG_ENCODING 0x80005

#define LOGGER_HEAP_SIZE 10*1024 // 10KB
#define LOG_MSG_MAX_LEN 2*1024 // 2KB
#define LOG_MSG_FORMATTING_BUF_LEN (PI_AES_BLOCKLEN + (2*(LOG_MSG_MAX_LEN + 1)))

// Include potential base64 padding and space for newline
#define LOG_MSG_MAX_ENCODED_LEN (((LOG_MSG_MAX_LEN + PI_AES_BLOCKLEN) * 4 / 3) + 5)

// Default to encrypted logging
#ifndef ENCRYPTED_LOGGING
    #define ENCRYPTED_LOGGING TRUE
#endif

struct logger_ctx {
    shared_func_pointers* fp;
    SYSTEMTIME time_buf;
    unsigned char* encoded_buf;
    char* formatting_buf;
    HANDLE h_log_file;
    HANDLE h_heap;
    unsigned char encryption_key[PI_AES_KEYLEN];
    BOOL enable_encryption;
};

namespace AesLogger {

DWORD InitializeLogger(shared_func_pointers* fp, LPCWSTR log_file, BOOL enable_encryption, logger_ctx* ctx);

void CloseLogger(logger_ctx* ctx);

DWORD LogDebug(logger_ctx* ctx, const char* fmt, ...);

DWORD LogInfo(logger_ctx* ctx, const char* fmt, ...);

DWORD LogWarn(logger_ctx* ctx, const char* fmt, ...);

DWORD LogError(logger_ctx* ctx, const char* fmt, ...);

DWORD LogSuccess(logger_ctx* ctx, const char* fmt, ...);

} // namespace
