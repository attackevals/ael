#include <set>
#include "enumerate.hpp"
#include "util/string_util.hpp"
#include "test_utils.hpp"
#include <gtest/gtest.h>

class EnumerateTest : public ::testing::Test {
protected:
    std::wstring root_dir = L"lockbit_unit_test_env";
    std::string plaintext = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

    std::vector<std::wstring> want_target_files = {
        L"lockbit_unit_test_env\\test.txt",
        L"lockbit_unit_test_env\\a\\test.txt",
        L"lockbit_unit_test_env\\a\\1\\test.txt",
        L"lockbit_unit_test_env\\a\\2\\test.txt",
        L"lockbit_unit_test_env\\b\\test.txt",
        L"lockbit_unit_test_env\\b\\1\\test.txt",
        L"lockbit_unit_test_env\\b\\2\\test.txt",
        L"lockbit_unit_test_env\\a\\1\\one\\test.txt",
        L"lockbit_unit_test_env\\a\\1\\two\\test.txt",
        L"lockbit_unit_test_env\\a\\1\\three\\test.txt",
        L"lockbit_unit_test_env\\a\\2\\one\\test.txt",
        L"lockbit_unit_test_env\\a\\2\\two\\test.txt",
        L"lockbit_unit_test_env\\a\\2\\three\\test.txt",
        L"lockbit_unit_test_env\\b\\1\\one\\test.txt",
        L"lockbit_unit_test_env\\b\\1\\two\\test.txt",
        L"lockbit_unit_test_env\\b\\1\\three\\test.txt",
        L"lockbit_unit_test_env\\b\\2\\one\\test.txt",
        L"lockbit_unit_test_env\\b\\2\\two\\test.txt",
        L"lockbit_unit_test_env\\b\\2\\three\\test.txt",
        L"lockbit_unit_test_env\\forbidden.pdf",
        L"lockbit_unit_test_env\\a\\forbidden.pdf",
        L"lockbit_unit_test_env\\a\\1\\forbidden.pdf",
        L"lockbit_unit_test_env\\a\\2\\forbidden.pdf",
        L"lockbit_unit_test_env\\b\\forbidden.pdf",
        L"lockbit_unit_test_env\\b\\1\\forbidden.pdf",
        L"lockbit_unit_test_env\\b\\2\\forbidden.pdf",
        L"lockbit_unit_test_env\\a\\1\\one\\forbidden.pdf",
        L"lockbit_unit_test_env\\a\\1\\two\\forbidden.pdf",
        L"lockbit_unit_test_env\\a\\1\\three\\forbidden.pdf",
        L"lockbit_unit_test_env\\a\\2\\one\\forbidden.pdf",
        L"lockbit_unit_test_env\\a\\2\\two\\forbidden.pdf",
        L"lockbit_unit_test_env\\a\\2\\three\\forbidden.pdf",
        L"lockbit_unit_test_env\\b\\1\\one\\forbidden.pdf",
        L"lockbit_unit_test_env\\b\\1\\two\\forbidden.pdf",
        L"lockbit_unit_test_env\\b\\1\\three\\forbidden.pdf",
        L"lockbit_unit_test_env\\b\\2\\one\\forbidden.pdf",
        L"lockbit_unit_test_env\\b\\2\\two\\forbidden.pdf",
        L"lockbit_unit_test_env\\b\\2\\three\\forbidden.pdf",
        L"lockbit_unit_test_env\\allowed.pdf",
        L"lockbit_unit_test_env\\a\\allowed.pdf",
        L"lockbit_unit_test_env\\a\\1\\allowed.pdf",
        L"lockbit_unit_test_env\\a\\2\\allowed.pdf",
        L"lockbit_unit_test_env\\b\\allowed.pdf",
        L"lockbit_unit_test_env\\b\\1\\allowed.pdf",
        L"lockbit_unit_test_env\\b\\2\\allowed.pdf",
        L"lockbit_unit_test_env\\a\\1\\one\\allowed.pdf",
        L"lockbit_unit_test_env\\a\\1\\two\\allowed.pdf",
        L"lockbit_unit_test_env\\a\\1\\three\\allowed.pdf",
        L"lockbit_unit_test_env\\a\\2\\one\\allowed.pdf",
        L"lockbit_unit_test_env\\a\\2\\two\\allowed.pdf",
        L"lockbit_unit_test_env\\a\\2\\three\\allowed.pdf",
        L"lockbit_unit_test_env\\b\\1\\one\\allowed.pdf",
        L"lockbit_unit_test_env\\b\\1\\two\\allowed.pdf",
        L"lockbit_unit_test_env\\b\\1\\three\\allowed.pdf",
        L"lockbit_unit_test_env\\b\\2\\one\\allowed.pdf",
        L"lockbit_unit_test_env\\b\\2\\two\\allowed.pdf",
        L"lockbit_unit_test_env\\b\\2\\three\\allowed.pdf"
    };

