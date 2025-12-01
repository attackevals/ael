#include "module_registry.hpp"
#include "Echo_C2.hpp"
#include <al/import.hpp>


void HandleEcho_C2(void* data, void* context, logger_ctx* log_ctx) {
    module_context_t* m_ctx = (module_context_t*)data;
    sh_context* ctx = (sh_context*)context;

    size_t buffer_size = 1024;
    char* buffer = (char*)ctx->fp.fp_malloc((buffer_size));
    if (buffer == NULL) {
        AesLogger::LogError(log_ctx, "Failed to allocate memory for buffer."_xor);
        m_ctx->output = (char*)"Failed to allocate memory for buffer.";
        m_ctx->outputSize = strlen((char*)m_ctx->output);
        return;
    }

    size_t offset = 0;
    offset += ctx->fp.fp_sprintf(buffer + offset, "Arg Count: %d "_xor, m_ctx->argc);

    for (int i = 0; i < m_ctx->argc; ++i) {
        const char* arg = m_ctx->argv[i];
        size_t needed = strlen(arg) + 16;
        // resize if needed:
        if (offset + needed >= buffer_size) {
            buffer_size *= 2;
            char* new_buf = (char*)ctx->fp.fp_realloc(buffer, buffer_size);
            if (!new_buf) {
                ctx->fp.fp_free(buffer);
                AesLogger::LogError(log_ctx, "Failed to reallocate memory for buffer."_xor);
                m_ctx->output = (char*)"Failed to reallocate memory for buffer.";
                m_ctx->outputSize = strlen((char*)m_ctx->output);
                return;
            }
            buffer = new_buf;
        }

        offset += ctx->fp.fp_sprintf(buffer + offset, "arg%d: %s\n"_xor, i + 1, arg);
    }

    m_ctx->output = buffer;
    m_ctx->outputSize = buffer_size;
    m_ctx->outputAlloc = TRUE;

    return;
}

DWORD RegisterModule_Echo_C2(sh_context* ctx, uint32_t module_id) {
    DWORD result = RegisterModule(ctx, module_id, HandleEcho_C2, ctx);
    if (result != ERROR_SUCCESS) {
        AesLogger::LogError(ctx->log_ctx, "Failed to register module ID 0x%x. Error code: %d"_xor, module_id, result);
        return CUSTOM_MODULE_ECHO_C2_FAIL_REGISTER_MODULE;
    }

    AesLogger::LogSuccess(ctx->log_ctx, "Successfully registered module 0x%x."_xor, module_id);

    return ERROR_SUCCESS;
}
