#include <filesystem>
#include <fstream>
#include <string_view>
#include <span>
#include <vector>
#include <format>
#include <print>
#include <iostream>

#ifndef TARGET_PATH
#error  "bin2h.cpp must have TARGET_PATH set at compile-time"
#endif

#ifndef OUTPUT_DIR
#error  "bin2h.cpp must have OUTPUT_DIR set at compile-time"
#endif
#define OUTPUT_PATH OUTPUT_DIR ## "/loader.dll.hpp"

constexpr const wchar_t* header_template = LR"(
// This is a header template that is populated at build time. The instantiated
// version will reside in the binary directory for this target.
#pragma once
#include <array>

// Create a dedicated section for the byte array
#pragma section(".rat", read)

namespace embedded {{

// Allocate the loader array within the specified section
__declspec(allocate(".rat"))
inline const std::array<uint8_t, {}> loader{{
{}
}};

}} // namespace embedded
)";

std::vector<std::byte> read_file(const std::filesystem::path& path) {
    std::basic_ifstream<std::byte> file{ path, std::ios::binary };

    // ~basic_ifstream() closes the file handle
    return std::vector<std::byte>{
        std::istreambuf_iterator<std::byte>{file},
        std::istreambuf_iterator<std::byte>()
    };
}

std::wstring read_template(const std::filesystem::path& path) {
    std::wifstream file{ path };

    // ~basic_ifstream() closes the file handle
    return std::wstring{
        std::istreambuf_iterator<wchar_t>{file},
        std::istreambuf_iterator<wchar_t>()
    };
}

void write_file(std::wstring content, const std::filesystem::path& path) {
    std::wofstream outfile{ path };

    try {
        outfile << content;
    }
    catch (const std::exception& e) {
        std::println("Failed to write to output file: {}", e.what());
        exit(EXIT_FAILURE);
    }
}

std::wstring format_bytes(
    std::span<std::byte> bytes,
    std::wstring_view source_template
) {
    constexpr const wchar_t* format = L"{:#04x},";
    std::wstring formatted;

    // Reserving space up front to prevent reallocations
    // There is a newline character for every 15 bytes
    size_t lines = bytes.size_bytes() / 15;
    // Format each byte as "0xFF," and account for newlines
    formatted.reserve((bytes.size_bytes() * (5 * sizeof(wchar_t))) + lines);

    for (size_t i = 0; i < bytes.size(); ++i) {
        formatted += std::format(format, std::to_integer<uint8_t>(bytes[i]));

        // Wrap the array 
        if (0 == ((i + 1)) % 15) {
            formatted += L'\n';
        }
    }

    // Clean up the end of the array
    while (formatted.back() == L'\n' || formatted.back() == L',') {
        formatted.pop_back();
    }

    return std::vformat(
        source_template,
        std::make_wformat_args(bytes.size_bytes(), formatted)
    );
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::println("{} usage: [input] [template] [output]", argv[0]);
    }

    auto input = std::filesystem::absolute(argv[1]);
    auto source_template = read_template(std::filesystem::absolute(argv[2]));
    auto output = std::filesystem::absolute(argv[3]);

    auto bytes = read_file(input);
    auto header = format_bytes(bytes, source_template);
    write_file(header, output);

    return EXIT_SUCCESS;
}