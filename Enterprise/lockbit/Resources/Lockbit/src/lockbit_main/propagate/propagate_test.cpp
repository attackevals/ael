#include "propagate.hpp"
#include "util/os_util.hpp"
#include <gtest/gtest.h>

class PropagateTest : public ::testing::Test {
protected:
    Config cfg = Config(R"(
    {
        "enable_enc": false,
        "enable_prop": true,
        "excluded_filenames": [
            "file1",
            "file2",
            "file3"
        ],
        "excluded_dirs": [
            "dir1",
            "dir2"
        ],
        "excluded_ext": [],
        "psexec_user": "dummyusername",
        "psexec_pass": "dummypassword",
        "rsa_pub": "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAgYJqmqcS6LdP8vfLOExoLIjqg8jncY17F+hXYxN1CL/rdfrQmtHwiTs7xP17CzhXwNGt781vHz/6GV2ZX5R7xBzE8EbRJJx72bn2Pd3sfxcyxtQnzKpsuZHdx8GkwBzqKRQKUNi3KyZgK1mcxjZHnHgrnOJ2Mk6U6S9yt8AXaAk9F15OdolOOrUY/dHbyIqDRSs8TPbx4/XOQGFNV0G1J9/hv4UvE+T1a2h/PsLqbAEOzxfbRf8ETCG8aDtvI+lnlLTN5Z9jYCu+fKVgSvjhbGdpeOTIXCpq5dJqMQ04jzqPTAVcg77HUUyWHtb6pVr63vyuDVqhOY/WgaAM6P6JzQIDAQAB"
    }
    )");
    Config cfg_no_user = Config(R"(
    {
        "enable_enc": false,
        "enable_prop": true,
        "excluded_filenames": [
            "file1",
            "file2",
            "file3"
        ],
        "excluded_dirs": [
            "dir1",
            "dir2"
        ],
        "excluded_ext": [],
        "psexec_user": "",
        "psexec_pass": "dummypassword",
        "rsa_pub": "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAgYJqmqcS6LdP8vfLOExoLIjqg8jncY17F+hXYxN1CL/rdfrQmtHwiTs7xP17CzhXwNGt781vHz/6GV2ZX5R7xBzE8EbRJJx72bn2Pd3sfxcyxtQnzKpsuZHdx8GkwBzqKRQKUNi3KyZgK1mcxjZHnHgrnOJ2Mk6U6S9yt8AXaAk9F15OdolOOrUY/dHbyIqDRSs8TPbx4/XOQGFNV0G1J9/hv4UvE+T1a2h/PsLqbAEOzxfbRf8ETCG8aDtvI+lnlLTN5Z9jYCu+fKVgSvjhbGdpeOTIXCpq5dJqMQ04jzqPTAVcg77HUUyWHtb6pVr63vyuDVqhOY/WgaAM6P6JzQIDAQAB"
    }
    )");
    Config cfg_no_password = Config(R"(
    {
        "enable_enc": false,
        "enable_prop": true,
        "excluded_filenames": [
            "file1",
            "file2",
            "file3"
        ],
        "excluded_dirs": [
            "dir1",
            "dir2"
        ],
        "excluded_ext": [],
        "psexec_user": "dummyuser",
        "psexec_pass": "",
        "rsa_pub": "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAgYJqmqcS6LdP8vfLOExoLIjqg8jncY17F+hXYxN1CL/rdfrQmtHwiTs7xP17CzhXwNGt781vHz/6GV2ZX5R7xBzE8EbRJJx72bn2Pd3sfxcyxtQnzKpsuZHdx8GkwBzqKRQKUNi3KyZgK1mcxjZHnHgrnOJ2Mk6U6S9yt8AXaAk9F15OdolOOrUY/dHbyIqDRSs8TPbx4/XOQGFNV0G1J9/hv4UvE+T1a2h/PsLqbAEOzxfbRf8ETCG8aDtvI+lnlLTN5Z9jYCu+fKVgSvjhbGdpeOTIXCpq5dJqMQ04jzqPTAVcg77HUUyWHtb6pVr63vyuDVqhOY/WgaAM6P6JzQIDAQAB"
    }
    )");
    std::vector<std::wstring> targets = {
        L"1.2.3.4",
        L"5.6.7.8",
        L"10.0.1.2"
    };
    Config cfg_strict_ranges_and_hosts = Config(R"(
    {
        "enable_enc": false,
        "enable_prop": true,
        "excluded_filenames": [],
        "excluded_dirs": [],
        "excluded_ext": [],
        "psexec_user": "",
        "psexec_pass": "",
        "strict_include_hosts": ["10.0.1.2", "10.0.1.3"],
        "strict_include_ranges": ["10.0.1.0/29"],
        "rsa_pub": "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAgYJqmqcS6LdP8vfLOExoLIjqg8jncY17F+hXYxN1CL/rdfrQmtHwiTs7xP17CzhXwNGt781vHz/6GV2ZX5R7xBzE8EbRJJx72bn2Pd3sfxcyxtQnzKpsuZHdx8GkwBzqKRQKUNi3KyZgK1mcxjZHnHgrnOJ2Mk6U6S9yt8AXaAk9F15OdolOOrUY/dHbyIqDRSs8TPbx4/XOQGFNV0G1J9/hv4UvE+T1a2h/PsLqbAEOzxfbRf8ETCG8aDtvI+lnlLTN5Z9jYCu+fKVgSvjhbGdpeOTIXCpq5dJqMQ04jzqPTAVcg77HUUyWHtb6pVr63vyuDVqhOY/WgaAM6P6JzQIDAQAB"
    }
    )");
    Config cfg_strict_ranges = Config(R"(
    {
        "enable_enc": false,
        "enable_prop": true,
        "excluded_filenames": [],
        "excluded_dirs": [],
        "excluded_ext": [],
        "psexec_user": "",
        "psexec_pass": "",
        "strict_include_hosts": [],
        "strict_include_ranges": ["10.0.1.0/29", "10.0.2.0/29"],
        "rsa_pub": "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAgYJqmqcS6LdP8vfLOExoLIjqg8jncY17F+hXYxN1CL/rdfrQmtHwiTs7xP17CzhXwNGt781vHz/6GV2ZX5R7xBzE8EbRJJx72bn2Pd3sfxcyxtQnzKpsuZHdx8GkwBzqKRQKUNi3KyZgK1mcxjZHnHgrnOJ2Mk6U6S9yt8AXaAk9F15OdolOOrUY/dHbyIqDRSs8TPbx4/XOQGFNV0G1J9/hv4UvE+T1a2h/PsLqbAEOzxfbRf8ETCG8aDtvI+lnlLTN5Z9jYCu+fKVgSvjhbGdpeOTIXCpq5dJqMQ04jzqPTAVcg77HUUyWHtb6pVr63vyuDVqhOY/WgaAM6P6JzQIDAQAB"
    }
    )");
    Config cfg_strict_hosts = Config(R"(
    {
        "enable_enc": false,
        "enable_prop": true,
        "excluded_filenames": [],
        "excluded_dirs": [],
        "excluded_ext": [],
        "psexec_user": "",
        "psexec_pass": "",
        "strict_include_hosts": ["10.1.1.4", "10.1.1.5"],
        "strict_include_ranges": [],
        "rsa_pub": "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAgYJqmqcS6LdP8vfLOExoLIjqg8jncY17F+hXYxN1CL/rdfrQmtHwiTs7xP17CzhXwNGt781vHz/6GV2ZX5R7xBzE8EbRJJx72bn2Pd3sfxcyxtQnzKpsuZHdx8GkwBzqKRQKUNi3KyZgK1mcxjZHnHgrnOJ2Mk6U6S9yt8AXaAk9F15OdolOOrUY/dHbyIqDRSs8TPbx4/XOQGFNV0G1J9/hv4UvE+T1a2h/PsLqbAEOzxfbRf8ETCG8aDtvI+lnlLTN5Z9jYCu+fKVgSvjhbGdpeOTIXCpq5dJqMQ04jzqPTAVcg77HUUyWHtb6pVr63vyuDVqhOY/WgaAM6P6JzQIDAQAB"
    }
    )");
};

