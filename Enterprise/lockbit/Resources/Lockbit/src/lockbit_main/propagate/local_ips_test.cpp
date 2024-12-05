#include <vector>
#include "propagate.hpp"
#include <gtest/gtest.h>

TEST(LocalIPsTests, TestGetTargetsFromLocalCidr) {
    std::set<IPAddr> empty_strict_include;
    std::set<IPAddr> strict_include = {
        inet_addr("10.1.2.0"),
        inet_addr("10.1.2.1"),
        inet_addr("10.1.2.2"),
        inet_addr("10.1.2.3"),
        inet_addr("10.2.3.128"),
        inet_addr("10.2.3.129"),
        inet_addr("10.2.3.130"),
        inet_addr("10.2.3.131"),
        inet_addr("10.2.3.132"),
        inet_addr("10.2.3.133"),
        inet_addr("10.2.3.134"),
        inet_addr("10.2.3.135"),
    };
    std::set<IPAddr> strict_include2 = {
        inet_addr("10.1.3.0"),
        inet_addr("10.1.3.1"),
        inet_addr("10.1.3.2"),
        inet_addr("10.1.3.3")
    };
    std::vector<IPAddr> want = {
        inet_addr("10.1.2.1"),
        inet_addr("10.1.2.2"),
        inet_addr("10.1.2.3"),
        inet_addr("10.1.2.5"),
        inet_addr("10.1.2.6"),
    };
    std::vector<IPAddr> want_filtered = {
        inet_addr("10.1.2.1"),
        inet_addr("10.1.2.2"),
        inet_addr("10.1.2.3"),
    };
    EXPECT_EQ(want, propagate::GetTargetsFromLocalCidr(inet_addr("10.1.2.4"), 29, empty_strict_include));
    EXPECT_EQ(want_filtered, propagate::GetTargetsFromLocalCidr(inet_addr("10.1.2.4"), 29, strict_include));
    EXPECT_EQ(0, propagate::GetTargetsFromLocalCidr(inet_addr("10.1.2.4"), 29, strict_include2).size());

    std::vector<IPAddr> want2 = {
        inet_addr("10.2.3.129"),
        inet_addr("10.2.3.130"),
        inet_addr("10.2.3.131"),
        inet_addr("10.2.3.132"),
        inet_addr("10.2.3.133"),
        inet_addr("10.2.3.134"),
        inet_addr("10.2.3.136"),
        inet_addr("10.2.3.137"),
        inet_addr("10.2.3.138"),
        inet_addr("10.2.3.139"),
        inet_addr("10.2.3.140"),
        inet_addr("10.2.3.141"),
        inet_addr("10.2.3.142"),
    };
    std::vector<IPAddr> want2_filtered = {
        inet_addr("10.2.3.129"),
        inet_addr("10.2.3.130"),
        inet_addr("10.2.3.131"),
        inet_addr("10.2.3.132"),
        inet_addr("10.2.3.133"),
        inet_addr("10.2.3.134"),
    };
    std::vector<IPAddr> want_filtered2 = {
        inet_addr("10.1.2.0"),
        inet_addr("10.1.2.1"),
        inet_addr("10.1.2.2"),
        inet_addr("10.1.2.3"),
    };
    EXPECT_EQ(want2, propagate::GetTargetsFromLocalCidr(inet_addr("10.2.3.135"), 28, empty_strict_include));
    EXPECT_EQ(want2_filtered, propagate::GetTargetsFromLocalCidr(inet_addr("10.2.3.135"), 28, strict_include));
    EXPECT_EQ(0, propagate::GetTargetsFromLocalCidr(inet_addr("10.2.3.135"), 28, strict_include2).size());
    EXPECT_EQ(253, propagate::GetTargetsFromLocalCidr(inet_addr("10.0.0.1"), 24, empty_strict_include).size());
    EXPECT_EQ(want_filtered2, propagate::GetTargetsFromLocalCidr(inet_addr("10.1.2.4"), 16, strict_include));
    EXPECT_EQ(0, propagate::GetTargetsFromLocalCidr(inet_addr("10.0.0.1"), 24, strict_include).size());
    EXPECT_EQ(0, propagate::GetTargetsFromLocalCidr(inet_addr("127.0.0.1"), 24, empty_strict_include).size());
    EXPECT_EQ(0, propagate::GetTargetsFromLocalCidr(inet_addr("239.0.0.1"), 16, empty_strict_include).size());
    EXPECT_EQ(0, propagate::GetTargetsFromLocalCidr(inet_addr("127.0.0.1"), 24, strict_include).size());
    EXPECT_EQ(0, propagate::GetTargetsFromLocalCidr(inet_addr("239.0.0.1"), 16, strict_include).size());
}
