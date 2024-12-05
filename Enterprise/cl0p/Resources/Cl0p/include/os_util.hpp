#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <string>
#include <Windows.h>

namespace os_util {

/*
 * GetCurrExecutableShortPath:
 *      About:
 *          Gets the short form of the absolute path to the current executable
 *          Relevant API calls:
 *              GetModuleFileNameW, GetShortPathNameW
 *      Result:
 *          string containing short form of current executable's absolute path
 *      CTI:
 *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
 */
std::wstring GetCurrExecutableShortPath();

} // namespace os_util
