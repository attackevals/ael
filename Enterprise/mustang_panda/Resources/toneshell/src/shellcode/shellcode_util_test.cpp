#include "shellcode_util.hpp"
#include <gtest/gtest.h>
#include "al/hash.hpp"
#include "fnv1a.hpp"

TEST(ShellcodeUtilTest, TestHashes) {
    // Case-insensitive: everything to lowercase
    // Values obtained from https://www.convertcase.com/hashing/djb-hash-calculator
    EXPECT_EQ(al::djb2("testvalue"), 0x9c06c6a2);
    EXPECT_EQ(al::djb2("kernel32.dll"), 0x7040ee75);
    EXPECT_EQ(al::djb2("KERNEL32.DLL"), 0x7040ee75);
    EXPECT_EQ(al::djb2("SetHandleInformation"), 0xfca2e263);
    EXPECT_EQ(al::djb2("sethandleinformation"), 0xfca2e263);
    EXPECT_EQ(al::djb2("ABC123"), 0xf147ebc1);
    EXPECT_EQ(al::djb2("abc123"), 0xf147ebc1);

    // Case-insensitive: everything to uppercase
    // Values obtained from https://asecuritysite.com/hash/ror13_2
    EXPECT_EQ(al::ror13("testvalue"), 0xD9F97C0F);
    EXPECT_EQ(al::ror13("TESTVALUE"), 0xD9F97C0F);
    EXPECT_EQ(al::ror13("kernel32.dll"), 0x6E2BCA17);
    EXPECT_EQ(al::ror13("KERNEL32.DLL"), 0x6E2BCA17);
    EXPECT_EQ(al::ror13("SetHandleInformation"), 0x5DA8210);
    EXPECT_EQ(al::ror13("SETHANDLEINFORMATION"), 0x5DA8210);
    EXPECT_EQ(al::ror13("ABC123"), 0x7942C94);
    EXPECT_EQ(al::ror13("abc123"), 0x7942C94);

    // Case-sensitive
    // Values obtained from https://md5calc.com/hash/fnv1a32
    EXPECT_EQ(cx_fnv_hash("testvalue"), 0x900e756a);
    EXPECT_EQ(cx_fnv_hash("TESTVALUE"), 0x54fbbf0a);
    EXPECT_EQ(cx_fnv_hash("kernel32.dll"), 0xa3e6f6c3);
    EXPECT_EQ(cx_fnv_hash("KERNEL32.DLL"), 0x29cdd463);
    EXPECT_EQ(cx_fnv_hash("SetHandleInformation"), 0x9ba12495);
    EXPECT_EQ(cx_fnv_hash("SETHANDLEINFORMATION"), 0xe408afb5);
    EXPECT_EQ(cx_fnv_hash("ABC123"), 0x45854fe5);
    EXPECT_EQ(cx_fnv_hash("abc123"), 0x38b29a05);
}

