#include <string>

typedef unsigned char byte;

namespace encryption {

    bool EncryptFileTwoFish(const std::string& filePath);

    bool DecryptFileTwoFish(const std::string& filePath);
}
