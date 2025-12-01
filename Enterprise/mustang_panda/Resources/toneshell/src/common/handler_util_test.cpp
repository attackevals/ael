#include "handler_util.hpp"
#include <gtest/gtest.h>

TEST(HandlerUtilTest, TestResolveSharedApis) {
    HMODULE h_kernel32 = GetModuleHandleW(L"kernel32.dll");
    HMODULE h_ntdll = GetModuleHandleW(L"ntdll.dll");
    ASSERT_TRUE(h_kernel32 != NULL);
    ASSERT_TRUE(h_ntdll != NULL);

    shared_func_pointers fp;
    memset(&fp, 0, sizeof(fp));
    ASSERT_EQ(ResolveSharedApis(&fp), ERROR_SUCCESS);
    HMODULE h_bcrypt = GetModuleHandleW(L"bcrypt.dll");
    ASSERT_TRUE(h_bcrypt != NULL);

    EXPECT_EQ((FARPROC)fp.fp_GetLastError, GetProcAddress(h_kernel32, "GetLastError"));
    EXPECT_EQ((FARPROC)fp.fp_CreateFileW, GetProcAddress(h_kernel32, "CreateFileW"));
    EXPECT_EQ((FARPROC)fp.fp_CloseHandle, GetProcAddress(h_kernel32, "CloseHandle"));
    EXPECT_EQ((FARPROC)fp.fp_vsnprintf, GetProcAddress(h_ntdll, "_vsnprintf"));
    EXPECT_EQ((FARPROC)fp.fp_snprintf, GetProcAddress(h_ntdll, "_snprintf"));
    EXPECT_EQ((FARPROC)fp.fp_GetSystemTime, GetProcAddress(h_kernel32, "GetSystemTime"));
    EXPECT_EQ((FARPROC)fp.fp_SetFilePointer, GetProcAddress(h_kernel32, "SetFilePointer"));
    EXPECT_EQ((FARPROC)fp.fp_HeapAlloc, GetProcAddress(h_kernel32, "HeapAlloc"));
    EXPECT_EQ((FARPROC)fp.fp_HeapCreate, GetProcAddress(h_kernel32, "HeapCreate"));
    EXPECT_EQ((FARPROC)fp.fp_HeapDestroy, GetProcAddress(h_kernel32, "HeapDestroy"));
    EXPECT_EQ((FARPROC)fp.fp_HeapFree, GetProcAddress(h_kernel32, "HeapFree"));
    EXPECT_EQ((FARPROC)fp.fp_WriteFile, GetProcAddress(h_kernel32, "WriteFile"));
    EXPECT_EQ((FARPROC)fp.fp_BCryptGenRandom, GetProcAddress(h_bcrypt, "BCryptGenRandom"));
}

TEST(WsdApiUtilTest, TestFilePathHasLeaf) {
    EXPECT_TRUE(FilePathHasLeaf(L"abc123", L"abc123"));
    EXPECT_TRUE(FilePathHasLeaf(L"aBc123", L"AbC123"));
    EXPECT_TRUE(FilePathHasLeaf(L"A\\B\\C\\123", L"123"));
    EXPECT_TRUE(FilePathHasLeaf(L"A\\B\\C123", L"c123"));
    EXPECT_TRUE(FilePathHasLeaf(L"A\\B\\C123", L"C123"));
    EXPECT_TRUE(FilePathHasLeaf(L"C:\\Windows\\System32\\test.exe", L"test.exe"));
    EXPECT_TRUE(FilePathHasLeaf(L"C:\\Windows\\System32\\test.exe", L"TEST.EXE"));

    EXPECT_FALSE(FilePathHasLeaf(L"abc123", L"abc124"));
    EXPECT_FALSE(FilePathHasLeaf(L"aBc123", L"Abd123"));
    EXPECT_FALSE(FilePathHasLeaf(L"A\\B\\C\\123", L"123.txt"));
    EXPECT_FALSE(FilePathHasLeaf(L"A\\B\\C123.txt", L"c123"));
    EXPECT_FALSE(FilePathHasLeaf(L"A\\B\\C123", L"b\\c123"));
    EXPECT_FALSE(FilePathHasLeaf(L"C:\\Windows\\System32\\test.exe", L"test"));
    EXPECT_FALSE(FilePathHasLeaf(L"C:\\Windows\\System32\\test.exe", L"TEST."));
}