TEST(ShellcodeUtilTest, TestApiResolution) {
    func_pointers fp;
    ASSERT_EQ(FetchFunctions(&fp), ERROR_SUCCESS);
    EXPECT_EQ((FARPROC)(fp.shared_fp.fp_GetLastError), GetProcAddress(fp.h_kernel32, "GetLastError"));
    EXPECT_EQ((FARPROC)(fp.shared_fp.fp_CreateFileW), GetProcAddress(fp.h_kernel32, "CreateFileW"));
    EXPECT_EQ((FARPROC)(fp.shared_fp.fp_CloseHandle), GetProcAddress(fp.h_kernel32, "CloseHandle"));
    EXPECT_EQ((FARPROC)(fp.shared_fp.fp_vsnprintf), GetProcAddress(fp.h_ntdll, "_vsnprintf"));
    EXPECT_EQ((FARPROC)(fp.shared_fp.fp_snprintf), GetProcAddress(fp.h_ntdll, "_snprintf"));
    EXPECT_EQ((FARPROC)(fp.shared_fp.fp_GetSystemTime), GetProcAddress(fp.h_kernel32, "GetSystemTime"));
    EXPECT_EQ((FARPROC)(fp.shared_fp.fp_SetFilePointer), GetProcAddress(fp.h_kernel32, "SetFilePointer"));
    EXPECT_EQ((FARPROC)(fp.shared_fp.fp_HeapAlloc), GetProcAddress(fp.h_ntdll, "RtlAllocateHeap"));
    EXPECT_EQ((FARPROC)(fp.shared_fp.fp_HeapCreate), GetProcAddress(fp.h_kernel32, "HeapCreate"));
    EXPECT_EQ((FARPROC)(fp.shared_fp.fp_HeapDestroy), GetProcAddress(fp.h_kernel32, "HeapDestroy"));
    EXPECT_EQ((FARPROC)(fp.shared_fp.fp_HeapFree), GetProcAddress(fp.h_kernel32, "HeapFree"));
    EXPECT_EQ((FARPROC)(fp.shared_fp.fp_WriteFile), GetProcAddress(fp.h_kernel32, "WriteFile"));
    EXPECT_EQ((FARPROC)(fp.shared_fp.fp_BCryptGenRandom), GetProcAddress(fp.h_bcrypt, "BCryptGenRandom"));
    EXPECT_EQ((FARPROC)(fp.fp_WSAStartup), GetProcAddress(fp.h_ws2_32, "WSAStartup"));
    EXPECT_EQ((FARPROC)(fp.fp_WSAGetLastError), GetProcAddress(fp.h_ws2_32, "WSAGetLastError"));
    EXPECT_EQ((FARPROC)(fp.fp_WSACleanup), GetProcAddress(fp.h_ws2_32, "WSACleanup"));
    EXPECT_EQ((FARPROC)(fp.fp_socket), GetProcAddress(fp.h_ws2_32, "socket"));
    EXPECT_EQ((FARPROC)(fp.fp_send), GetProcAddress(fp.h_ws2_32, "send"));
    EXPECT_EQ((FARPROC)(fp.fp_recv), GetProcAddress(fp.h_ws2_32, "recv"));
    EXPECT_EQ((FARPROC)(fp.fp_connect), GetProcAddress(fp.h_ws2_32, "connect"));
    EXPECT_EQ((FARPROC)(fp.fp_closesocket), GetProcAddress(fp.h_ws2_32, "closesocket"));
    EXPECT_EQ((FARPROC)(fp.fp_inet_addr), GetProcAddress(fp.h_ws2_32, "inet_addr"));
    EXPECT_EQ((FARPROC)(fp.fp_htons), GetProcAddress(fp.h_ws2_32, "htons"));
    EXPECT_EQ((FARPROC)(fp.fp_GetComputerNameA), GetProcAddress(fp.h_kernel32, "GetComputerNameA"));
    EXPECT_EQ((FARPROC)(fp.fp_shutdown), GetProcAddress(fp.h_ws2_32, "shutdown"));
    EXPECT_EQ((FARPROC)(fp.fp_Sleep), GetProcAddress(fp.h_kernel32, "Sleep"));
    EXPECT_EQ((FARPROC)(fp.fp_CreateProcessW), GetProcAddress(fp.h_kernel32, "CreateProcessW"));
    EXPECT_EQ((FARPROC)(fp.fp_GetExitCodeProcess), GetProcAddress(fp.h_kernel32, "GetExitCodeProcess"));
    EXPECT_EQ((FARPROC)(fp.fp_PeekNamedPipe), GetProcAddress(fp.h_kernel32, "PeekNamedPipe"));
    EXPECT_EQ((FARPROC)(fp.fp_WaitForSingleObject), GetProcAddress(fp.h_kernel32, "WaitForSingleObject"));
    EXPECT_EQ((FARPROC)(fp.fp_ReadFile), GetProcAddress(fp.h_kernel32, "ReadFile"));
    EXPECT_EQ((FARPROC)(fp.fp_CreatePipe), GetProcAddress(fp.h_kernel32, "CreatePipe"));
    EXPECT_EQ((FARPROC)(fp.fp_SetHandleInformation), GetProcAddress(fp.h_kernel32, "SetHandleInformation"));
}

