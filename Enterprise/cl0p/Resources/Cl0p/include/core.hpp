#pragma once
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <filesystem>

#include "logger.hpp"
#include "xor_obfuscation.hpp"


// Globals
#define INITIAL_FILE XOR_LIT("Favbug.txt")
#define MAX_CMD_LINE_LENGTH 1024

// Error codes
#define LOG_SETUP_FAIL 1
#define LANG_CHECK_FAIL 2
#define DROP_GUARDFILE_FAIL 3
#define CREATE_MUTEX_FAIL 4
#define RELEASE_MUTEX_FAIL 5


namespace Core {

    /*
     * GetExecDir()
     *      About:
     *          Find the directory containing this executable
     *          See https://stackoverflow.com/a/62743863/
     *      Result:
     *          Return the path where this executable is located
     *      Returns:
     *          [std::filesystem::path]
     */
    std::filesystem::path GetExecDir();

    /*
     * DropFile(std::string filename)
     *      About:
     *         Create an empty file with the given filename
     *      Result:
     *          A file is written to disk
     *      Returns:
     *          [bool]
     */
    bool DropFile(std::string filename);

    /*
     * DropInitialFile()
     *      About:
     *         Create an empty file with a specific filename and wait 5 seconds
     *      Result:
     *          A file is written to disk
     *      Returns:
     *          [bool]
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    bool DropInitialFile();


    /*
     * cmdExecuteCommand(std::string command)
     *      About:
     *         Executes an arbitrary windows command with cmd.exe
     *      Result:
     *          Returns stdout/stderr output
     *      Returns:
     *          [std::string]
     *      MITRE ATT&CK Techniques:
     *          T1059.003: Command and Scripting Interpreter: Windows Command Shell
     *          T1106: Native API
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    std::string cmdExecuteCommand(std::string command);

    /*
     * SpawnProcessNoWait:
     *      About:
     *          Creates a process using the given command line, but does not collect output or wait for process
     *          to terminate. Leverages the CreateProcessW API call.
     *      Returns:
     *          Returns ERROR_SUCCESS on success, non-zero error code otherwise.
     *      MITRE ATT&CK Techniques:
     *          T1106: Native API
     */
    DWORD SpawnProcessNoWait(LPWSTR command_line);
}