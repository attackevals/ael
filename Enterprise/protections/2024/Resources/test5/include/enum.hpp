#include <Windows.h>
#include <vector>
#include <iostream>
#include <fileapi.h>
#include <filesystem>
#include <string>

namespace enumeration {

    std::vector<std::string> EnumerateFilesystemPath(const std::string& path);
}