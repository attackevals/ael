#include "guardrails.hpp"
#include "execute.hpp"
#include "util/string_util.hpp"
#include <map>
#include <gtest/gtest.h>

class LangTests : public ::testing::Test {
protected:
    std::map<std::string, LANGID> lang_str_id_map = {
        {"en", 0x0009},
        {"en-AU", 0x0C09},
        {"en-CA", 0x1009},
        {"en-US", 0x0409},
        {"en-GB", 0x0809},
        {"en-UM", 0x1000},
        {"en-VI", 0x1000},
        {"en-150", 0x1000},
        {"en-IE", 0x1809},
        {"en-NZ", 0x1409},
        {"en-ZA", 0x1C09},
    };

    void GetInstalledLanguage(LANGID* lang_id) {
        wchar_t command_line[] = L"powershell.exe -command \"(Get-WinSystemLocale).Name\"";
        DWORD error_code;
        DWORD exit_code;
        std::vector<char> output = execute::ExecuteProcess(command_line, 5, &error_code, &exit_code); 
        ASSERT_EQ(error_code, ERROR_SUCCESS);
        ASSERT_EQ(exit_code, 0);
        std::string lang_str = string_util::rtrim(std::string(output.begin(), output.end()));
        std::cout << "Get-InstalledLanguage returned " << lang_str << std::endl;
        *lang_id = lang_str_id_map[lang_str];
    }

    void GetPreferredUILanguage(LANGID* lang_id) {
        wchar_t command_line[] = L"powershell.exe -command \"(Get-UICulture).Name\"";
        DWORD error_code;
        DWORD exit_code;
        std::vector<char> output = execute::ExecuteProcess(command_line, 5, &error_code, &exit_code); 
        ASSERT_EQ(error_code, ERROR_SUCCESS);
        ASSERT_EQ(exit_code, 0);
        std::string lang_str = string_util::rtrim(std::string(output.begin(), output.end()));
        std::cout << "Get-SystemPreferredUILanguage returned " << lang_str << std::endl;
        *lang_id = lang_str_id_map[lang_str];
    }
};

TEST_F(LangTests, TestSystemHasExcludedLang) {
    LANGID installed_lang_id = 0;
    LANGID preferred_lang_id = 0;
    GetInstalledLanguage(&installed_lang_id);
    GetPreferredUILanguage(&preferred_lang_id);
    std::set<LANGID> exclusion_set = {
        installed_lang_id
    };

    EXPECT_TRUE(guardrails::SystemHasExcludedLang(std::set<LANGID>{installed_lang_id}));
    EXPECT_TRUE(guardrails::SystemHasExcludedLang(std::set<LANGID>{preferred_lang_id}));
    EXPECT_FALSE(guardrails::SystemHasExcludedLang(std::set<LANGID>{0x0}));
}

TEST_F(LangTests, TestExclusionSet) {
    std::set<LANGID> want = {
        0x2801, // ar-SY - Arabic (Syria)
        0x0819, // ru-MD - Russian (Moldova)
        0x042B, // hy-AM - Armenian (Armenia)
        0x0419, // ru-RU - Russian (Russia)
        0x742C, // az-Cyrl - Azerbaijani (Cyrillic Azerbaijani)
        0x0428, // tg-Cyrl-TJ - Tajik (Cyrillic Tajikistan)
        0x782C, // az-Latn - Azerbaijani (Latin Azerbaijani)
        0x042C, // az-Latn-AZ - Azerbaijani (Latin Azerbaijani (Azerbaijan))
        0x0442, // tk-TM - Turkmenistan (Turkmenistan)
        0x0423, // be-BY - Belarusian (Belarus)
        0x0444, // tt-RU - Tatar (Russia)
        0x0437, // ka-GE - Georgian (Georgia)
        0x0422, // uk-UA - Ukrainian (Ukraine)
        0x043F, // kk-KZ - Kazakh (Kazakhstan)
        0x0843, // uz-Cyrl-UZ - Uzbek (Cyrillic Uzbekistan)
        0x0440, // ky-KG - (Kyrgyzstan)
        0x0443, // uz-Latn-UZ - Uzbek (Latin Uzbekistan)
        0x0818, // ro-MD - Romanian (Moldova)
    };

    ASSERT_EQ(want, guardrails::GetDefaultLanguageExclusionSet());
}