#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "crypto/xor.hpp"
#include "crypto/xor_obfuscation.hpp"
#include "util/os_util.hpp"
#include "util/string_util.hpp"
#include <format>
#include <vector>

using FP_FindResourceW = decltype(&FindResourceW);
using FP_LoadResource = decltype(&LoadResource);
using FP_LockResource = decltype(&LockResource);
using FP_SizeofResource = decltype(&SizeofResource);

// Defined in src/common/util/resource_util.cpp
namespace resource_util {

// Get and decrypt resource data - throws exception on failure
std::vector<char> ReadResource(int id);

// Fetch given resource and write it to disk. Will throw exception on error
void WriteResourceToDisk(int resource_id, LPCWSTR path);

} // namespace resource_util