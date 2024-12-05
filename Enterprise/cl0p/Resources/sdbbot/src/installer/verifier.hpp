#pragma once
#include "errors.hpp"
#include <expected>
#include <string_view>
#include <span>

#define IFEO_KEY LR"(SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\)"

// Registers a VerifierDLL for the executable specified.
std::expected<void, common::windows_error>
register_verifier_transacted(
    HANDLE transaction,
    std::wstring_view executable,
    std::wstring_view dll
);

