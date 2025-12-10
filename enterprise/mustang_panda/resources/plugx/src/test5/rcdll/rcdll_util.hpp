#pragma once

#include <Windows.h>
#include <string_view>

// Reads given file into buffer
DWORD ReadFileBytes(HANDLE h_file, unsigned char* buffer, DWORD bytes_to_read);

size_t GetExecutableNameLength(std::wstring_view command_line);

BOOL CommandLineHasArg(std::wstring_view command_line, LPCWSTR arg);

BOOL RunningFromInstaller();

void ConcatToSelfDir(char* path, size_t buf_size, const char* to_append);
