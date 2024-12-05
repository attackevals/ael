#include "io.hpp"
#include <filesystem>
#include <fstream>
#include <time.h>
#include <gtest/gtest.h>


class IOTesting : public ::testing::Test {

protected:
    std::wstring root_dir = L"stealbit_unit_test_env1";

    void createDummyFile(std::string path) {
        std::ofstream ofs(path);
        ofs << "This is quality content " + path;
        ofs.close();
    }

    void setupTestEnvironment() {
        cleanupTestEnvironment();

        std::vector<std::string> directories = {
            "TestDir",
        };


        std::vector<std::string> files = {
            "test1.txt",
            "test2.txt",
            "test3.txt",
            "test4.txt",
            "test5.txt",
            "test6.txt",
            "test7.txt",
            "test8.txt",
            "test9.txt",
            "test10.txt",
        };

        // Create directories and files
        std::string full_dir_path = std::filesystem::current_path().string() + "\\" + "stealbit_unit_test_env1";
        std::filesystem::create_directories(full_dir_path);
        // Standard files
        for (std::string f : files) {
            createDummyFile(full_dir_path + "\\" + f);
        }
    }

    void cleanupTestEnvironment() {
        std::filesystem::remove_all("stealbit_unit_test_env1");
    }

    virtual void SetUp() {
        setupTestEnvironment();
    }

    virtual void TearDown() {
        cleanupTestEnvironment();
    }
};

TEST_F(IOTesting, IOCPExpectedOpsTest)
{
    std::vector<std::string> files = {
            "test1.txt",
            "test2.txt",
            "test3.txt",
            "test4.txt",
            "test5.txt",
            "test6.txt",
            "test7.txt",
            "test8.txt",
            "test9.txt",
            "test10.txt",
    };


    HANDLE hCompletionPort = IO::CreateIOCP();

    std::string full_dir_path = std::filesystem::current_path().string() + "\\" + "stealbit_unit_test_env1";
    // loop async read
    for (const auto& f : files)
    {
        IO::opsRemaining.fetch_add(1, std::memory_order_relaxed);
        IO::StartAsyncRead(full_dir_path + "\\" + f, hCompletionPort);
    }
    std::atomic<int> expected = 10;

    EXPECT_EQ(IO::opsRemaining, expected);

    while (IO::opsRemaining.load(std::memory_order_relaxed) > 0)
    {
        // loop process completion port until all files are done
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        OVERLAPPED* pOverlapped = nullptr;

        BOOL result = GetQueuedCompletionStatus(hCompletionPort, &bytesTransferred, &completionKey, &pOverlapped, INFINITE);
        if (!result) {
            XorLogger::LogError(XOR_LIT("GetQueuedCompletionStatus failed with error: ") + GetLastError());
            return;
        }

        IoOperationContext* context = reinterpret_cast<IoOperationContext*>(completionKey);
        if (bytesTransferred > 0) {
            // do whatever we want to do with the file data here.
            std::cout << "Filename: " << context->fileName << " file data: " << context->buffer.data() << std::endl;
        }
        else {
            XorLogger::LogError(XOR_LIT("No bytes transferred for file: ") + context->fileName);
        }

        // cleanup 
        CloseHandle(context->hFile);

        //delete context;
        IO::opsRemaining.fetch_sub(1, std::memory_order_relaxed); // dec the op count
    }
    CloseHandle(hCompletionPort);
}

TEST_F(IOTesting, IOCPStdoutTest)
{
    std::vector<std::string> files = {
            "test1.txt",
            "test2.txt",
            "test3.txt",
            "test4.txt",
            "test5.txt",
            "test6.txt",
            "test7.txt",
            "test8.txt",
            "test9.txt",
            "test10.txt",
    };


    HANDLE hCompletionPort = IO::CreateIOCP();

    std::string full_dir_path = std::filesystem::current_path().string() + "\\" + "stealbit_unit_test_env1";
    // loop async read
    for (const auto& f : files)
    {
        IO::opsRemaining.fetch_add(1, std::memory_order_relaxed);
        IO::StartAsyncRead(full_dir_path + "\\" + f, hCompletionPort);
    }

    std::vector<std::string> fileContents;

    while (IO::opsRemaining.load(std::memory_order_relaxed) > 0)
    {
        // loop process completion port until all files are done
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        OVERLAPPED* pOverlapped = nullptr;

        BOOL result = GetQueuedCompletionStatus(hCompletionPort, &bytesTransferred, &completionKey, &pOverlapped, INFINITE);
        if (!result) {
            XorLogger::LogError(XOR_LIT("GetQueuedCompletionStatus failed with error: ") + GetLastError());
            return;
        }

        IoOperationContext* context = reinterpret_cast<IoOperationContext*>(completionKey);
        if (bytesTransferred > 0) {
            // do whatever we want to do with the file data here.
            std::cout << "Filename: " << context->fileName << " file data: " << context->buffer.data() << std::endl;
        }
        else {
            XorLogger::LogError(XOR_LIT("No bytes transferred for file: ") + context->fileName);
        }

        // cleanup 
        CloseHandle(context->hFile);

        //delete context;
        IO::opsRemaining.fetch_sub(1, std::memory_order_relaxed); // dec the op count
    }
    CloseHandle(hCompletionPort);

    std::atomic<int> expected = 0;

    EXPECT_EQ(IO::opsRemaining, expected);
}