#include "io.hpp"
#include "networking.hpp"
#include <format>

namespace IO {

    // threadsafe counter
    std::atomic<int> opsRemaining = 0;

    HANDLE CreateIOCP() {
        // Create IO Completion port
        HANDLE hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
        if (hCompletionPort == nullptr)
            return nullptr;

        return hCompletionPort;
    }

    bool StartAsyncRead(const std::string& filePath, HANDLE hCompletionPort) {
        // open file
        HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            XorLogger::LogError(std::format(
                "{} {}: {}",
                XOR_LIT("Failed to open source file"),
                filePath,
                GetLastError()
            ));
            return false;
        }

        // allocate the context struct
        auto* context = new IoOperationContext(filePath, MAX_CHUNK_SIZE);
        context->hFile = hFile;

        // Associate the handle with IOCP
        CreateIoCompletionPort(hFile, hCompletionPort, (ULONG_PTR)context, 0);

        // start async read into context buffer
        BOOL readResult = ReadFile(hFile, context->buffer.data(), context->buffer.size(), nullptr, &context->overlapped);
        if (!readResult && GetLastError() != ERROR_IO_PENDING) {
            XorLogger::LogError(XOR_LIT("Async read failed to start: ") + GetLastError());
            CloseHandle(hFile);
            delete context;
            return false;
        }

        return true;
    }

    void ProcessIoCompletionPort(HANDLE hCompletionPort, std::atomic<int>& opsRemaining) {
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        OVERLAPPED* pOverlapped = nullptr;
        DWORD error_code;
        DWORD bytes_read = 0;
        while (true) {
            // Time out after 1 minute
            BOOL result = GetQueuedCompletionStatus(hCompletionPort, &bytesTransferred, &completionKey, &pOverlapped, 60*1000);
            if (!result) {
                error_code = GetLastError();
                if (error_code != ERROR_IO_PENDING) {
                    XorLogger::LogError(std::format(
                        "{}: {}",
                        XOR_LIT("GetQueuedCompletionStatus failed with error code"),
                        error_code
                    ));
                    break;
                }
                XorLogger::LogError(XOR_LIT("GetQueuedCompletionStatus waiting on IO."));
                continue;
            }

            IoOperationContext* context = reinterpret_cast<IoOperationContext*>(completionKey);
            if (bytesTransferred == 0) {
                //nothing to read
                // cleanup 
                CloseHandle(context->hFile);
                delete context;
                break;
            }
            // send off data.
            network::UploadFilePUTRequest(context->fileName, context->buffer, static_cast<int>(bytesTransferred));

            if (bytesTransferred < context->buffer.size()) {
                // reached end of file
                CloseHandle(context->hFile);
                delete context;
                break;
            }

            DWORD newOffset = pOverlapped->Offset + bytesTransferred;
            DWORD newOffsetHigh = pOverlapped->OffsetHigh;

            if (newOffset < pOverlapped->Offset) {
                newOffsetHigh++;
            }

            pOverlapped->Offset = newOffset;
            pOverlapped->OffsetHigh = newOffsetHigh;

            memset(context->buffer.data(), 0, context->buffer.size());
            if (!ReadFile(context->hFile, context->buffer.data(), context->buffer.size(), &bytes_read, &context->overlapped)) {
                error_code = GetLastError();
                if (error_code != ERROR_IO_PENDING) {
                    XorLogger::LogError(std::format(
                        "{}: {}",
                        XOR_LIT("Failed to read file. Error code"),
                        error_code
                    ));
                    CloseHandle(context->hFile);
                    delete context;
                    break;
                }
            } else if (bytes_read == 0) {
                // Exactly at end of file - nothing to transfer next round
                CloseHandle(context->hFile);
                delete context;
                break;
            }
        }
        opsRemaining.fetch_sub(1, std::memory_order_relaxed); // dec the op count
    }
}