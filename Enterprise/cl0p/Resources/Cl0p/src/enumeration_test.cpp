#include <fstream>
#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>
#include <set>
#include <Windows.h>

#include "enumeration.hpp"


class EnumerateTest : public ::testing::Test {

protected:
    std::wstring root_dir = L"clop_unit_test_env";

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
            "test.exe",
            "a.lnk",
            "b.dll",
            "c.C_I0P",
            "Readme!_CI0P!.txt"
        };

        // Create directories and files
        std::filesystem::create_directories(std::filesystem::current_path().string() + "\\" + "clop_unit_test_env");
        for (std::string dir : directories) {
            std::string full_dir_path = std::filesystem::current_path().string() + "\\" + "clop_unit_test_env" + "\\" + dir;
            std::filesystem::create_directories(full_dir_path);

            // Standard files
            for (std::string f : files) {
                createDummyFile(full_dir_path + "\\" + f);
            }

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
                std::filesystem::current_path().string() + "\\clop_unit_test_env\\a",
                std::filesystem::current_path().string() + "\\clop_unit_test_env\\linkdir"
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
        std::filesystem::remove_all("clop_unit_test_env");
    }

    virtual void SetUp() {
        setupTestEnvironment();
    }

    virtual void TearDown() {
        cleanupTestEnvironment();
    }
};

std::stringstream outstream;

bool fileCallback(const std::string filepath) {
    outstream << filepath << std::endl;
    return true;
}


TEST(EnumerationTests, TestCheckDirectoryFalse) {
    EXPECT_FALSE(Enumeration::CheckValidDirectory("Windows"));
}

TEST(EnumerationTests, TestCheckDirectoryTrue) {
    EXPECT_TRUE(Enumeration::CheckValidDirectory("Foobar"));
}

TEST(EnumerationTests, TestCheckFilenameFalse) {
    EXPECT_FALSE(Enumeration::CheckValidFilename("boot.ini"));
}

TEST(EnumerationTests, TestCheckFilenameTrue) {
    EXPECT_TRUE(Enumeration::CheckValidFilename("Foo.docx"));
}

TEST(EnumerationTests, TestCheckExtensionFalse) {
    EXPECT_FALSE(Enumeration::CheckValidExtension(".exe"));
}

TEST(EnumerationTests, TestCheckExtensionTrue) {
    EXPECT_TRUE(Enumeration::CheckValidExtension(".foo"));
}

TEST_F(EnumerateTest, TestEnumerateFiles) {
    Enumeration::EnumerateFiles(std::filesystem::current_path().string() + "\\clop_unit_test_env\\MyStuff\\", fileCallback);

    std::stringstream want;
    want << std::filesystem::current_path().string() + "\\clop_unit_test_env\\MyStuff\\allowed.pdf" << std::endl;
    want << std::filesystem::current_path().string() + "\\clop_unit_test_env\\MyStuff\\forbidden" << std::endl;
    want << std::filesystem::current_path().string() + "\\clop_unit_test_env\\MyStuff\\forbidden.pdf" << std::endl;
    want << std::filesystem::current_path().string() + "\\clop_unit_test_env\\MyStuff\\forbidden.txt" << std::endl;
    want << std::filesystem::current_path().string() + "\\clop_unit_test_env\\MyStuff\\test.txt" << std::endl;
    ASSERT_EQ(want.str(), outstream.str());
}