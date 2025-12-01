#include "module_registry.hpp"
#include "module_list.hpp"

DWORD RegisterModule(sh_context* ctx, module_id_t id, module_handler_t handler, void* module_context) {
    if (ctx->moduleCount < MAX_MODULES) {
        ctx->module_table[ctx->moduleCount].id = id;
        ctx->module_table[ctx->moduleCount].handler = handler;
        ctx->module_table[ctx->moduleCount].context = module_context;
        ctx->moduleCount++;
        return ERROR_SUCCESS;
    }
    AesLogger::LogError(ctx->log_ctx, "Module count of %d is beyond the supported limit of %d modules"_xor, ctx->moduleCount, MAX_MODULES);
    return MODULE_REGISTRATION_FAILED;
}

DWORD DispatchModule(sh_context* ctx, module_id_t id, void* data) {
    for (int i = 0; i < ctx->moduleCount; i++) {
        if (ctx->module_table[i].id == id) {
            ctx->module_table[i].handler(data, ctx->module_table[i].context, ctx->log_ctx);
            return ERROR_SUCCESS;
        }
    }
    AesLogger::LogError(ctx->log_ctx, "Failed to find module 0x%x to dispatch."_xor, id);
    return MODULE_DISPATCH_FAILED;
}

int ModuleRegistered(sh_context* ctx, module_id_t id) {
    for (int i = 0; i < ctx->moduleCount; ++i) {
        if (ctx->module_table[i].id == id) {
            return 1;
        }
    }
    return 0;
}