    std::vector<std::wstring> want_ransom_notes = {
        L"lockbit_unit_test_env\\testlbext.README.txt",
        L"lockbit_unit_test_env\\a\\testlbext.README.txt",
        L"lockbit_unit_test_env\\a\\1\\testlbext.README.txt",
        L"lockbit_unit_test_env\\a\\2\\testlbext.README.txt",
        L"lockbit_unit_test_env\\b\\testlbext.README.txt",
        L"lockbit_unit_test_env\\b\\1\\testlbext.README.txt",
        L"lockbit_unit_test_env\\b\\2\\testlbext.README.txt",
        L"lockbit_unit_test_env\\a\\1\\one\\testlbext.README.txt",
        L"lockbit_unit_test_env\\a\\1\\two\\testlbext.README.txt",
        L"lockbit_unit_test_env\\a\\1\\three\\testlbext.README.txt",
        L"lockbit_unit_test_env\\a\\2\\one\\testlbext.README.txt",
        L"lockbit_unit_test_env\\a\\2\\two\\testlbext.README.txt",
        L"lockbit_unit_test_env\\a\\2\\three\\testlbext.README.txt",
        L"lockbit_unit_test_env\\b\\1\\one\\testlbext.README.txt",
        L"lockbit_unit_test_env\\b\\1\\two\\testlbext.README.txt",
        L"lockbit_unit_test_env\\b\\1\\three\\testlbext.README.txt",
        L"lockbit_unit_test_env\\b\\2\\one\\testlbext.README.txt",
        L"lockbit_unit_test_env\\b\\2\\two\\testlbext.README.txt",
        L"lockbit_unit_test_env\\b\\2\\three\\testlbext.README.txt"
    };

    std::vector<std::string> directories = {
        "lockbit_unit_test_env",
        "lockbit_unit_test_env\\a",
        "lockbit_unit_test_env\\a\\1",
        "lockbit_unit_test_env\\a\\2",
        "lockbit_unit_test_env\\a\\3",
        "lockbit_unit_test_env\\b",
        "lockbit_unit_test_env\\b\\1",
        "lockbit_unit_test_env\\b\\2",
        "lockbit_unit_test_env\\b\\3",
        "lockbit_unit_test_env\\c",
        "lockbit_unit_test_env\\c\\1",
        "lockbit_unit_test_env\\c\\2",
        "lockbit_unit_test_env\\c\\3",
        "lockbit_unit_test_env\\a\\1\\one",
        "lockbit_unit_test_env\\a\\1\\two",
        "lockbit_unit_test_env\\a\\1\\three",
        "lockbit_unit_test_env\\a\\2\\one",
        "lockbit_unit_test_env\\a\\2\\two",
        "lockbit_unit_test_env\\a\\2\\three",
        "lockbit_unit_test_env\\a\\3\\one",
        "lockbit_unit_test_env\\a\\3\\two",
        "lockbit_unit_test_env\\a\\3\\three",
        "lockbit_unit_test_env\\b\\1\\one",
        "lockbit_unit_test_env\\b\\1\\two",
        "lockbit_unit_test_env\\b\\1\\three",
        "lockbit_unit_test_env\\b\\2\\one",
        "lockbit_unit_test_env\\b\\2\\two",
        "lockbit_unit_test_env\\b\\2\\three",
        "lockbit_unit_test_env\\b\\3\\one",
        "lockbit_unit_test_env\\b\\3\\two",
        "lockbit_unit_test_env\\b\\3\\three",
        "lockbit_unit_test_env\\c\\1\\one",
        "lockbit_unit_test_env\\c\\1\\two",
        "lockbit_unit_test_env\\c\\1\\three",
        "lockbit_unit_test_env\\c\\2\\one",
        "lockbit_unit_test_env\\c\\2\\two",
        "lockbit_unit_test_env\\c\\2\\three",
        "lockbit_unit_test_env\\c\\3\\one",
        "lockbit_unit_test_env\\c\\3\\two",
        "lockbit_unit_test_env\\c\\3\\three"
    };