TEST(ShellcodeUtilTest, TestWidenString) {
    wchar_t buf[16];

    // fill partial dest buf
    LPCSTR src = "hello world!";
    unsigned char want1[] = {
        0x68,0x00,0x65,0x00,0x6c,0x00,0x6c,0x00,0x6f,0x00,0x20,0x00,0x77,0x00,0x6f,0x00,
        0x72,0x00,0x6c,0x00,0x64,0x00,0x21,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01
    };
    pi_memset(buf, 1, sizeof(buf));
    pi_widen_str(src, strlen(src), buf, 13);
    EXPECT_EQ(memcmp(buf, want1, sizeof(buf)), 0);
    EXPECT_STREQ(L"hello world!", buf);

    pi_memset(buf, 1, sizeof(buf));
    pi_widen_str(src, strlen(src) + 1, buf, 13);
    EXPECT_EQ(memcmp(buf, want1, sizeof(buf)), 0);
    EXPECT_STREQ(L"hello world!", buf);

    pi_memset(buf, 1, sizeof(buf));
    pi_widen_str(src, 100, buf, 13);
    EXPECT_EQ(memcmp(buf, want1, sizeof(buf)), 0);
    EXPECT_STREQ(L"hello world!", buf);

    // use partial src
    unsigned char want2[] = {
        0x68,0x00,0x65,0x00,0x6c,0x00,0x6c,0x00,0x6f,0x00,0x20,0x00,0x77,0x00,0x6f,0x00,
        0x72,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01
    };
    pi_memset(buf, 1, sizeof(buf));
    pi_widen_str(src, 9, buf, 10);
    EXPECT_EQ(memcmp(buf, want2, sizeof(buf)), 0);
    EXPECT_STREQ(L"hello wor", buf);

    // truncate with null byte
    pi_memset(buf, 1, sizeof(buf));
    pi_widen_str(src, 100, buf, 10);
    EXPECT_EQ(memcmp(buf, want2, sizeof(buf)), 0);
    EXPECT_STREQ(L"hello wor", buf);

    // fill partial dest buf, but clear all of it
    unsigned char want3[] = {
        0x68,0x00,0x65,0x00,0x6c,0x00,0x6c,0x00,0x6f,0x00,0x20,0x00,0x77,0x00,0x6f,0x00,
        0x72,0x00,0x6c,0x00,0x64,0x00,0x21,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    pi_memset(buf, 1, sizeof(buf));
    pi_widen_str(src, 100, buf, 16);
    EXPECT_EQ(memcmp(buf, want3, sizeof(buf)), 0);

    pi_memset(buf, 1, sizeof(buf));
    pi_widen_str(src, strlen(src), buf, 16);
    EXPECT_EQ(memcmp(buf, want3, sizeof(buf)), 0);

    // test long source string
    LPCSTR long_src = "hello world!helloworld!";
    unsigned char want4[] = {
        0x68,0x00,0x65,0x00,0x6c,0x00,0x6c,0x00,0x6f,0x00,0x20,0x00,0x77,0x00,0x6f,0x00,
        0x72,0x00,0x6c,0x00,0x64,0x00,0x21,0x00,0x68,0x00,0x65,0x00,0x6c,0x00,0x00,0x00
    };
    pi_memset(buf, 1, sizeof(buf));
    pi_widen_str(long_src, strlen(long_src), buf, 16);
    EXPECT_EQ(memcmp(buf, want4, sizeof(buf)), 0);
    EXPECT_STREQ(L"hello world!hel", buf);
}

TEST(ShellcodeUtilTest, TestConcatString) {
    wchar_t buf[16];

    LPCWSTR start = L"hello ";
    memcpy(buf, start, sizeof(start)*sizeof(wchar_t));

    pi_concat_wstrn(buf, sizeof(buf) / sizeof(wchar_t), L"world!");
    EXPECT_STREQ(L"hello world!", buf);

    pi_concat_wstrn(buf, sizeof(buf) / sizeof(wchar_t), L"overflow");
    EXPECT_STREQ(L"hello world!ove", buf);

    pi_concat_wstrn(buf, sizeof(buf) / sizeof(wchar_t), L"this shouldnt get added at all");
    EXPECT_STREQ(L"hello world!ove", buf);
}
