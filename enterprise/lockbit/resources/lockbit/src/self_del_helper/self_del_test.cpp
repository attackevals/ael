#include <filesystem>
#include "defense_evasion.hpp"
#include "test_utils.hpp"
#include <gtest/gtest.h>

class SelfDelTest : public ::testing::Test {
protected:
    std::wstring root_dir = L"lockbit_unit_test_env";
    std::string plaintext = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

    std::filesystem::path test_dir_path = std::filesystem::current_path() / std::filesystem::path("lockbit_unit_test_env");
    std::string target_file_name = "lockbit_unit_test_sd.exe";
    std::string final_file_name = "ZZZZZZZZZZZZZZZZZZZZZZZZ";
    std::filesystem::path initial_file_path = test_dir_path / std::filesystem::path(target_file_name);
    std::filesystem::path final_file_path = test_dir_path / std::filesystem::path(final_file_name);

    void setupTestEnvironment() {
        std::filesystem::create_directories(test_dir_path);
        test_utils::CreateDummyFile(initial_file_path.string(), plaintext);
    }

    void cleanupTestEnvironment() {
        std::filesystem::remove_all(test_dir_path);
    }

    virtual void SetUp() {      
        setupTestEnvironment();
    }

    virtual void TearDown() {
        cleanupTestEnvironment();
    }
};

TEST_F(SelfDelTest, TestRenameCycleFile) {
    ASSERT_EQ(defense_evasion::RenameCycleFile(initial_file_path.wstring()), final_file_path.wstring());
    EXPECT_FALSE(std::filesystem::exists(initial_file_path));
    EXPECT_TRUE(std::filesystem::exists(final_file_path));
}

TEST_F(SelfDelTest, TestOverWriteRenameAndDeleteFile) {
    ASSERT_TRUE(defense_evasion::OverWriteRenameAndDeleteFile(initial_file_path.wstring()));
    EXPECT_FALSE(std::filesystem::exists(initial_file_path));
    EXPECT_FALSE(std::filesystem::exists(final_file_path));
}