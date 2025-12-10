#include "rcdll_util.hpp"
#include <winternl.h>

// Modifies path by appending to_append to the directory of path.
// Assumes path has at least 1 backslash and does not start or end with one.
void ConcatToSelfDir(char* path, size_t buf_size, const char* to_append) {
    // Get index of last back-slash
    size_t i = 0;
    size_t ult = 0;
    while (path[i] != '\0' && i < buf_size) {
        if (path[i] == '\\') {
            ult = i;
        }
        i++;
    }

    size_t start = ult + 1;

    for (size_t j = 0; j + start < buf_size; j++) {
        path[j + start] = to_append[j];
        if (to_append[j] == '\0') {
            break;
        }
    }

    // Truncate
    path[buf_size - 1] = '\0';
}

// Reads given file into buffer
DWORD ReadFileBytes(HANDLE h_file, unsigned char* buffer, DWORD bytes_to_read) {
    unsigned char* p_seek_buffer = buffer;
    DWORD remaining_bytes = bytes_to_read;
    DWORD bytes_read;
    while (remaining_bytes > 0) {
        if (!ReadFile(h_file, p_seek_buffer, remaining_bytes, &bytes_read, NULL)) {
            return GetLastError();
        }
        p_seek_buffer += bytes_read;
        remaining_bytes -= bytes_read;
    }
    return ERROR_SUCCESS;
}

// Given the command-line string, returns the length of the command executable substring
size_t GetExecutableNameLength(std::wstring_view command_line) {
    size_t module_name_len = 0;
    if (command_line[0] == L'"') {
        // Loop until we hit closing double quotes
        do {
            module_name_len++;
        } while (module_name_len < command_line.length() && command_line[module_name_len] != L'"');
        module_name_len++;
    } else if (command_line[0] == L'\'') {
        // Loop until we hit closing single quotes
        do {
            module_name_len++;
        } while (module_name_len < command_line.length() && command_line[module_name_len] != L'\'');
        module_name_len++;
    } else {
        // No enclosing quotes - loop until we hit a space
        do {
            module_name_len++;
        } while (module_name_len < command_line.length() && command_line[module_name_len] != L' ');
    }
    return module_name_len;
}

// Returns true if command line has the arg, otherwise returns false
BOOL CommandLineHasArg(std::wstring_view command_line, LPCWSTR arg) {
    if (arg[0] == L'\0') {
        return FALSE;
    }

    // Start searching for args after executable name
    size_t i = GetExecutableNameLength(command_line);

    while (i < command_line.length()) {
        if (command_line[i] == arg[0]) {
            // Match entire arg
            size_t j = 1;
            BOOL mismatch = FALSE;
            for (; i + j < command_line.length(); j++) {
                if (arg[j] == L'\0') {
                    // Reached end of arg - make sure next character is space or we reached end of cmd line
                    return i + j >= command_line.length() || command_line[i + j] == L'\0' || command_line[i + j] == L' ';
                }

                if (command_line[i + j] != arg[j]) {
                    mismatch = TRUE;
                    break;
                }
            }

            if (!mismatch) {
                // Reached end of command-line string - check that we also reached end of arg
                return arg[j] == L'\0';
            }
        }
        i++;
    }

    return FALSE;
}

// Returns true if running from MSI installer (if run with "-i" arg), false otherwise
BOOL RunningFromInstaller() {
    PPEB peb = NtCurrentTeb()->ProcessEnvironmentBlock;
    std::wstring_view command_line = std::wstring_view(
        peb->ProcessParameters->CommandLine.Buffer,
        peb->ProcessParameters->CommandLine.Length
    );

    return CommandLineHasArg(command_line, L"-i");
}
