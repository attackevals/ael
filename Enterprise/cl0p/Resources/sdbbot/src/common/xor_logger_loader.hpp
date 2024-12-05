#pragma once

#include <Windows.h>
#include "base64_loader.hpp"
#include "errors.hpp"

namespace common {
	// Setup the logger by opening a handle to log_file_path and establishing the XOR encryption key
	int initialize_logger(std::wstring_view log_file_path, unsigned char* xor_key, size_t xor_key_len);

    void close_logger();

    // Log debug message
    bool log_debug(std::wstring_view msg);

    // Log info message
    bool log_info(std::wstring_view msg);

    // Log warning message
    bool log_warning(std::wstring_view msg);

    // Log error message
    bool log_error(std::wstring_view msg);

    // Log success message
    bool log_success(std::wstring_view msg);

    int append_to_log(std::span<std::byte> data);

    std::span<std::byte> encrypt_and_encode_log_msg(std::wstring_view to_log);
}