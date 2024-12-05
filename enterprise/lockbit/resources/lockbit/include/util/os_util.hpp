#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <filesystem>
#include <string>
#include <Windows.h>

// Defined in src/common/util/os_utils.cpp
namespace os_util {

// Gets the ID of the calling thread
std::string GetThreadIdStr();

// Writes buffer to specified file
DWORD WriteFileBytes(HANDLE h_file, char* buffer, DWORD buffer_len);

// Reads given file into buffer
DWORD ReadFileBytes(HANDLE h_file, unsigned char* buffer, DWORD bytes_to_read);

// Writes buffer to specified file on disk
DWORD WriteFileToDisk(LPCWSTR path, char* buffer, DWORD buffer_len);

// Gets the value of the specified environment variable
std::string GetEnvVariable(LPCSTR var);

// Gets the path to the PROGRAMDATA folder
std::filesystem::path GetProgramDataPath();

// Get absolute path to current executable
std::wstring GetCurrExecutablePath();

} // namespace util