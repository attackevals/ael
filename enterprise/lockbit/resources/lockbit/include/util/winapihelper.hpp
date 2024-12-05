#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <string>
#include <Windows.h>
#include <libloaderapi.h>

#define FAIL_BAD_IMAGE_DOS_SIGNATURE 0x64000001
#define FAIL_BAD_IMAGE_NT_SIGNATURE 0x64000002
#define FAIL_IMAGE_NOT_A_DLL 0x64000003
#define FAIL_IMAGE_EXPORT_DIR_NULL_ADDR 0x64000004
#define FAIL_IMAGE_EXPORT_DIR_EMPTY 0x64000005
#define FAIL_REQUESTED_API_NOT_FOUND 0x64000006
#define FAIL_BAD_FORWARDED_NAME 0x64000007
#define FAIL_PROCESS_FORWARDED_MODULE 0x64000008
#define FAIL_FORWARDED_API_NOT_FOUND 0x64000008

typedef LONG NTSTATUS, *PNTSTATUS;
#define STATUS_SUCCESS (NTSTATUS)0x00000000

using FP_GetCurrentThread = decltype(&GetCurrentThread);
using FP_WaitForSingleObject = decltype(&WaitForSingleObject);

// Defined in src/common/util/winapihelper.cpp
namespace winapi_helper {

/*
 * GetAPI:
 *      About:
 *          Given a desired API hash and module name, returns the process address
 *          for the given function.
 *          If the function fails to get the process address, it will return NULL
 *          and set error_code accordingly.
 *          Relevant API calls: LoadLibraryW
 *      Result:
 *          Process address, or NULL. error_code set to ERROR_SUCCESS or corresponding error code.
 *      MITRE ATT&CK Techniques:
 *          T1027.007: Obfuscated Files or Information: Dynamic API Resolution
 *          T1106: Native API
 *      CTI:
 *          https://redpiranha.net/news/look-lockbit-3-ransomware
 */
FARPROC GetAPI(unsigned long hash, LPCWSTR module, DWORD* error_code);

} // namespace