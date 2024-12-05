/**
 * Provides XOR-encrypted and base64-encoded logging capabilities, where each line of the log file
 * is of the following format: base64(XOR(plaintext))
 * To use this header file, include it and XorLogger.cpp in your implant source code, adjust DEBUG_MODE if desired,
 * adjust the Base64 encoder include path in XorLogger.cpp, and call XorLogger::InitializeLogger from your main
 * method with the desired log file path and XOR key
*/
#pragma once

#ifndef XOR_LOGGING_H_
#define XOR_LOGGING_H_

#define INVALID_XOR_KEY 1
#define INVALID_LOG_PATH 2
#define FAILED_OPEN_LOG_FILE 3
#define LOG_FILE_CLOSED 4
#define FAIL_WRITE 5
#define FAILED_FETCH_TIMESTAMP 6

#include <string>
#include <vector>
#include "base64.hpp"

namespace XorLogger {
    // Setup logging mechanism
    bool SetupLogger();

    // Setup the logger by opening a handle to log_file_path and establishing the XOR encryption key
    int InitializeLogger(std::string log_file_path, std::vector<unsigned char> xor_key);

    // Log debug message
    int LogDebug(std::string msg);

    // Log info message
    int LogInfo(std::string msg);

    // Log warning message
    int LogWarning(std::string msg);

    // Log error message
    int LogError(std::string msg);

    // Log success message
    int LogSuccess(std::string msg);
}

#endif