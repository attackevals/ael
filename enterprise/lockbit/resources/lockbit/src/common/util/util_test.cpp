#include "util/network_util.hpp"
#include "util/os_util.hpp"
#include "util/string_util.hpp"
#include <gtest/gtest.h>


TEST(UtilTests, TestIntPow) {
    EXPECT_EQ(network_util::IntPow(0, 0), 1);
    EXPECT_EQ(network_util::IntPow(1, 0), 1);
    EXPECT_EQ(network_util::IntPow(10, 0), 1);
    EXPECT_EQ(network_util::IntPow(1, 0), 1);
    EXPECT_EQ(network_util::IntPow(1, 2), 1);
    EXPECT_EQ(network_util::IntPow(1, 100), 1);
    EXPECT_EQ(network_util::IntPow(3, 1), 3);
    EXPECT_EQ(network_util::IntPow(3, 2), 9);
    EXPECT_EQ(network_util::IntPow(3, 3), 27);
    EXPECT_EQ(network_util::IntPow(3, 10), 59049);
    EXPECT_EQ(network_util::IntPow(10, 3), 1000);
    EXPECT_EQ(network_util::IntPow(2, 10), 1024);
}

TEST(UtilTests, TestTrimSuffix) {
    EXPECT_EQ(string_util::trim_suffix(L"test.exe", L".exe"), std::wstring(L"test"));
    EXPECT_EQ(string_util::trim_suffix(L"test.exe", L""), std::wstring(L"test.exe"));
    EXPECT_EQ(string_util::trim_suffix(L"test.exe", L"e"), std::wstring(L"test.ex"));
    EXPECT_EQ(string_util::trim_suffix(L"test.exe", L"f"), std::wstring(L"test.exe"));
}

TEST(IpUtilTests, TestIPAddrToString) {
    std::vector<std::string> addrs = {
        "10.0.0.1",
        "127.123.4.2",
        "255.255.255.255",
        "1.2.3.4"
    };
    for (std::string a: addrs) {
        EXPECT_EQ(network_util::IPAddrToString(inet_addr(a.c_str())), a);
    }
}

TEST(IpUtilTests, VerifyINAddr) {
    EXPECT_EQ(inet_addr("1.2.3.4"), 0x04030201);
    EXPECT_EQ(inet_addr("10.0.2.11"), 0x0b02000a);
    IN_ADDR in_addr;
    in_addr.S_un.S_addr = inet_addr("1.2.3.4");
    EXPECT_EQ(in_addr.S_un.S_un_b.s_b1, 1);
    EXPECT_EQ(in_addr.S_un.S_un_b.s_b2, 2);
    EXPECT_EQ(in_addr.S_un.S_un_b.s_b3, 3);
    EXPECT_EQ(in_addr.S_un.S_un_b.s_b4, 4);
    EXPECT_EQ(in_addr.S_un.S_un_w.s_w1, 0x0201);
    EXPECT_EQ(in_addr.S_un.S_un_w.s_w2, 0x0403);
    EXPECT_EQ(in_addr.S_un.S_addr, 0x04030201);
}

TEST(IpUtilTests, TestGetNetMaskSize) {
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.255.255"), 32);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.255.254"), 31);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.255.252"), 30);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.255.248"), 29);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.255.240"), 28);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.255.224"), 27);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.255.192"), 26);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.255.128"), 25);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.255.0"), 24);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.254.0"), 23);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.252.0"), 22);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.248.0"), 21);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.240.0"), 20);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.224.0"), 19);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.192.0"), 18);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.128.0"), 17);
    EXPECT_EQ(network_util::GetNetMaskSize("255.255.0.0"), 16);
    EXPECT_EQ(network_util::GetNetMaskSize("255.254.0.0"), 15);
    EXPECT_EQ(network_util::GetNetMaskSize("255.252.0.0"), 14);
    EXPECT_EQ(network_util::GetNetMaskSize("255.248.0.0"), 13);
    EXPECT_EQ(network_util::GetNetMaskSize("255.240.0.0"), 12);
    EXPECT_EQ(network_util::GetNetMaskSize("255.224.0.0"), 11);
    EXPECT_EQ(network_util::GetNetMaskSize("255.192.0.0"), 10);
    EXPECT_EQ(network_util::GetNetMaskSize("255.128.0.0"), 9);
    EXPECT_EQ(network_util::GetNetMaskSize("255.0.0.0"), 8);
    EXPECT_EQ(network_util::GetNetMaskSize("254.0.0.0"), 7);
    EXPECT_EQ(network_util::GetNetMaskSize("252.0.0.0"), 6);
    EXPECT_EQ(network_util::GetNetMaskSize("248.0.0.0"), 5);
    EXPECT_EQ(network_util::GetNetMaskSize("240.0.0.0"), 4);
    EXPECT_EQ(network_util::GetNetMaskSize("224.0.0.0"), 3);
    EXPECT_EQ(network_util::GetNetMaskSize("192.0.0.0"), 2);
    EXPECT_EQ(network_util::GetNetMaskSize("128.0.0.0"), 1);
    EXPECT_EQ(network_util::GetNetMaskSize("0.0.0.0"), 0);
}

