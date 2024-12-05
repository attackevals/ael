#include <filesystem>
#include "tamper.hpp"
#include "execute.hpp"
#include "test_utils.hpp"
#include <gtest/gtest.h>

class ProcTest : public ::testing::Test {
protected:
    std::vector<HANDLE> open_handles;

    void StartNoopProcess(LPWSTR proc_name, int num_processes) {
        for (int i = 0; i < num_processes; i++) {
            STARTUPINFOW noop_si;
            PROCESS_INFORMATION noop_pi;
            ZeroMemory(&noop_si, sizeof(noop_si));
            noop_si.cb = sizeof(noop_si);
            ZeroMemory(&noop_pi, sizeof(noop_pi));
            bool result = CreateProcessW(
                NULL,
                proc_name,
                NULL,           // Process handle not inheritable
                NULL,           // Thread handle not inheritable
                FALSE,          // Set handle inheritance to FALSE
                0,              // No creation flags
                NULL,           // Use parent's environment block
                NULL,           // Use parent's starting directory 
                &noop_si,            // Pointer to STARTUPINFO structure
                &noop_pi           // Pointer to PROCESS_INFORMATION structure
            );
            ASSERT_TRUE(result);
            open_handles.push_back(noop_pi.hProcess);
            open_handles.push_back(noop_pi.hThread);
        }
    }

    virtual void SetUp() {
        wchar_t noop_exe[] = L"lockbit-test-noop-exe.exe";
        wchar_t noop_exe2[] = L"lockbit-test-noop-multi.exe";
        StartNoopProcess(noop_exe, 1);
        StartNoopProcess(noop_exe2, 3);
        ASSERT_EQ(test_utils::GetNumProcesses(L"lockbit-test-noop-exe"), 1);
        ASSERT_EQ(test_utils::GetNumProcesses(L"lockbit-test-noop-multi"), 3);
    }

    virtual void TearDown() {
        test_utils::KillProcess(L"lockbit-test-noop-exe");
        test_utils::KillProcess(L"lockbit-test-noop-multi");
        for (HANDLE h: open_handles) {
            CloseHandle(h);
        }
    }
};

TEST_F(ProcTest, TestKillProc) {
    ASSERT_EQ(test_utils::GetNumProcesses(L"lockbit-test-noop-exe"), 1);
    ASSERT_EQ(test_utils::GetNumProcesses(L"lockbit-test-noop-multi"), 3);
    std::set<unsigned long> to_kill = {
        0x0ba4f132, // L"lockbit-test-noop-exe",
        0x893551bb, // L"lockbit-test-noop-multi",
        0x0ba4eba7, // L"lockbit-test-dne",
        0x804260b9, // L"lockbit-test-dne2",
    };
    tamper::KillProcesses(to_kill);
    EXPECT_EQ(test_utils::GetNumProcesses(L"lockbit-test-noop-exe"), 0);
    EXPECT_EQ(test_utils::GetNumProcesses(L"lockbit-test-noop-multi"), 0);
}