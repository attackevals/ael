#include "test.h"

using namespace std;

// std::string IterNamedPipes(std::vector<std::string> namedPipes)
//      About:
//         takes a vector string list of named pipes and returns data from the first one in the list.
//      Result:
//          [std::string] returns data from the first named pipe found.
std::string IterNamedPipes(std::vector<std::string> namedPipes) {
	unsigned int vecSize = namedPipes.size();
	std::vector<char> output;
    BOOL result;
    std::vector<char> response_buffer = std::vector<char>(PIPE_OUT_BUFFER); 
    DWORD numBytesRead = 0;
    DWORD totalBytesRead = 0;
    DWORD error = ERROR_SUCCESS;

    // Loop through namedPipes to read output
	for (unsigned int i = 0; i < vecSize; i++) {
		HANDLE hFile = CreateFile(namedPipes[i].c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		std::cout << XorStr("Reading data from pipe: ") << std::string{ namedPipes[i] } << std::endl;

		while(TRUE) {
            result = ReadFile(
                hFile,
                &response_buffer[0],
                PIPE_OUT_BUFFER,
                &numBytesRead,
                NULL
            );

            totalBytesRead += numBytesRead;
            if (numBytesRead > 0) {
                output.insert(output.end(), response_buffer.begin(), response_buffer.begin() + numBytesRead);
            }
            if (!result) {
                error = GetLastError();
                if (error == ERROR_BROKEN_PIPE) {
                    // End of pipe. Normal flow.
                    error = ERROR_SUCCESS;
                    break;
                } else if (error != ERROR_MORE_DATA) {
                    cout << XorStr("Failed to read data from pipe. Error code: ") + std::to_string(error) << endl;
                    return "";
                }
            }
        }
	    std::cout << XorStr("Read a total of ") << std::to_string(totalBytesRead) << XorStr(" bytes from pipe: ") << std::string(output.begin(), output.end()) << std::endl;
	}
    return std::string(output.begin(), output.end());
}

// std::vector<std::string> FindPipeWithPrefix()
//      About:
//         takes a prefix string and looks for any named pipe that have the matching prefix string.
//      Result:
//          std::vector<std::string>] returns a vector string list of matching named pipes.
std::vector<std::string> FindPipeWithPrefix()
{
    std::string directory = "\\\\.\\pipe";
    std::vector<std::string> pipeDirs;
    HANDLE dir;
    WIN32_FIND_DATA file_data;

	std::cout << XorStr("Looking for Named Pipes prefixed with: ") << DEFAULT_PIPE_PREFIX << std::endl;
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
		std::cout << XorStr("Found Named Pipe: ") << full_file_name << std::endl;
        pipeDirs.push_back(full_file_name);
    } while (FindNextFile(dir, &file_data));

    FindClose(dir);

    return pipeDirs;
}

int main() {
    std::vector<string> pipes = FindPipeWithPrefix();
    std::string msg = IterNamedPipes(pipes);
	return 0;
}