#include "execute.hpp"
#include "impact.hpp"
#include "util/string_util.hpp"
#include "test_utils.hpp"
#include <iostream>
#include <gtest/gtest.h>

class ImpactTest : public ::testing::Test {
protected:
    std::wstring extension = L"testlockbitext";
    std::wstring icon_file_name = extension + L".ico";
    std::wstring wallpaper_file_name = L"testlockbitext.bmp";
    std::string expected_icon_hash = "C15046D9D02C44B225FEDAADFD828C96651DB7FCECB69E35471A849DA6C482D6";
    std::string expected_wallpaper_hash = "690C2EE278602E7D89790EC2ECE6D20367D11DACA94141E571D2F2676520C778"; // "B22C22438B2548BBB681E8684F3DE6AE1E9CF9AA805C0638F89C6380C164092A";
    std::wstring old_wallpaper_path = L"";
    bool changed_wallpaper = false;

    void getOldWallpaperPath() {
        DWORD error_code;
        DWORD exit_code;
        wchar_t command[] = L"powershell.exe -command \"(Get-ItemProperty 'HKCU:\\Control Panel\\Desktop' -Name Wallpaper).Wallpaper\"";
        std::vector<char> output = execute::ExecuteProcess(command, 5, &error_code, &exit_code);
        ASSERT_EQ(error_code, ERROR_SUCCESS);
        ASSERT_EQ(exit_code, 0);
        std::string result(output.begin(), output.end());
        std::string old_path = string_util::rtrim(result);
        std::cout << "Old wallpaper path: " << old_path << std::endl;
        old_wallpaper_path = string_util::string_to_wstring(old_path);
    }

    void restoreWallpaperPath() {
        wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
        swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"reg.exe add \"HKCU\\Control Panel\\Desktop\" /v Wallpaper /d %s /t REG_SZ /f", old_wallpaper_path.c_str());
        DWORD error_code;
        DWORD exit_code;
        std::vector<char> output = execute::ExecuteProcess(command_line, 5, &error_code, &exit_code);
        ASSERT_EQ(error_code, ERROR_SUCCESS);
        ASSERT_EQ(exit_code, 0);
        std::string result(output.begin(), output.end());
        std::cout << "Output for restoring wallpaper reg value: " << result << std::endl;
    }

    virtual void SetUp() {
        getOldWallpaperPath();
    }

    virtual void TearDown() {
        std::filesystem::remove("C:\\ProgramData\\testlockbitext.ico");
        std::filesystem::remove("C:\\ProgramData\\testlockbitext.bmp");

        test_utils::DeleteRegKey(L"HKCU\\.testlockbitext");
        test_utils::DeleteRegKey(L"HKCU\\testlockbitext");
        test_utils::DeleteRegKey(L"HKCU\\testlockbitext");

        if (changed_wallpaper) {
            restoreWallpaperPath();
            changed_wallpaper = false;
        }
    }
};


TEST_F(ImpactTest, TestDropIconAndWallpaper) {
    EXPECT_TRUE(impact::DropIconFile(icon_file_name.c_str()));
    EXPECT_TRUE(impact::DropWallpaperFile(wallpaper_file_name.c_str()));
    test_utils::CompareFileHash(L"C:\\ProgramData\\testlockbitext.ico", expected_icon_hash);
    test_utils::CompareFileHash(L"C:\\ProgramData\\testlockbitext.bmp", expected_wallpaper_hash);
}

TEST_F(ImpactTest, TestSetIcon) {
    EXPECT_TRUE(impact::SetEncryptedFileIcon(extension));
    test_utils::CompareFileHash(L"C:\\ProgramData\\testlockbitext.ico", expected_icon_hash);
    test_utils::CompareRegContentsDefaultValue(L"HKCR\\.testlockbitext", "testlockbitext");
    test_utils::CompareRegContentsDefaultValue(L"HKCR\\testlockbitext\\DefaultIcon", "C:\\ProgramData\\testlockbitext.ico");
}

TEST_F(ImpactTest, TestChangeWallpaper) {
    bool result = impact::ChangeWallPaper(extension + L".bmp");
    changed_wallpaper = result;
    EXPECT_TRUE(result);
    test_utils::CompareFileHash(L"C:\\ProgramData\\testlockbitext.bmp", expected_wallpaper_hash);
    test_utils::CompareRegContents(L"HKCU:\\Control Panel\\Desktop", L"Wallpaper", "C:\\ProgramData\\testlockbitext.bmp");
}