    std::vector<std::string> all_files = {
        "test.txt",
        "forbidden.txt",
        "forbidden.pdf",
        "forbidden",
        "allowed.pdf",
        "test.exe",
        "a.bin",
        "sys.txt",
        "ro.txt",
        "sym"
    };

    void setupTestEnvironment() {
        cleanupTestEnvironment();
        
        std::vector<std::string> files = {
            "test.txt",
            "forbidden.txt",
            "forbidden.pdf",
            "forbidden",
            "allowed.pdf",
            "test.exe",
            "a.bin"
        };

        // Create directories and files
        for (const std::string& dir: directories) {
            std::string full_dir_path = std::filesystem::current_path().string() + "\\" + dir;
            std::filesystem::create_directories(full_dir_path);

            // Standard files
            for (const std::string& f: files) {
                test_utils::CreateDummyFile(full_dir_path + "\\" + f, plaintext);
            }

            // System files
            std::string sys_file(full_dir_path + "\\sys.txt");
            test_utils::CreateDummyFile(sys_file, plaintext);
            SetFileAttributesW(string_util::string_to_wstring(sys_file).c_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);

            // Readonly files
            std::string readonly(full_dir_path + "\\ro.txt");
            test_utils::CreateDummyFile(readonly, plaintext);
            SetFileAttributesW(string_util::string_to_wstring(readonly).c_str(), FILE_ATTRIBUTE_READONLY);

            // Symlink
            try {
                std::filesystem::create_symlink(full_dir_path + "\\test.txt", full_dir_path + "\\sym");
            } catch (const std::exception& ex) {
                std::string msg(ex.what());
                if (msg.find("A required privilege is not held by the client") != std::string::npos) {
                    std::cout << "Run as admin to test with symlinks." << std::endl;
                } else {
                    throw std::runtime_error(ex.what());
                }
            }
        }

        // Create some directory symlinks
        try {
            std::filesystem::create_directory_symlink(
                std::filesystem::current_path().string() + "\\lockbit_unit_test_env\\a", 
                std::filesystem::current_path().string() + "\\lockbit_unit_test_env\\linkdir"
            );
        } catch (const std::exception& ex) {
            std::string msg(ex.what());
            if (msg.find("A required privilege is not held by the client") != std::string::npos) {
                std::cout << "Run as admin to test with directory symlinks." << std::endl;
            } else {
                throw std::runtime_error(ex.what());
            }
        }
    }

    void cleanupTestEnvironment() {
        std::filesystem::remove_all("lockbit_unit_test_env");
    }

    virtual void SetUp() {      
        setupTestEnvironment();
    }

    virtual void TearDown() {
        cleanupTestEnvironment();
    }
};

