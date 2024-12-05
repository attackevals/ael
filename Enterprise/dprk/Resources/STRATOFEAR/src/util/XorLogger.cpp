/**
 * Provides XOR-encrypted and base64-encoded logging capabilities, where each line of the log file
 * is of the following format: base64(XOR(plaintext))
 * To use this file, include it and XorLogger.hpp in your implant source code, adjust DEBUG_MODE if desired,
 * adjust the Base64 encoder include path in XorLogger.cpp, and call XorLogger::InitializeLogger from your main
 * method with the desired log file path and XOR key
*/

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <vector>
#include <ctime>

#include "XorLogger.hpp"

namespace XorLogger {

#ifdef DEBUG
    std::mutex m_module_log_file;
    std::ofstream log_file_ofs;
#endif

    std::mutex m_module_log_console;
    std::vector<unsigned char> log_xor_key;
    size_t log_xor_key_len = 0;
    bool initialized = false;

    int _logMsg(std::string log_level, std::string msg);

    // Setup the logger by opening a handle to log_file_path and establishing the XOR encryption key
    int InitializeLogger(std::string log_file_path, std::vector<unsigned char> xor_key) {
        // No need to reinitialize
        if (initialized) return 0;

        // Verify and set key
        if (xor_key.size() == 0) {
            return INVALID_XOR_KEY;
        }
        log_xor_key = xor_key;
        log_xor_key_len = xor_key.size();

// Only check and create the log file if compiled for debug
#ifdef DEBUG
        // Verify log file path and open file
        if (log_file_path.length() == 0) {
            return INVALID_LOG_PATH;
        }
        std::filesystem::path file_path(log_file_path);
        if (file_path.empty() || !file_path.has_filename()) {
            return INVALID_LOG_PATH;
        }
        log_file_ofs.open(log_file_path, std::ios_base::app);
        if (!log_file_ofs.is_open()) {
            return FAILED_OPEN_LOG_FILE;
        }
#endif

        initialized = true;
        return 0;
    }

    // Log debug message
    int LogDebug(std::string msg) {
        return _logMsg("DEBUG", msg);
    }

    // Log info message
    int LogInfo(std::string msg) {
        return _logMsg("INFO", msg);
    }

    // Log warning message
    int LogWarning(std::string msg) {
        return _logMsg("WARN", msg);
    }

    // Log error message
    int LogError(std::string msg) {
        return _logMsg("ERROR", msg);
    }

    // Log success message
    int LogSuccess(std::string msg) {
        return _logMsg("SUCCESS", msg);
    }

    /// HELPER FUNCTIONS
#ifdef DEBUG
    int _appendStringToLog(std::string data) {
        // Critical section
        std::lock_guard<std::mutex> lock(m_module_log_file);
        try {
            if (!log_file_ofs.is_open()) {
                return LOG_FILE_CLOSED;
            }
            log_file_ofs << data << std::endl;
        } catch(...) {
            return FAIL_WRITE;
        }
        return 0;
        // End Critical section
    }

    std::string _encryptAndEncodeLogMsg(std::string to_log) {
        std::vector<unsigned char> buffer(to_log.begin(), to_log.end());
        size_t num_elements = buffer.size();
        for (size_t i = 0; i < num_elements; i++) {
            buffer[i] ^= log_xor_key[i % log_xor_key_len];
        }
        return Base64Encoder::Base64Encode(buffer);
    }
#endif
    // Get current UTC time as a formatted string: 2024-01-02T13:45:56
    // Returns empty string if an error occurs.
    std::string _getCurrUtcTimeStr() {
        struct tm *time_info;
        char time_buffer[100];
        time_t raw_time = time(NULL);
        time_info = gmtime(&raw_time);
        if (time_info != NULL) {
            strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%dT%H:%M:%S", time_info);
            return std::string(time_buffer);
        }
        return "";
    }

    int _logMsg(std::string log_level, std::string msg) {
        std::string timestamp = _getCurrUtcTimeStr();
        if (timestamp.length() == 0) {
            return FAILED_FETCH_TIMESTAMP;
        }
        std::string formatted = "[" + log_level + "] [" + timestamp + "] " + msg;

        // Print to stdout in plaintext if in debug mode
        {
            // Critical section
            std::lock_guard<std::mutex> lock(m_module_log_console);
            std::cout << formatted << std::endl;
        }

        // Make sure logger is initialized before attempting to encrypt/log to file
        if (!initialized) return 1;

// Only proceed to write to log file if compiled for debug, otherwise return success.
#ifdef DEBUG
        std::string encrypted_encoded = _encryptAndEncodeLogMsg(formatted);
        return _appendStringToLog(encrypted_encoded);
#else
        return 0;
#endif
    }
}