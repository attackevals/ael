#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <string>
#include <Windows.h>
#include <gtest/gtest.h>

// Defined in src/common/util/test_utils.cpp
namespace test_utils {

void CompareFileHash(const std::wstring& path, const std::string& want);

void CompareRegContentsDefaultValue(const std::wstring& key_path, const std::string& expected);

void CompareRegContents(const std::wstring& key_path, const std::wstring& value, const std::string& expected);

void CompareRegBinaryContents(const std::wstring& key_path, const std::wstring& value, const std::string& expected_hex_str);

void CreateRegKey(const std::wstring& key_path);

void DeleteRegKey(const std::wstring& key_path);

void CreateDummyFile(const std::string& path, const std::string& contents);

void CompareFileContents(const std::wstring& path, const std::vector<char>& want);

int GetNumProcesses(LPCWSTR proc_name);

void KillProcess(LPCWSTR proc_name);

} // namespace