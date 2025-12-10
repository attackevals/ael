#pragma once

#include "api_helper.hpp"
#include "shared_func.hpp"
#include "obfuscation.hpp"

#define FAIL_GET_CREATE_MUTEXW 0x64001
#define FAIL_GET_OPEN_MUTEXW 0x64002

using CreateMutexW_t = decltype(&CreateMutexW);
using OpenMutexW_t = decltype(&OpenMutexW);

DWORD EnsureSingleton(LPCWSTR mutex_name, BOOL create_mutex);