TEST_F(PropagateTest, TestGetStrictIncludePingTargets) {
    std::set<IPAddr> want = {
        inet_addr("10.0.1.0"),
        inet_addr("10.0.1.1"),
        inet_addr("10.0.1.2"),
        inet_addr("10.0.1.3"),
        inet_addr("10.0.1.4"),
        inet_addr("10.0.1.5"),
        inet_addr("10.0.1.6"),
        inet_addr("10.0.1.7"),
    };
    EXPECT_EQ(want, propagate::GetStrictIncludePingTargets(cfg_strict_ranges_and_hosts));
    want = {
        inet_addr("10.0.1.0"),
        inet_addr("10.0.1.1"),
        inet_addr("10.0.1.2"),
        inet_addr("10.0.1.3"),
        inet_addr("10.0.1.4"),
        inet_addr("10.0.1.5"),
        inet_addr("10.0.1.6"),
        inet_addr("10.0.1.7"),
        inet_addr("10.0.2.0"),
        inet_addr("10.0.2.1"),
        inet_addr("10.0.2.2"),
        inet_addr("10.0.2.3"),
        inet_addr("10.0.2.4"),
        inet_addr("10.0.2.5"),
        inet_addr("10.0.2.6"),
        inet_addr("10.0.2.7"),
    };
    EXPECT_EQ(want, propagate::GetStrictIncludePingTargets(cfg_strict_ranges));
    EXPECT_EQ(0, propagate::GetStrictIncludePingTargets(cfg_strict_hosts).size());
}

