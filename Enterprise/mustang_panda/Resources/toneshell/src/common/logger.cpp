#include "logger.hpp"
#include "util.hpp"
#include "xor.hpp"

#define _log_with_level(LEVEL) DWORD result; \
    va_list args; \
    va_start(args, fmt); \
    result = _logMsg(ctx, LEVEL, fmt, args); \
    va_end(args); \
    return result

namespace AesLogger {

// Opens the log file and stores the HANDLE in the logger context.
// Returns 0 on success, otherwise an error code
DWORD InitializeLogger(shared_func_pointers* fp, LPCWSTR log_file, BOOL enable_encryption, logger_ctx* ctx) {
    // Set function pointers for logger context
    ctx->fp = fp;

    DWORD result;
    ctx->h_heap = NULL;
    ctx->h_log_file = NULL;
    ctx->encoded_buf = NULL;
    ctx->formatting_buf = NULL;

    // Set up logging encryption key
    // (key will be decrypted during encryption routine)
    // key: c47001f8de67d8fe23b76d7685fe75fbb0abec9b3bb23f4cf99d7f3ece345c18
    // XOR mask: 1e170d15f7cfa948fb6b13365f76b08c712d9f50cf72b18863e02f569bb3c8e3
    if (enable_encryption) {
        ctx->enable_encryption = TRUE;
        unsigned char encrypted_key[PI_AES_KEYLEN] = {
            0xda,0x67,0x0c,0xed,0x29,0xa8,0x71,0xb6,
            0xd8,0xdc,0x7e,0x40,0xda,0x88,0xc5,0x77,
            0xc1,0x86,0x73,0xcb,0xf4,0xc0,0x8e,0xc4,
            0x9a,0x7d,0x50,0x68,0x55,0x87,0x94,0xfb
        };
        pi_memcpy(ctx->encryption_key, encrypted_key, PI_AES_KEYLEN);
    } else {
        ctx->enable_encryption = FALSE;
    }

    do {
        // Create heap and message buffers
        ctx->h_heap = ctx->fp->fp_HeapCreate(0, LOGGER_HEAP_SIZE, LOGGER_HEAP_SIZE);
        if (ctx->h_heap == NULL) {
            result = ctx->fp->fp_GetLastError();
            break;
        }

        if (ctx->enable_encryption) {
            ctx->encoded_buf = reinterpret_cast<unsigned char*>(ctx->fp->fp_HeapAlloc(ctx->h_heap, HEAP_ZERO_MEMORY, LOG_MSG_MAX_ENCODED_LEN));
            if (ctx->encoded_buf == NULL) {
                result = FAIL_HEAP_ALLOCATION;
                break;
            }
        }

        ctx->formatting_buf = reinterpret_cast<char*>(ctx->fp->fp_HeapAlloc(ctx->h_heap, HEAP_ZERO_MEMORY, LOG_MSG_FORMATTING_BUF_LEN));
        if (ctx->formatting_buf == NULL) {
            result = FAIL_HEAP_ALLOCATION;
            break;
        }

        // Open logger file
        ctx->h_log_file = ctx->fp->fp_CreateFileW(
            log_file,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_HIDDEN,
            NULL
        );
        if (ctx->h_log_file == INVALID_HANDLE_VALUE) {
            result = ctx->fp->fp_GetLastError();
            break;
        }

        // Seek to end of file to append
        if (ctx->fp->fp_SetFilePointer(ctx->h_log_file, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER) {
            result = ctx->fp->fp_GetLastError();
            break;
        }
        result = ERROR_SUCCESS;
    } while (false);

    if (result != ERROR_SUCCESS) {
        CloseLogger(ctx);
    }

    return result;
}

void CloseLogger(logger_ctx* ctx) {
    // clean up heap
    if (ctx->encoded_buf != NULL) {
        ctx->fp->fp_HeapFree(ctx->h_heap, 0, ctx->encoded_buf);
        ctx->encoded_buf = NULL;
    }
    if (ctx->formatting_buf != NULL) {
        ctx->fp->fp_HeapFree(ctx->h_heap, 0, ctx->formatting_buf);
        ctx->formatting_buf = NULL;
    }
    if (ctx->h_heap != NULL) {
        ctx->fp->fp_HeapDestroy(ctx->h_heap);
        ctx->h_heap = NULL;
    }

    // close log file
    if (ctx->h_log_file != NULL && ctx->h_log_file != INVALID_HANDLE_VALUE) {
        ctx->fp->fp_CloseHandle(ctx->h_log_file);
        ctx->h_log_file = NULL;
    }
}

DWORD _logMsg(logger_ctx* ctx, const char* log_level, const char* fmt, va_list args) {
    // Fetch timestamp
    ctx->fp->fp_GetSystemTime(&(ctx->time_buf));

    // Logged message format: [LEVEL] [TIMESTAMP] formatted_msg
    // "[%s] [%d-%02d-%02dT%02d:%02d:%02d.%03d] %s";
    char final_fmt[43];
    final_fmt[42] = 0;
    final_fmt[0] = final_fmt[5] = '[';
    final_fmt[1] = final_fmt[6] = final_fmt[9] = final_fmt[14] = '%';
    final_fmt[19] = final_fmt[24] = final_fmt[29] = final_fmt[34] = final_fmt[40] = '%';
    final_fmt[2] = final_fmt[41] = 's';
    final_fmt[3] = final_fmt[38] = ']';
    final_fmt[4] = final_fmt[39] = ' ';
    final_fmt[7] = final_fmt[12] = final_fmt[17] = final_fmt[22] = 'd';
    final_fmt[27] = final_fmt[32] = final_fmt[37] = 'd';
    final_fmt[8] = final_fmt[13] = '-';
    final_fmt[10] = final_fmt[15] = final_fmt[20] = '0';
    final_fmt[25] = final_fmt[30] = final_fmt[35] = '0';
    final_fmt[11] = final_fmt[16] = final_fmt[21] = final_fmt[26] = final_fmt[31] = '2';
    final_fmt[18] = 'T';
    final_fmt[23] = final_fmt[28] = ':';
    final_fmt[33] = '.';
    final_fmt[36] = '3';

    // Format message using second half of buffer, offsetting for encryption IV
    ctx->fp->fp_vsnprintf(
        ctx->formatting_buf + LOG_MSG_MAX_LEN + 1 + PI_AES_BLOCKLEN,
        LOG_MSG_MAX_LEN,
        fmt,
        args
    );

    // Format with timestamp and level using first half of buffer,
    // offsetting for encryption IV
    int formatted_bytes = ctx->fp->fp_snprintf(
        ctx->formatting_buf + PI_AES_BLOCKLEN,
        LOG_MSG_MAX_LEN,
        final_fmt,
        log_level,
        ctx->time_buf.wYear,
        ctx->time_buf.wMonth,
        ctx->time_buf.wDay,
        ctx->time_buf.wHour,
        ctx->time_buf.wMinute,
        ctx->time_buf.wSecond,
        ctx->time_buf.wMilliseconds,

        // starting point of first formatted message
        ctx->formatting_buf + LOG_MSG_MAX_LEN + 1 + PI_AES_BLOCKLEN
    );

    // encrypt and encode
    if (ctx->enable_encryption) {
        unsigned char* iv = reinterpret_cast<unsigned char*>(ctx->formatting_buf);
        NTSTATUS rand_result = ctx->fp->fp_BCryptGenRandom(
            BCRYPT_RNG_ALG_HANDLE,
            iv,
            PI_AES_BLOCKLEN,
            0
        );
        if (rand_result != STATUS_SUCCESS) {
            return (DWORD)rand_result;
        }
        PI_AesCtrCrypt(
            ctx->encryption_key,
            iv,
            iv + PI_AES_BLOCKLEN, // plaintext starts after the IV
            formatted_bytes // length of the previously formatted string
        );
        size_t encoded_len = LOG_MSG_MAX_ENCODED_LEN - 1; // reserve spot for newline
        if (!PI_Base64Encode(
            reinterpret_cast<unsigned char*>(ctx->formatting_buf),
            formatted_bytes + PI_AES_BLOCKLEN,
            ctx->encoded_buf,
            &encoded_len // will contain actual length of encoded string
        )) {
            return FAILED_LOG_ENCODING;
        }
        ctx->encoded_buf[encoded_len] = '\n';

        // Log to file
        return WriteFileBytes(
            ctx->fp,
            ctx->h_log_file,
            reinterpret_cast<char*>(ctx->encoded_buf),
            encoded_len + 1 // include newline
        );
    } else {
        char* start = ctx->formatting_buf + PI_AES_BLOCKLEN;
        start[formatted_bytes] = '\n'; // append newline
        start[formatted_bytes + 1] = 0;

        // Log to file without encrypting/encoding
        return WriteFileBytes(
            ctx->fp,
            ctx->h_log_file,
            start,
            formatted_bytes + 1 // include newline
        );
    }
}

DWORD LogDebug(logger_ctx* ctx, const char* fmt, ...) {
    char level[6];
    level[0] = 'D';
    level[1] = level[0] + 1;
    level[2] = level[0] - 2;
    level[3] = 'U';
    level[4] = level[0] + 3;
    level[5] = 0;
    _log_with_level(level);
}

DWORD LogInfo(logger_ctx* ctx, const char* fmt, ...) {
    char level[5];
    level[0] = 'I';
    level[1] = level[0] + 5;
    level[2] = level[0] - 3;
    level[3] = level[1] + 1;
    level[4] = 0;
    _log_with_level(level);
}

DWORD LogWarn(logger_ctx* ctx, const char* fmt, ...) {
    char level[5];
    level[0] = 'W';
    level[1] = 'A';
    level[2] = 'R';
    level[3] = 'N';
    level[4] = 0;
    _log_with_level(level);
}

DWORD LogError(logger_ctx* ctx, const char* fmt, ...) {
    char level[6];
    level[0] = 'E';
    level[1] = level[2] = level[4] = 'R';
    level[3] = 'O';
    level[5] = 0;
    _log_with_level(level);
}

DWORD LogSuccess(logger_ctx* ctx, const char* fmt, ...) {
    char level[8];
    level[0] = level[5] = level[6] = 'S';
    level[1] = 'U';
    level[2] = level[3] = 'C';
    level[4] = 'E';
    level[7] = 0;
    _log_with_level(level);
}

} // namespace
