#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <sddl.h>
#include <vector>
#include <string>

#define FAIL_TIMEOUT_REACHED 0x3001
#define MAX_CMD_LINE_LENGTH 1024

using FP_CreatePipe = decltype(&CreatePipe);
using FP_CreateProcessW = decltype(&CreateProcessW);
using FP_GetExitCodeProcess = decltype(&GetExitCodeProcess);
using FP_PeekNamedPipe = decltype(&PeekNamedPipe);
using FP_SetHandleInformation = decltype(&SetHandleInformation);

// Defined in src/lockbit_main/execute.cpp
namespace execute {

/*
 * SpawnProcessNoWait:
 *      About:
 *          Create a process using the given command line, but does not collect output or wait for process
 *          to terminate. Leverages the CreateProcessW API call.
 *      Returns:
 *          Returns ERROR_SUCCESS on success, non-zero error code otherwise.
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 */
DWORD SpawnProcessNoWait(LPWSTR command_line);

/*
 * ExecuteProcess:
 *      About:
 *          Create a process using the given command line and return its output, using the CreateProcessW API call.
 *      Result:
 *          Returns a char vector of process output on success. error_code will be populated with ERROR_SUCCESS on success, otherwise
 *          some other error code.
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 *          T1057: Process Discovery
 *          T1559: Inter-Process Communication
 */
std::vector<char> ExecuteProcess(
    LPWSTR command_line,
    DWORD timeout_seconds,
    DWORD* error_code,
    DWORD* exit_code
);

} // namespace execute