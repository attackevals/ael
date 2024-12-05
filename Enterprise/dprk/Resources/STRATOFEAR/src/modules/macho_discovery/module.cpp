/*
* macho_discovery
*      About:
*           Collects local system information using `popen` to 
*           execute system_profiler, sw_vers & dscl native utilities
*      Result:
*           Prints collected system users, hardware information, Operating 
*           System, and Build version to the terminal
*      MITRE ATT&CK Techniques:
*           T1033 System Owner/User Discovery
*           T1082 System Information Discovery
*           T1059.004 Command and Scripting Interpreter: Unix Shell
*      CTI:
*           https://www.mandiant.com/resources/blog/north-korea-supply-chain
*           https://www.jamf.com/blog/threat-hunting-unraveling-malware-tactics/
*           https://www.sentinelone.com/blog/macos-malware-2023-a-deep-dive-into
*           -emerging-trends-and-evolving-techniques/ 
*/
#include "module.hpp"

std::string runCommand(const char* command) {
    std::string output;
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        return "Error: Couldn't open pipe for command: " + std::string(command);
    }
    char buffer[128];
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            output += buffer;
    }
    pclose(pipe);
    return output;
}

std::string collectSystemInfo(){

    std::string systemInfo = "\nSystem Information\n";
    //Collect system name
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    std::string hostnameString(hostname);
    systemInfo += "\nHostname: " + hostnameString +"\n";

    const char* commands[] = {
    // Current User
    "logname",
    // System's architecture
    "/usr/sbin/system_profiler SPHardwareDataType",
    // OS version & build
    "/usr/bin/sw_vers",
    // List of current users (non-services)
    "dscl . -list /Users | grep -v '^_'",
    };
    for (const auto& command : commands) {
        std::string commandString(command);
        systemInfo += "Command: " + commandString + "\n";
        systemInfo += runCommand(command) + "\n";
    }
    return systemInfo;
}

int main(){
    std::cout << collectSystemInfo() << std::endl;
    return 0;
}