#include <iostream>
#include <windows.h>
#include <thread>
#include <vector>
#include <string>
#include <atomic>
#include "logger.hpp"
#include "xor_string.hpp"


struct IoOperationContext
{
    OVERLAPPED overlapped; // overlapped structure for async operations
    std::string fileName; // file name
    std::vector<unsigned char> buffer; // buffer for read operation
    HANDLE hFile; // handle for the file, can be used to close later instead of my vector


    // Initialize
    IoOperationContext(const std::string& fName, size_t bufferSize)
        : fileName(fName), buffer(bufferSize)
    {
        ZeroMemory(&overlapped, sizeof(OVERLAPPED));
        hFile = INVALID_HANDLE_VALUE;
    }
};

namespace IO {

    // threadsafe counter
    extern std::atomic<int> opsRemaining;

     /*
     * CreateIOCP:
     *      About:
     *          Creates initial IOCP handle using CreateIoCompletionPort call with an INVALID_HANDLE_VALUE to initialize the IOCP.
     *      Result:
     *          returns a handle to the IoCompletionPort
     *      Returns:
     *          HANDLE
     *      MITRE ATT&CK Techniques:
     *          n/a
     *      CTI:
     *          https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf
     *          https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/
     *      Other References:
     *          n/a
     */
    HANDLE CreateIOCP();


     /*
     * StartAsyncRead:
     *      About:
     *          Takes the file path for a given file and the IOCP handle.
     *          Generates a context struct that holds the buffer, handle and other contextual information to be used
     *          as the IOCP "key". 
     *          Creates an IOCP with a handle to the file at the given file path.
     *          finally, starts an async read on the file.
     *      Result:
     *          starts the async read on a given file handle and returns true on success, false on failure.
     *      Returns:
     *          True on success, false on failure.
     *      MITRE ATT&CK Techniques:
     *          n/a
     *      CTI:
     *          https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf
     *          https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/
     *      Other References:
     *          n/a
     */
    bool StartAsyncRead(const std::string& filePath, HANDLE hCompletionPort);

     /*
     * ProcessIoCompletionPort:
     *      About:
     *          Takes the completion port handle and the threadsafe counter as arguments
     *          checks to see if any buffered files are complete using GetQueuedCompletionStatus against the 
     *          IOCP handle + context key.
     *          if a completed read is queued context->buffer is passed to stdout.
     *      Result:
     *          takes filled buffers and sends the data to stdout
     *      Returns:
     *          void
     *      MITRE ATT&CK Techniques:
     *          n/a
     *      CTI:
     *          https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf
     *          https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/
     *      Other References:
     *          n/a
     */
    void ProcessIoCompletionPort(HANDLE hCompletionPort, std::atomic<int>& opsRemaining);
}