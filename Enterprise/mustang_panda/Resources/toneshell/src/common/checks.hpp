#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Windows.h"

#define FAIL_LOAD_USER32 0x53003

#define PROCESS_NAME_MISMATCH 0x53001
#define WINDOW_CHECK_TIMEOUT 0x53002

DWORD VerifyProcessName(LPCWSTR desired);

DWORD ForegroundWindowCheck(DWORD timeout);
