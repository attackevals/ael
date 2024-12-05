#pragma once
#include "errors.hpp"
#include <filesystem>
#include <expected>
#include <string_view>
#include <span>

// Decrypts and writes the embedded loader dll (loader.dll.hpp -> embedded::loader) to the path specified
std::expected<void, common::windows_error>
drop_loader(const std::filesystem::path& path);
