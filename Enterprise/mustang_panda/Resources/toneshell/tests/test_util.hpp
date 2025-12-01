#pragma once

#include "shellcode.hpp"
#include <vector>

namespace test_utils {

// Buffer that will contain mock data that the client sends to the c2 server
extern std::vector<unsigned char> mock_client_send_buf;
extern std::vector<std::vector<unsigned char>> mock_sent_data_vectors;

// Buffer that will contain mock data that the client receives from the c2 server
extern std::vector<unsigned char> mock_client_recv_buf;
extern std::vector<unsigned char> mock_server_resp_data;

extern sh_context mock_context;

extern unsigned char mock_id[16];

extern LPCSTR mock_hostname;

void Initialize();

void Cleanup();

// Mock functions
int mock_closesocket(SOCKET s);

SOCKET WSAAPI mock_socket(
    int af,
    int type,
    int protocol
);

int WSAAPI mock_connect(
    SOCKET         s,
    const sockaddr *name,
    int            namelen
);

int WSAAPI mock_shutdown(
  SOCKET s,
  int    how
);

int WSAAPI mock_send(
    SOCKET     s,
    const char *buf,
    int        len,
    int        flags
);

int mock_recv(
    SOCKET s,
    char   *buf,
    int    len,
    int    flags
);

} // namespace
