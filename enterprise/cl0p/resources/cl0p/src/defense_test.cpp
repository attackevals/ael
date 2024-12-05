#include <gtest/gtest.h>
#include <chrono>
#include <thread>

#include "defense.hpp"


class ProcTest : public ::testing::Test {
protected:
    std::vector<HANDLE> open_handles;

    int GetNumProcesses(std::string proc_name) {
        std::string command_line = std::format("powershell.exe -command \"(get-process {} -erroraction silentlycontinue).count\"", proc_name);
        std::string output = Core::cmdExecuteCommand(command_line);
        return std::stoi(output);
    }

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

    DWORD FindPID(LPSTR proc_name) {
        WTS_PROCESS_INFO* pWPIs = NULL;
        DWORD dwProcCount = 0;
        if (WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, NULL, 1, &pWPIs, &dwProcCount)) {
            //Go through all processes retrieved
            for (DWORD i = 0; i < dwProcCount; i++) {
                LPSTR name = pWPIs[i].pProcessName;
                DWORD pid = pWPIs[i].ProcessId;

                if (lstrcmp(name, proc_name) == 0) {
                    std::cout << proc_name << std::endl;
                    return pid;
                }
            }
        }
        return 0;
    }

    void KillProcess(std::string proc_name) {
        std::string command_line = std::format("powershell.exe -command \"(get-process {} -erroraction silentlycontinue | stop-process -f\").count\"", proc_name);
        ASSERT_NO_THROW(Core::cmdExecuteCommand(command_line));
        ASSERT_EQ(GetNumProcesses(proc_name), 0);
    }

    virtual void SetUp() {
        wchar_t noop_exe[] = L"clop-test-noop-exe.exe";
        wchar_t noop_exe2[] = L"clop-test-noop-multi.exe";
        StartNoopProcess(noop_exe, 1);
        StartNoopProcess(noop_exe2, 3);
        ASSERT_EQ(GetNumProcesses("clop-test-noop-exe"), 1);
        ASSERT_EQ(GetNumProcesses("clop-test-noop-multi"), 3);
    }

    virtual void TearDown() {
        KillProcess("clop-test-noop-exe");
        KillProcess("clop-test-noop-multi");
        for (HANDLE h : open_handles) {
            CloseHandle(h);
        }
    }
};


TEST(DefenseTests, TestMutexCreated) {
    ASSERT_NO_THROW(Defense::DummyLoop());
}

TEST(DefenseTests, TestDisableSecurity) {
    ASSERT_NO_THROW(Defense::StopServices());
}

TEST(DefenseTests, TestProcessCompareTrue) {
    EXPECT_TRUE(Defense::CompareProcess("wordpad.exe"));
}

TEST(DefenseTests, TestProcessCompareFalse) {
    EXPECT_FALSE(Defense::CompareProcess("foo.exe"));
}

TEST_F(ProcTest, TestKillProc) {
    ASSERT_EQ(GetNumProcesses("clop-test-noop-exe"), 1);
    ASSERT_EQ(GetNumProcesses("clop-test-noop-multi"), 3);
    std::this_thread::sleep_for(std::chrono::milliseconds(10000)); // avoid race condition

    std::vector<std::string> to_kill = {

        "clop-test-noop-exe.exe",
        "clop-test-noop-multi.exe",
        "clop-test-noop-multi.exe",
        "clop-test-noop-multi.exe",
        "clop-test-dne.exe",
        "clop-test-dne2.exe",
    };
    for (const std::string proc_name : to_kill) {
        LPSTR pname = const_cast<char*>(proc_name.c_str());
        DWORD pid = FindPID(pname);
        Defense::KillProcess(pname, pid);
        std::this_thread::sleep_for(std::chrono::milliseconds(10000)); // avoid race condition
    }

    EXPECT_EQ(GetNumProcesses("clop-test-noop-exe"), 0);
    EXPECT_EQ(GetNumProcesses("clop-test-noop-multi"), 0);
}

TEST(DefenseTests, TestDisableErrors) {
    ASSERT_NO_THROW(Defense::DisableErrors());
}