TEST_F(EnumerateTest, TestEnumerate) {
    Config cfg(R"(
    {
        "enable_enc": false,
        "enable_prop": true,
        "excluded_filenames": [
            "forbidden",
            "forbidden.txt"
        ],
        "excluded_dirs": [
            "c",
            "3"
        ],
        "excluded_ext": [
            ".exe", 
            ".bin"
        ],
        "extension": "testlbext",
        "note_contents": "dummy ransom note contents",
        "rsa_pub": ""
    }
    )");
    std::vector<std::wstring> dir_paths = { std::filesystem::current_path().wstring() + L"\\" + root_dir };
    enumerate::EnumerateAndEncrypt(cfg, dir_paths, true, LB_ENUMERATE_ONLY);
    std::vector<std::wstring> processed = enumerate::GetProcessedFiles();
    std::set<std::wstring> result(processed.begin(), processed.end());
    std::set<std::wstring> want;
    std::vector<char> want_contents;
    std::vector<char> plaintext_vec(plaintext.begin(), plaintext.end());
    for (int i = 0; i < 20; i++) {
        want_contents.insert(want_contents.end(), plaintext_vec.begin(), plaintext_vec.end());
    }
    for (const std::wstring& f: want_target_files) {
        std::wstring full_path = std::filesystem::current_path().wstring() + L"\\" + f;
        want.insert(full_path);

        // Make sure files are untouched
        test_utils::CompareFileContents(full_path, want_contents);
    }
    EXPECT_EQ(want, result);

    // Verify ransom notes not created when just enumerating
    for (const std::wstring& r: want_ransom_notes) {
        EXPECT_FALSE(std::filesystem::exists(std::filesystem::current_path().wstring() + L"\\" + r));
    }
}

TEST_F(EnumerateTest, TestEnumerateAndEncrypt) {
    Config cfg(R"(
    {
        "enable_enc": true,
        "enable_prop": false,
        "excluded_filenames": [
            "forbidden",
            "forbidden.txt"
        ],
        "excluded_dirs": [
            "c",
            "3"
        ],
        "excluded_ext": [
            ".exe", 
            ".bin"
        ],
        "extension": "testlbext",
        "note_contents": "dummy ransom note contents",
        "rsa_pub": ""
    }
    )");
    Config cleanup_cfg(R"({
        "enable_enc": true,
        "enable_prop": false,
        "excluded_filenames": [
            "forbidden",
            "forbidden.txt"
        ],
        "excluded_dirs": [
            "c",
            "3"
        ],
        "excluded_ext": [
            ".exe", 
            ".bin"
        ],
        "extension": "testlbext",
        "note_contents": "dummy ransom note contents",
        "rsa_pub": "",
        "rsa_priv": ""
    })");
    std::vector<std::wstring> dir_paths = { std::filesystem::current_path().wstring() + L"\\" + root_dir };
    enumerate::EnumerateAndEncrypt(cfg, dir_paths, true, LB_ENCRYPT_FILES);
    std::vector<std::wstring> processed = enumerate::GetProcessedFiles();
    std::set<std::wstring> result(processed.begin(), processed.end());
    std::set<std::wstring> want;
    for (const std::wstring& f: want_target_files) {
        std::wstring full_path = std::filesystem::current_path().wstring() + L"\\" + f;
        want.insert(full_path);
        std::wstring new_file_path = full_path + L".testlbext";

        // Check for extension
        EXPECT_TRUE(std::filesystem::exists(new_file_path));
        EXPECT_FALSE(std::filesystem::exists(f));
    }
    EXPECT_EQ(want, result);

    // Verify ransom notes created when encrypting
    std::string want_content = "dummy ransom note contents";
    std::vector<char> want_content_vec(want_content.begin(), want_content.end());
    for (const std::wstring& r: want_ransom_notes) {
        std::wstring path = std::filesystem::current_path().wstring() + L"\\" + r;
        EXPECT_TRUE(std::filesystem::exists(path));
        test_utils::CompareFileContents(path, want_content_vec);
    }
}