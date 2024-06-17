
#include "NamedPipe.h"

// std::string IterNamedPipes(std::vector<std::string> namedPipes)
//      About:
//         takes a vector string list of named pipes and returns data from the first one in the list.
//      Result:
//          [std::string] returns data from the first named pipe found.
std::string IterNamedPipes(std::vector<std::string> namedPipes) {
    unsigned int vecSize = namedPipes.size();
    std::string outString;
    std::vector<char> response_buffer(PIPE_OUT_BUFFER, 0);
    DWORD totalBytesRead = 0;
    DWORD error = ERROR_SUCCESS;

    // Loop through namedPipes to read output
    for (unsigned int i = 0; i < vecSize; i++) {
        HANDLE hFile = CreateFile(namedPipes[i].c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

        if (hFile == INVALID_HANDLE_VALUE) {
            if (DEBUG_MSG)
                writeToFile(XorStr("Failed to open pipe: ") + namedPipes[i]);
            continue;
        }

        std::vector<char> output;

        if (DEBUG_MSG)
            writeToFile(XorStr("Reading data from pipe: ") + namedPipes[i]);

        DWORD numBytesRead = 0;
        BOOL result;
        while (TRUE) {
            result = ReadFile(
                hFile,
                response_buffer.data(),
                PIPE_OUT_BUFFER,
                &numBytesRead,
                NULL
            );

            if (numBytesRead > 0) {
                outString.append(response_buffer.begin(), response_buffer.begin() + numBytesRead);
                totalBytesRead += numBytesRead;
                if (DEBUG_MSG)
                    writeToFile(XorStr("Read a total of ") + std::to_string(totalBytesRead) + XorStr(" bytes from pipe: ") + outString);
                CloseHandle(hFile); // Close pipe handle
                return outString;
            }
            if (!result) {
                error = GetLastError();
                if (error == ERROR_BROKEN_PIPE) {
                    // End of pipe. Normal flow.
                    error = ERROR_SUCCESS;
                    break;
                }
                else if (error != ERROR_MORE_DATA) {
                    if (DEBUG_MSG)
                        writeToFile(XorStr("Failed to read data from pipe. Error code: ") + std::to_string(error),"error");

                    CloseHandle(hFile); // Close pipe handle
                    return "";
                }
            }
        }

        CloseHandle(hFile); // Close pipe handle
        
        

        if (DEBUG_MSG)
            writeToFile(XorStr("Read a total of ") + std::to_string(totalBytesRead) + XorStr(" bytes from pipe: ") + outString);
    }
    return outString;
}

// std::vector<std::string> FindPipeWithPrefix()
//      About:
//         takes a prefix string and looks for any named pipe that have the matching prefix string.
//      Result:
//          std::vector<std::string>] returns a vector string list of matching named pipes.
std::vector<std::string> FindPipeWithPrefix()
{
    std::string directory = XorStr("\\\\.\\pipe");
    std::vector<std::string> pipeDirs;
    HANDLE dir;
    WIN32_FIND_DATA file_data;
    if (DEBUG_MSG)
        writeToFile(XorStr("Looking for Named Pipes with custom prefix"));
    if ((dir = FindFirstFile((directory + "\\" + DEFAULT_PIPE_PREFIX + "*").c_str(), &file_data)) == INVALID_HANDLE_VALUE)
        return pipeDirs; /* No files found */

    do {
        const std::string file_name = file_data.cFileName;
        const std::string full_file_name = directory + "\\" + file_name;
        const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

        if (file_name[0] == '.')
            continue;

        if (is_directory)
            continue;
        if (DEBUG_MSG)
            writeToFile(XorStr("Found Named Pipe: ") + full_file_name);
        pipeDirs.push_back(full_file_name);
    } while (FindNextFile(dir, &file_data));

    FindClose(dir);

    return pipeDirs;
}