TEST_F(PropagateTest, TestFilterDiscoveredHosts) {
    std::vector<std::wstring> initial_targets = {
        L"10.0.1.1",
        L"10.0.1.2",
        L"10.0.1.3",
        L"10.0.1.4",
        L"10.0.1.5",
        L"10.0.1.6",
        L"10.0.1.7",
        L"10.0.1.8",
        L"10.0.1.9",
        L"10.0.1.10",
        L"10.0.1.11",
        L"10.0.2.1",
        L"10.0.3.2",
        L"10.0.4.3",
        L"10.0.5.4",
        L"10.0.6.5",
        L"10.0.7.6",
        L"10.0.8.7",
        L"10.0.9.8",
        L"10.0.10.9",
    };
    std::vector<std::wstring> want = {
        L"10.0.1.2",
        L"10.0.1.3",
    };
    EXPECT_EQ(want, propagate::FilterDiscoveredHosts(cfg_strict_ranges_and_hosts, initial_targets));
    EXPECT_EQ(0, propagate::FilterDiscoveredHosts(cfg_strict_hosts, initial_targets).size());
    EXPECT_EQ(initial_targets, propagate::FilterDiscoveredHosts(cfg_strict_ranges, initial_targets));
}


TEST_F(PropagateTest, TestGetPsExecCommandLine) {
    std::wstring path = os_util::GetCurrExecutablePath();
    std::wstring want = std::format(
        L"PsExec64.exe -accepteula \\\\1.2.3.4,5.6.7.8,10.0.1.2 -u dummyusername -p dummypassword -s -e -d -f -c \"{}\" -d -w",
        path
    );
    ASSERT_EQ(propagate::GetPsExecCommandLine(cfg, targets, L"dummytoken"), want);
}

TEST_F(PropagateTest, TestGetPsExecCommandLineBadArgs) {
    EXPECT_THROW({
        try {
            propagate::GetPsExecCommandLine(cfg_no_user, targets, L"dummytoken");
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("No username provided for PsExec.", e.what());
            throw;
        }
    }, std::invalid_argument);
    EXPECT_THROW({
        try {
            propagate::GetPsExecCommandLine(cfg_no_password, targets, L"dummytoken");
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("No password provided for PsExec.", e.what());
            throw;
        }
    }, std::invalid_argument);
    EXPECT_THROW({
        try {
            propagate::GetPsExecCommandLine(cfg, std::vector<std::wstring>(), L"dummytoken");
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("No remote targets provided for PsExec.", e.what());
            throw;
        }
    }, std::invalid_argument);
}
