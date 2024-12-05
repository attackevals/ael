#pragma once
#ifndef __ENUMERATE_H__
#define __ENUMERATE_H__

#include <Windows.h>
#include <vector>
#include <iostream>
#include <fileapi.h>
#include <filesystem>
#include <string>
#include <algorithm>

#include "logger.hpp"
#include "xor_string.hpp"
#include "argparser.hpp"
#include "hash_string.hpp"

namespace enumerate {

    /*
     * EnumerateFilesystemPath:
     *      About:
     *          Takes a string directory and enumerates all files/directories in the directory to determine if they are valid files/directories
     *          For Files: Takes a string file or file path and determines if it has the correct file attributes, and is not on the blacklist.
     *          For directories: Takes a string directory or directory path and determines if it is not on the blacklist.
     *      Result:
     *          returns a tuple of vector strings. One is a list of files to be exfiltrated, and one is directories to be enumerated.
     *      Returns:
     *          std::tuple<std::vector<std::string>, std::vector<std::string>>
     *      MITRE ATT&CK Techniques:
     *          T1083 - File and Directory Discovery
     *          T1119 - Automated Collection
     *          T1020 - Automated Exfiltration
     *      CTI:
     *          https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf
     *          https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/
     *          https://www.cybereason.com/blog/research/threat-analysis-report-inside-the-lockbit-arsenal-the-stealbit-exfiltration-tool
     *      Other References:
     *          n/a
     */
    std::vector<std::string> EnumerateFilesystemPath(std::string);

    std::string to_lowercase(const std::string& str);
}

namespace blacklist {
    extern std::vector<unsigned long> blackListedFileNames;
    extern std::vector<unsigned long> blackListedFileExtensions;
    extern std::vector<unsigned long> blackListedDirectoryNames;
}

#endif