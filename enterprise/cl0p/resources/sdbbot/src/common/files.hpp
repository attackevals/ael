#pragma once
#include "errors.hpp"
#include "handles.hpp"
#include <expected>
#include <filesystem>
#include <fstream>
#include <span>

namespace common {

// Reads a file, by its handle, into a vector of bytes.
std::expected<std::vector<std::byte>, windows_error>
read_file(HANDLE handle);

// Writes the specified byte array to a file on disk transactionally.
std::expected<unique_handle, windows_error>
write_file(HANDLE transaction, const auto& bytes, std::wstring_view path) {
    unique_handle file = CreateFileTransactedW(
        path.data(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr,
        transaction,
        nullptr,
        nullptr
    );
    if (!file) {
        return std::unexpected(get_last_error());
    }

    unsigned long count{};
    if (!WriteFile(file.get(), bytes.data(), bytes.size(), &count, nullptr)) {
        return std::unexpected(get_last_error());
    }

    return file;
}

std::expected<void, windows_error>
create_symlink(
    HANDLE transaction,
    const std::filesystem::path& link_path,
    const std::filesystem::path& file_path,
    unsigned long flags = 0
);

} //namespace common
