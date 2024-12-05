#include "utilities.hpp"

namespace Utilities {

    // utility to get file name from a given file path string
    std::string GetFileNameFromPath(std::string filePath) 
    {
        try {
            std::filesystem::path pathObj(filePath);

            // check file exists
            if (!std::filesystem::exists(pathObj)) {
                common::LogError(XOR_LIT("Cannot extract filename from non-existent file: ") + filePath);
                return "";
            }

            std::string filename = pathObj.filename().string();
            return filename;
        }
        catch (const std::filesystem::filesystem_error& e) {
            common::LogWarning(XOR_LIT("Filesystem error: ") + std::string{ e.what() });
            return "";
        }
        catch (const std::exception& e) {
            common::LogWarning(XOR_LIT("Standard exception: ") + std::string{ e.what() });
            return "";
        }
    }

    std::string XorStringData(std::string data)
    {
        std::string output = data;
        for (size_t i = 0; i < data.size(); ++i)
        {
            output[i] = data[i] ^ Settings::key[i % Settings::key.size()];
        }

        return output;
    }
}