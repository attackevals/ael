#pragma once
#ifndef __NET_H__
#define __NET_H__

#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#endif