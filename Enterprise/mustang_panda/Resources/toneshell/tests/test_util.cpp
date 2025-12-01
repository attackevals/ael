#include "test_util.hpp"
#include "shellcode_util.hpp"
#include "util.hpp"
#include "comms.hpp"

namespace test_utils {

// Buffers that will contain mock data that the client sends to the c2 server
std::vector<unsigned char> mock_client_send_buf;
std::vector<std::vector<unsigned char>> mock_sent_data_vectors;

// Buffer that will contain mock data that the client receives from the c2 server
std::vector<unsigned char> mock_client_recv_buf;
std::vector<unsigned char> mock_server_resp_data;

sh_context mock_context;

SOCKET dummy_socket = SOCKET(2);

unsigned char mock_id[16] = {
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,
    0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf
};

LPCSTR mock_hostname = "HOSTNAME";

std::vector<wchar_t> mock_command_buf;

void Initialize() {
    mock_client_send_buf.reserve(sizeof(client_message));
    mock_client_recv_buf.reserve(sizeof(server_response));
    mock_server_resp_data.reserve(sizeof(server_response));
    mock_command_buf.reserve(MAX_CMD_LEN + 1);

    pi_memset(&mock_context, 0, sizeof(mock_context));
    FetchFunctions(&(mock_context.fp));
    memcpy(mock_context.victim_id, mock_id, sizeof(mock_id));
    memcpy(mock_context.hostname, mock_hostname, strlen(mock_hostname));
    mock_context.hostname_size = strlen(mock_hostname);
    AesLogger::InitializeLogger(&(mock_context.fp.shared_fp), WIDEN(SH_LOG_FILE), ENCRYPTED_LOGGING, &(mock_context.log_ctx));
    mock_context.command_buf = mock_command_buf.data();

    // Set up mock functions
    mock_context.fp.fp_closesocket = test_utils::mock_closesocket;
    mock_context.fp.fp_socket = test_utils::mock_socket;
    mock_context.fp.fp_connect = test_utils::mock_connect;
    mock_context.fp.fp_shutdown = test_utils::mock_shutdown;
    mock_context.fp.fp_send = test_utils::mock_send;
    mock_context.fp.fp_recv = test_utils::mock_recv;
}

void Cleanup() {
    mock_client_send_buf.clear();
    for (auto v: mock_sent_data_vectors) {
        v.clear();
    }
    mock_sent_data_vectors.clear();
    mock_client_recv_buf.clear();
    mock_server_resp_data.clear();
    mock_command_buf.clear();

    AesLogger::CloseLogger(&(mock_context.log_ctx));
}

// Mock functions
int mock_closesocket(SOCKET s) {
    return 0;
}

SOCKET WSAAPI mock_socket(
    int af,
    int type,
    int protocol
) {
    return dummy_socket;
}

int WSAAPI mock_connect(
    SOCKET         s,
    const sockaddr *name,
    int            namelen
) {
    return 0;
}

int WSAAPI mock_shutdown(
    SOCKET s,
    int    how
) {
    return 0;
}

int WSAAPI mock_send(
    SOCKET     s,
    const char *buf,
    int        len,
    int        flags
) {
    std::vector<unsigned char> send_buf;
    send_buf.reserve(sizeof(client_message));
    size_t n_copy = len > send_buf.capacity() ? send_buf.capacity() : len;
    send_buf.insert(send_buf.end(), buf, buf + n_copy);
    mock_sent_data_vectors.push_back(send_buf);
    return n_copy;
}


int mock_recv(
    SOCKET s,
    char   *buf,
    int    len,
    int    flags
) {
    size_t n_copy = len < mock_server_resp_data.size() ? len : mock_server_resp_data.size();
    memcpy(buf, mock_server_resp_data.data(), n_copy);
    return n_copy;
}

} // namespace
