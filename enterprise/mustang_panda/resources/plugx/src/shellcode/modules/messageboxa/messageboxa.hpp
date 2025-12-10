#include "shellcode.hpp"

#define CUSTOM_MODULE_MESSAGEBOXA_FAIL_MALLOC_STRUCT    0x1001
#define CUSTOM_MODULE_MESSAGEBOXA_FAIL_GET_POINTERS     0x1002
#define CUSTOM_MODULE_MESSAGEBOXA_FAIL_REGISTER_MODULE  0x1003
#define ERROR_GET_POINTER_CUSTOM_MODULE_MESSAGEBOXA     0x1004

DWORD RegisterModule_MessageBoxA(sh_context* ctx, uint32_t module_id);

using MessageBoxA_t = decltype(&MessageBoxA);

struct messagebox_pointers {
    MessageBoxA_t fp_MessageBoxA;
};
