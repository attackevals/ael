#include "xor_logger_loader.hpp"

namespace common {
#if DEBUG
    bool debug = false;
#else
    bool debug = true;
#endif
#define SUCCESS 0;
#define ALREADY_INITIALIZED 1;
#define CREATE_LOG_FAILED 2;
#define CREATE_MUTEX_FAILED 3;
#define NOT_INITIALIZED 4;
#define GET_TIMESTAMP_FAILED 5;
#define WRITE_FAILED 6;
#define WRITE_FAILED_NL 7;
    
    unsigned char* log_xor_key = nullptr;
    size_t loader_log_xor_key_len = 0;
    HANDLE h_log_file = nullptr;
    HANDLE h_log_mutex = nullptr;
    bool loader_log_initialized = false;

    int _log_msg(std::wstring_view level, std::wstring_view msg);

    int initialize_logger(std::wstring_view log_file_path, unsigned char* xor_key, size_t xor_key_len) {
        
        // check if the logger is already initialized
        if (loader_log_initialized) {
            return ALREADY_INITIALIZED;
        }

        log_xor_key = xor_key;
        loader_log_xor_key_len = xor_key_len;

// Only check and create the log file if compiled for debug
#ifdef DEBUG
        // attempt to create the log file and open a handle to it
        h_log_file = CreateFileW(
            log_file_path.data(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );
        if (h_log_file == INVALID_HANDLE_VALUE) {
            return CREATE_LOG_FAILED;
        }
#endif

        // attempt to create a mutex for the log file
        h_log_mutex = CreateMutexW(
            nullptr,
            true,
            nullptr
        );
        if (h_log_mutex == NULL) {
            return CREATE_MUTEX_FAILED;
        }

        // initialized
        loader_log_initialized = true;
        return SUCCESS;
    }

    void close_logger() {
        log_xor_key = nullptr;
        loader_log_xor_key_len = 0;
        h_log_file = nullptr;
        h_log_mutex = nullptr;
        loader_log_initialized = false;
    }

    bool log_debug(std::wstring_view msg) {
        return _log_msg(L"DEBUG", msg) == SUCCESS;
    }

    bool log_info(std::wstring_view msg) {
        return _log_msg(L"INFO", msg) == SUCCESS;
    }

    bool log_warning(std::wstring_view msg) {
        return _log_msg(L"WARN", msg) == SUCCESS;
    }

    bool log_error(std::wstring_view msg) {
        return _log_msg(L"ERROR", msg) == SUCCESS;
    }

    bool log_success(std::wstring_view msg) {
        return _log_msg(L"SUCCESS", msg) == SUCCESS;
    }

    int append_to_log(std::span<std::byte> data) {
        WaitForSingleObject(h_log_mutex, INFINITE);

        // write the log message to the file
        DWORD written = 0;
        if (!WriteFile(
            h_log_file,
            data.data(),
            data.size_bytes(),
            &written,
            nullptr
        )) {
            
            return WRITE_FAILED;
        }

        // write a new line to the file
        if (!WriteFile(
            h_log_file,
            L"\n",
            1,
            &written,
            nullptr
        )) {
            return WRITE_FAILED_NL;
        }
        
        // free the buffer containing the log message
        VirtualFree(data.data(), 0, MEM_RELEASE);
        return SUCCESS;
    }

    std::span<std::byte> encrypt_and_encode_log_msg(std::wstring_view to_log) {
        // convert to char array so output isn't wchar
        size_t len = to_log.size();
        auto buf = static_cast<char*>(VirtualAlloc(NULL, len, MEM_COMMIT, PAGE_READWRITE));
        wcstombs(buf, to_log.data(), len);

        // perform xor
        for (size_t i = 0; i < len; i++) {
            buf[i] = buf[i] ^ log_xor_key[i % loader_log_xor_key_len];
        }

        // encode xor'd string
        std::span<std::byte> ret = common::base64_encode(std::span<std::byte>(reinterpret_cast<std::byte*>(buf), len));

        // free buffer containing xor'd string
        VirtualFree(buf, 0, MEM_RELEASE);
        return ret;
    }

    bool _get_curr_utc_time_str(wchar_t* time_buffer, size_t buf_len) {
        struct tm time_info;
        memset(time_buffer, 0, buf_len);
        time_t raw_time = time(NULL);
        if (gmtime_s(&time_info, &raw_time) == 0) {
            wcsftime(time_buffer, buf_len, L"%Y-%m-%d %H:%M:%S", &time_info);
            return true;
        }
        return false;
    }

    void _format_string(wchar_t* out, const wchar_t* level, const wchar_t* timestamp, const wchar_t* msg) {
        wcscat(out, L"[");
        wcscat(out, level);
        wcscat(out, L"] [");
        wcscat(out, timestamp);
        wcscat(out, L"] ");
        wcscat(out, msg);
    }

    int _log_msg(std::wstring_view level, std::wstring_view msg) {
        // ensure initialized
        if (!loader_log_initialized) {
            return NOT_INITIALIZED;
        }

// Only proceed to write to log file if compiled for debug, otherwise return success.
#ifdef DEBUG
        // get time stamp
        const size_t buf_len = 100;
        wchar_t time_buffer[buf_len]{};
        if (!_get_curr_utc_time_str(time_buffer, buf_len)) {
            return GET_TIMESTAMP_FAILED;
        }
        std::wstring_view timestamp = time_buffer;

        // format log message
        size_t size = level.size() + timestamp.size() + msg.size() + 7;
        wchar_t* formatted = static_cast<wchar_t*>(VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE));
        _format_string(formatted, level.data(), timestamp.data(), msg.data());

        
        if (debug) {
            // TODO: output to console. doing so crashes the loader
        }

        // log the message
        int ret = append_to_log(encrypt_and_encode_log_msg(formatted));

        // free the buffer containing the formatted log message
        VirtualFree(formatted, 0, MEM_RELEASE);
        return ret;
#else
        return 0;
#endif
    }
}