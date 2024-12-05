#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <vector>

#include "logger.hpp"

namespace Logger {

#ifdef DEBUG
    std::mutex m_module_log_file;
#endif
    std::mutex m_module_log_console;
    std::ofstream log_file_ofs;
    bool initialized = false;

    int _logMsg(const std::string& log_level, const char* msg);

    void CloseLogger() {
        if (log_file_ofs.is_open()) {
            log_file_ofs.close();
        }
    }

    // Setup the logger by opening a handle to log_file_path
    int InitializeLogger(const std::string& log_file_path) {
        // No need to reinitialize
        if (initialized) return 0;

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
    int LogDebug(const std::string& msg) {
        return _logMsg("DEBUG", msg.c_str());
    }
    int LogDebug(const char* msg) {
        return _logMsg("DEBUG", msg);
    }

    // Log info message
    int LogInfo(const std::string& msg) {
        return _logMsg("INFO", msg.c_str());
    }
    int LogInfo(const char* msg) {
        return _logMsg("INFO", msg);
    }

    // Log warning message
    int LogWarning(const std::string& msg) {
        return _logMsg("WARN", msg.c_str());
    }
    int LogWarning(const char* msg) {
        return _logMsg("WARN", msg);
    }

    // Log error message
    int LogError(const std::string& msg) {
        return _logMsg("ERROR", msg.c_str());
    }
    int LogError(const char* msg) {
        return _logMsg("ERROR", msg);
    }

    // Log success message
    int LogSuccess(const std::string& msg) {
        return _logMsg("SUCCESS", msg.c_str());
    }
    int LogSuccess(const char* msg) {
        return _logMsg("SUCCESS", msg);
    }

    /// HELPER FUNCTIONS
#ifdef DEBUG
    int _appendStringToLog(const std::string& data) {
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
#endif
    // Get current UTC time as a formatted string: 2024-01-02T13:45:56
    // Returns empty string if an error occurs.
    std::string _getCurrUtcTimeStr() {
        struct tm time_info;
        char time_buffer[100];
        time_t raw_time = time(NULL);
        if (gmtime_s(&time_info, &raw_time) == 0) {
            strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%dT%H:%M:%S", &time_info);
            return std::string(time_buffer);
        }
        return "";
    }

    int _logMsg(const std::string& log_level, const char* msg) {
        std::string timestamp = _getCurrUtcTimeStr();
        if (timestamp.length() == 0) {
            return FAILED_FETCH_TIMESTAMP;
        }
        std::string formatted = std::format("[{}] [{}] {}", log_level, timestamp, msg);
        
        // Print to stdout in plaintext if in debug mode
        {
            // Critical section
            std::lock_guard<std::mutex> lock(m_module_log_console);
            std::cout << formatted << std::endl;
        }

        // Make sure logger is initialized before attempting to log to file
        if (!initialized) return 1;

// Only proceed to write to log file if compiled for debug, otherwise return success.
#ifdef DEBUG
        return _appendStringToLog(formatted);
#else
        return 0;
#endif
    }
}