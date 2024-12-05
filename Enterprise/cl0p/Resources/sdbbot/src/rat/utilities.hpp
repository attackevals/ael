#pragma once
#ifndef __UTIL_H__
#define __UTIL_H__

#include <filesystem>
#include <string>
#include "xor_logger.hpp"
#include "settings.hpp"
#include "xor_string.hpp"

// General utilities used by the RAT.
namespace Utilities {

    //  GetFileNameFromPath(std::string filePath)
    //      About:
    //         taskes a string file path and returns a file name if the file exists.
    //      Result:
    //         A string file name is returned
    //      Returns:
    //         [std::string]
    //      MITRE ATT&CK Techniques:
    //         n/a
    //      CTI:
    //         n/a
    std::string GetFileNameFromPath(std::string filePath);

    //  XorStringData()
    //      About:
    //         Gets the local systems two letter country code
    //      Result:
    //         returns the country code as a string
    //      Returns:
    //         [std::string]
    //      CTI:
    //         https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
    //         https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
    std::string XorStringData(std::string data);
}

#endif