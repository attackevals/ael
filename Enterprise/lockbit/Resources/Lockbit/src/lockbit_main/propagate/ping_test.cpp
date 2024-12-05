#include <vector>
#include "propagate.hpp"
#include <gtest/gtest.h>


TEST(PropagateTests, TestPingLocalhost) {
    std::vector<std::string> targets = {
        "127.0.0.1",
        "127.0.0.2",
        "127.0.0.3",
        "127.0.0.4"
    };
    for (std::string t: targets) {
        ASSERT_TRUE(propagate::PingRemoteHost(t));
    }
}