/*Good reference for integrating with cmake https://discourse.cmake.org/t/cmake-cant-find-library-header-files/1334*/

#include "module.hpp"

char* Initialize(){
    // Variable used in executeModuleEnvVarSuccess GoogleTest
    if (setenv("fn", "test/to/path", 1) != 0) {
        XorLogger::LogError("Failed to set environment variable fn");
        }

    // char* used in DlopenDlsymSuccessTest GoogleTest
    char result[] = "Success";
    char* heapCopy = strdup(result);
    return heapCopy;
}