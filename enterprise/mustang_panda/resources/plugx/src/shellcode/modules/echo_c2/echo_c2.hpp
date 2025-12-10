#include "shellcode.hpp"

#define CUSTOM_MODULE_ECHO_C2_FAIL_REGISTER_MODULE  0x9001

DWORD RegisterModule_Echo_C2(sh_context* ctx, uint32_t module_id);
