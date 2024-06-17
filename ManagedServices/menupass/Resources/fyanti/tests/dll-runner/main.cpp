// dll-runner is a testing utility for the native-loader DLL.
#include "native-loader.hpp"
#include <Windows.h>
#include <iostream>

int wmain(int argc, wchar_t* argv[]) {
    if (!execute_assembly()) {
        std::println(std::cerr, "Failed to execute assembly");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}