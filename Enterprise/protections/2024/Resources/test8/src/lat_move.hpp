#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <string>

namespace lat_move {

// 
/*
 * GetCurrExecutablePath:
 *      About:
 *          Gets absolute path to current executable using the GetModuleFileNameW API call.
 *      Result:
 *          Absolute path to self, or throws exception on failure.
 */
std::wstring GetCurrExecutablePath();

/*
 * CreateAndExecRemoteScheduledTask:
 *      About:
 *          Creates a remote scheduled task on the specific target to execute the given executable with the given arguments.
 *          The task executable will be executed 15 seconds after the task is registered, and the task will run as the SYSTEM
 *          use with highest privileges. The task name is set to "\Microsoft\Windows\Net Session File Management Task".
 *          Relevant API calls:
 *              CoInitializeEx, CoInitializeSecurity, CoCreateInstance, ITaskService::Connect, ITaskService::GetFolder,
 *              ITaskService::NewTask, ITaskDefinition::get_RegistrationInfo, IRegistrationInfo::put_Author,
 *              ITaskDefinition::get_Principal, IPrincipal::put_Id, IPrincipal::put_GroupId, IPrincipal::put_LogonType,
 *              IPrincipal::put_RunLevel, ITaskDefinition::get_Settings, ITaskSettings::put_StartWhenAvailable,
 *              ITaskDefinition::get_Triggers, ITriggerCollection::Create, ITrigger::QueryInterface, IRegistrationTrigger::put_Id,
 *              IRegistrationTrigger::put_Delay, ITaskDefinition::get_Actions, IActionCollection::Create, 
 *              IAction::QueryInterface, IExecAction::put_Path, ITaskFolder::RegisterTaskDefinition
 *      Result:
 *          Returns true on success, false otherwise.
 *      MITRE ATT&CK Techniques:
 *          T1053.005: Scheduled Task/Job: Scheduled Task
 */
bool CreateAndExecRemoteScheduledTask(const std::wstring& target, const std::wstring& exec_path, const std::wstring& args);

} // namespace