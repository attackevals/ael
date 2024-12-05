#pragma once
#include <string>


namespace Core {
    /*
     * CompareHash(std::string value, std::vector<size_t> hashList)
     *      About:
     *          Takes a string, converts it to uppercase, hashes it, and compares the hash
     *          to the specified predefined list of hashes.
     *      Result:
     *          Returns true if the hash of the string is found in the list, otherwise
     *          false.
     *      Returns:
     *          [bool]
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    bool CompareHash(std::string value, std::vector<size_t> hashList);
}