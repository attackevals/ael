#pragma once
#include "shellcode.hpp"
#include <stdint.h>

DWORD RegisterModule(sh_context* ctx, module_id_t id, module_handler_t handler ,void* module_context);
DWORD DispatchModule(sh_context* ctx, module_id_t id, void* data);
int ModuleRegistered(sh_context* ctx, module_id_t id);
DWORD AutoRegisterModules(sh_context* ctx);
