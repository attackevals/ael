#include "api_helper.hpp"
#include <gtest/gtest.h>

TEST(ApiHelperTest, TestFNV1aHash) {
    // Values obtained from https://md5calc.com/hash/fnv1a32
    EXPECT_EQ(cx_fnv_hash("testvalue"), 0x900e756a);
    EXPECT_EQ(cx_fnv_hash("SetHandleInformation"), 0x9ba12495);
    EXPECT_EQ(cx_fnv_hash("ABC123"), 0x45854fe5);
}

TEST(ApiHelperTest, TestApiResolution) {
    HMODULE h_kernel32 = GetModuleHandleW(L"kernel32.dll");
    HMODULE h_ntdll = GetModuleHandleW(L"ntdll.dll");
    ASSERT_TRUE(h_kernel32 != NULL);
    ASSERT_TRUE(h_ntdll != NULL);
    EXPECT_EQ((FARPROC)(RESOLVE_FN_PTR_FNV1A(L"kernel32.dll", GetLastError)), GetProcAddress(h_kernel32, "GetLastError"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_FNV1A(L"kernel32.dll", GetLastError)), GetProcAddress(h_kernel32, "GetLastError"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_PTR_FNV1A(L"kernel32.dll", CreateFileW)), GetProcAddress(h_kernel32, "CreateFileW"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_FNV1A(L"kernel32.dll", CreateFileW)), GetProcAddress(h_kernel32, "CreateFileW"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_PTR_FNV1A(L"kernel32.dll", CloseHandle)), GetProcAddress(h_kernel32, "CloseHandle"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_FNV1A(L"kernel32.dll", CloseHandle)), GetProcAddress(h_kernel32, "CloseHandle"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_PTR_FNV1A(L"ntdll.dll", _vsnprintf)), GetProcAddress(h_ntdll, "_vsnprintf"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_FNV1A(L"ntdll.dll", _vsnprintf)), GetProcAddress(h_ntdll, "_vsnprintf"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_PTR_FNV1A(L"ntdll.dll", _snprintf)), GetProcAddress(h_ntdll, "_snprintf"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_FNV1A(L"ntdll.dll", _snprintf)), GetProcAddress(h_ntdll, "_snprintf"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_FNV1A(L"kernel32.dll", VirtualAlloc)), GetProcAddress(h_kernel32, "VirtualAlloc"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_PTR_FNV1A(L"kernel32.dll", VirtualAlloc)), GetProcAddress(h_kernel32, "VirtualAlloc"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_FNV1A(L"kernel32.dll", VirtualProtect)), GetProcAddress(h_kernel32, "VirtualProtect"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_PTR_FNV1A(L"kernel32.dll", VirtualProtect)), GetProcAddress(h_kernel32, "VirtualProtect"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_FNV1A(L"kernel32.dll", VirtualFree)), GetProcAddress(h_kernel32, "VirtualFree"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_PTR_FNV1A(L"kernel32.dll", VirtualFree)), GetProcAddress(h_kernel32, "VirtualFree"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_FNV1A(L"kernel32.dll", CreateProcessW)), GetProcAddress(h_kernel32, "CreateProcessW"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_PTR_FNV1A(L"kernel32.dll", CreateProcessW)), GetProcAddress(h_kernel32, "CreateProcessW"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_FNV1A(L"kernel32.dll", GetModuleFileNameA)), GetProcAddress(h_kernel32, "GetModuleFileNameA"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_PTR_FNV1A(L"kernel32.dll", GetModuleFileNameA)), GetProcAddress(h_kernel32, "GetModuleFileNameA"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_FNV1A(L"kernel32.dll", GetModuleFileNameW)), GetProcAddress(h_kernel32, "GetModuleFileNameW"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_PTR_FNV1A(L"kernel32.dll", GetModuleFileNameW)), GetProcAddress(h_kernel32, "GetModuleFileNameW"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_FNV1A(L"kernel32.dll", CreateThread)), GetProcAddress(h_kernel32, "CreateThread"));
    EXPECT_EQ((FARPROC)(RESOLVE_FN_PTR_FNV1A(L"kernel32.dll", CreateThread)), GetProcAddress(h_kernel32, "CreateThread"));
}