TEST(IpUtilTests, TestGetSubnetStartEnd) {
    EXPECT_EQ(network_util::GetSubnetStart(ntohl(inet_addr("1.2.3.4")), 32), 0x01020304);
    EXPECT_EQ(network_util::GetSubnetEnd(ntohl(inet_addr("1.2.3.4")), 32), 0x01020304);
    EXPECT_EQ(network_util::GetSubnetStart(ntohl(inet_addr("1.2.3.4")), 31), 0x01020304);
    EXPECT_EQ(network_util::GetSubnetEnd(ntohl(inet_addr("1.2.3.4")), 31), 0x01020305);
    EXPECT_EQ(network_util::GetSubnetStart(ntohl(inet_addr("1.2.3.4")), 30), 0x01020304);
    EXPECT_EQ(network_util::GetSubnetEnd(ntohl(inet_addr("1.2.3.4")), 30), 0x01020307);
    EXPECT_EQ(network_util::GetSubnetStart(ntohl(inet_addr("1.2.3.4")), 29), 0x01020300);
    EXPECT_EQ(network_util::GetSubnetEnd(ntohl(inet_addr("1.2.3.4")), 29), 0x01020307);
    EXPECT_EQ(network_util::GetSubnetStart(ntohl(inet_addr("1.2.3.4")), 24), 0x01020300);
    EXPECT_EQ(network_util::GetSubnetEnd(ntohl(inet_addr("1.2.3.4")), 24), 0x010203ff);
    EXPECT_EQ(network_util::GetSubnetStart(ntohl(inet_addr("1.2.3.4")), 16), 0x01020000);
    EXPECT_EQ(network_util::GetSubnetEnd(ntohl(inet_addr("1.2.3.4")), 16), 0x0102ffff);
}

TEST(IpUtilTests, TestIsLoopback) {
    EXPECT_TRUE(network_util::IsLoopback(ntohl(inet_addr("127.0.0.0"))));
    EXPECT_TRUE(network_util::IsLoopback(ntohl(inet_addr("127.0.0.1"))));
    EXPECT_TRUE(network_util::IsLoopback(ntohl(inet_addr("127.1.2.3"))));
    EXPECT_TRUE(network_util::IsLoopback(ntohl(inet_addr("127.255.255.255"))));
    EXPECT_FALSE(network_util::IsLoopback(ntohl(inet_addr("126.255.255.255"))));
    EXPECT_FALSE(network_util::IsLoopback(ntohl(inet_addr("128.0.0.0"))));
    EXPECT_FALSE(network_util::IsLoopback(ntohl(inet_addr("10.0.0.0"))));
}

TEST(IpUtilTests, TestIsMulticast) {
    EXPECT_TRUE(network_util::IsMulticast(ntohl(inet_addr("224.0.0.0"))));
    EXPECT_TRUE(network_util::IsMulticast(ntohl(inet_addr("224.0.0.1"))));
    EXPECT_TRUE(network_util::IsMulticast(ntohl(inet_addr("235.0.0.1"))));
    EXPECT_TRUE(network_util::IsMulticast(ntohl(inet_addr("239.1.2.3"))));
    EXPECT_TRUE(network_util::IsMulticast(ntohl(inet_addr("239.255.255.255"))));
    EXPECT_FALSE(network_util::IsMulticast(ntohl(inet_addr("223.255.255.255"))));
    EXPECT_FALSE(network_util::IsMulticast(ntohl(inet_addr("240.0.0.0"))));
    EXPECT_FALSE(network_util::IsMulticast(ntohl(inet_addr("10.0.0.0"))));
}

TEST(IpUtilTests, TestConvertCidrRangeStringToIPAddrs) {
    std::vector<IPAddr> want = {
        inet_addr("10.0.1.1"),
    };
    EXPECT_EQ(want, network_util::ConvertCidrRangeStringToIPAddrs("10.0.1.1/32"));
    want = {
        inet_addr("10.0.1.0"),
        inet_addr("10.0.1.1"),
        inet_addr("10.0.1.2"),
        inet_addr("10.0.1.3"),
    };
    EXPECT_EQ(want, network_util::ConvertCidrRangeStringToIPAddrs("10.0.1.0/30"));
    want = {
        inet_addr("10.0.1.0"),
        inet_addr("10.0.1.1"),
        inet_addr("10.0.1.2"),
        inet_addr("10.0.1.3"),
        inet_addr("10.0.1.4"),
        inet_addr("10.0.1.5"),
        inet_addr("10.0.1.6"),
        inet_addr("10.0.1.7"),
        inet_addr("10.0.1.8"),
        inet_addr("10.0.1.9"),
        inet_addr("10.0.1.10"),
        inet_addr("10.0.1.11"),
        inet_addr("10.0.1.12"),
        inet_addr("10.0.1.13"),
        inet_addr("10.0.1.14"),
        inet_addr("10.0.1.15"),
    };
    EXPECT_EQ(want, network_util::ConvertCidrRangeStringToIPAddrs("10.0.1.0/28"));
    auto result =  network_util::ConvertCidrRangeStringToIPAddrs("10.1.0.0/16");
    EXPECT_EQ(65536, result.size());
    EXPECT_EQ(inet_addr("10.1.0.0"), result[0]);
    EXPECT_EQ(inet_addr("10.1.255.255"), result.back());
}