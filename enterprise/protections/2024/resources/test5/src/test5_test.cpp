#include "encrypt.hpp"
#include "enum.hpp"
#include <fstream>
#include <gtest/gtest.h>

class ProtectionsTest5Tests : public ::testing::Test {

protected:
    std::wstring root_dir = L"test5_unit_test_env";

    void createDummyFile(std::string path) {
        std::ofstream ofs(path);
        ofs << "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
        ofs.close();
    }

    void setupTestEnvironment() {
        cleanupTestEnvironment();

        std::vector<std::string> directories = {
            "appdata",
            "MyStuff",
            "common files",
            "Vacation Pictures"
        };


        std::vector<std::string> files = {
            "test.txt",
            "forbidden.txt",
            "forbidden.pdf",
            "forbidden",
            "allowed.pdf",
        };

        // Create directories and files
        std::filesystem::create_directories(std::filesystem::current_path().string() + "\\" + "test5_unit_test_env");
        for (std::string dir : directories) {
            std::string full_dir_path = std::filesystem::current_path().string() + "\\" + "test5_unit_test_env" + "\\" + dir;
            std::filesystem::create_directories(full_dir_path);

            // Standard files
            for (std::string f : files) {
                createDummyFile(full_dir_path + "\\" + f);
            }

            // System files
            std::string sys_file(full_dir_path + "\\sys.txt");
            createDummyFile(sys_file);
            SetFileAttributesA(sys_file.c_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);


            // Symlink
            try {
                std::filesystem::create_symlink(full_dir_path + "\\test.txt", full_dir_path + "\\sym");
            }
            catch (const std::exception& ex) {
                std::string msg(ex.what());
                if (msg.find("A required privilege is not held by the client") != std::string::npos) {
                    std::cout << "Run as admin to test with symlinks." << std::endl;
                }
                else {
                    throw std::runtime_error(ex.what());
                }
            }

        }

        // Create some directory symlinks
        try {
            std::filesystem::create_directory_symlink(
                std::filesystem::current_path().string() + "\\test5_unit_test_env\\a",
                std::filesystem::current_path().string() + "\\test5_unit_test_env\\linkdir"
            );
        }
        catch (const std::exception& ex) {
            std::string msg(ex.what());
            if (msg.find("A required privilege is not held by the client") != std::string::npos) {
                std::cout << "Run as admin to test with directory symlinks." << std::endl;
            }
            else {
                throw std::runtime_error(ex.what());
            }
        }
    }

    void cleanupTestEnvironment() {
        std::filesystem::remove_all("test5_unit_test_env");
    }

    virtual void SetUp() {
        setupTestEnvironment();
    }

    virtual void TearDown() {
        cleanupTestEnvironment();
    }
};


TEST_F(ProtectionsTest5Tests, TestEnumerateFiles) {

    std::vector<std::string> files = enumeration::EnumerateFilesystemPath(std::filesystem::current_path().string() + "\\test5_unit_test_env\\MyStuff\\");
    std::set< std::string > result(files.begin(), files.end());
    std::vector<std::string> expected = {
        std::filesystem::current_path().string() + "\\test5_unit_test_env\\MyStuff\\test.txt",
        std::filesystem::current_path().string() + "\\test5_unit_test_env\\MyStuff\\forbidden.txt",
        std::filesystem::current_path().string() + "\\test5_unit_test_env\\MyStuff\\forbidden.pdf",
        std::filesystem::current_path().string() + "\\test5_unit_test_env\\MyStuff\\forbidden",
        std::filesystem::current_path().string() + "\\test5_unit_test_env\\MyStuff\\allowed.pdf"
    };
    std::set<std::string> want(expected.begin(), expected.end());
    ASSERT_EQ(want, result);
}
