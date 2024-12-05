#include "enum.hpp"

namespace enumeration {
    bool CheckValidFile(std::string filePath) {
        DWORD fileInfo = GetFileAttributes(filePath.c_str());

        if (fileInfo == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND)
            return false;

        if (fileInfo & FILE_ATTRIBUTE_SYSTEM) 
            return false;

        return true;
    }


    std::vector<std::string> EnumerateFilesystemPath(const std::string& path) {
        std::vector<std::string> files;
        std::filesystem::path fsPath(path);
        std::string enumPath = path + "*";

        try {
            if (!std::filesystem::is_symlink(fsPath)) {
                if (std::filesystem::is_directory(fsPath)) {
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
                                    }

                                }
                            }
                        } while (FindNextFile(hFind, &findData) != 0);

                        FindClose(hFind);
                    }
                }
                else {
                    if (CheckValidFile(fsPath.string()))
                        files.push_back(fsPath.string());
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
        }

        return files;
    }
}