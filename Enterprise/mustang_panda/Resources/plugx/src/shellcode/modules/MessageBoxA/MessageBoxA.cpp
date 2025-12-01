#include "module_registry.hpp"
#include "MessageBoxA.hpp"
#include <al/import.hpp>
#include "fnv1a.hpp"

// Populates function pointers for use throughout shellcode
DWORD FetchFunctions(messagebox_pointers* fp) {
    auto kernel32 = GM(L"KERNEL32.DLL", by_ror13);
    if (!kernel32) {
        return FAIL_GET_KERNEL32;
    }

    auto llw = GP(kernel32, LoadLibraryW, by_fnv1a);
    if (!llw) {
        return FAIL_GET_LOADLIBRARYW;
    }

    HMODULE user32 = llw(L"user32.dll"_xor);
    if (!user32) {
        return FAIL_LOAD_USER32;
    }

    fp->fp_MessageBoxA = GP(user32, MessageBoxA, by_fnv1a);
    if (!(fp->fp_MessageBoxA)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_MESSAGEBOXA;
    }
    return ERROR_SUCCESS;
}

void HandleMessageBoxA(void* data, void* context, logger_ctx* log_ctx) {
    messagebox_pointers* ctx = (messagebox_pointers*)context;
    module_context_t* m_ctx = (module_context_t*)data;

    if (ctx && ctx->fp_MessageBoxA) {
        const char* msg = (m_ctx->argc > 0) ? m_ctx->argv[0] : "";
        ctx->fp_MessageBoxA(NULL, msg, "", MB_OK | MB_ICONINFORMATION);
        return;
    }
    AesLogger::LogError(log_ctx, "Could not create message box. Missing context or MessageBoxA function"_xor);
    return;
}

DWORD RegisterModule_MessageBoxA(sh_context* ctx, uint32_t module_id) {
    messagebox_pointers* m_ctx = (messagebox_pointers*)ctx->fp.fp_malloc(sizeof(messagebox_pointers));
    if (!m_ctx) {
        AesLogger::LogError(ctx->log_ctx, "Failed to allocate memory for module with ID 0x%x"_xor, module_id);
        return CUSTOM_MODULE_MESSAGEBOXA_FAIL_MALLOC_STRUCT;
    }

    DWORD result = ERROR_SUCCESS;

    result = FetchFunctions(m_ctx);
    if (result != ERROR_SUCCESS) {
        ctx->fp.fp_free(m_ctx);
        AesLogger::LogError(ctx->log_ctx, "Failed to fetch functions for module with ID 0x%x. Error code: %d"_xor, module_id, result);
        return CUSTOM_MODULE_MESSAGEBOXA_FAIL_GET_POINTERS;
    }


    result = RegisterModule(ctx, module_id, HandleMessageBoxA, m_ctx);
    if (result != ERROR_SUCCESS) {
        ctx->fp.fp_free(m_ctx);
        AesLogger::LogError(ctx->log_ctx, "Failed to register module ID 0x%x. Error code: %d"_xor, module_id, result);
        return CUSTOM_MODULE_MESSAGEBOXA_FAIL_REGISTER_MODULE;
    }

    AesLogger::LogSuccess(ctx->log_ctx, "Successfully registered module 0x%x."_xor, module_id);

    return result;
}
