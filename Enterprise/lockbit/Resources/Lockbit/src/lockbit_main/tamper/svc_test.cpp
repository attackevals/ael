#include <filesystem>
#include "tamper.hpp"
#include "execute.hpp"
#include "util/string_util.hpp"
#include <gtest/gtest.h>

#define STANDALONE_SVC_NAME L"LockbitTestSvc"
#define PARENT_SVC_NAME L"LockbitTestSvcParent"
#define DEP1_SVC_NAME L"LockbitTestSvcDep1"
#define DEP2_SVC_NAME L"LockbitTestSvcDep2"
#define NESTED_PARENT_SVC_NAME L"LockbitTestSvcNested"
#define NESTED_L1_SVC_NAME L"LockbitTestSvcNestedL1"
#define NESTED_L2_SVC_NAME L"LockbitTestSvcNestedL2"
#define NESTED_L3_SVC_NAME L"LockbitTestSvcNestedL3"

class SvcTest : public ::testing::Test {
protected:
    std::vector<std::wstring> test_services = {
        STANDALONE_SVC_NAME,
        DEP1_SVC_NAME,
        DEP2_SVC_NAME,
        PARENT_SVC_NAME,
        NESTED_L3_SVC_NAME,
        NESTED_L2_SVC_NAME,
        NESTED_L1_SVC_NAME,
        NESTED_PARENT_SVC_NAME
    };

    void CreateNoopSvc(LPCWSTR svc_name, LPCWSTR binary_path, LPCWSTR dependency) {
        wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
        if (wcslen(dependency) == 0) {
            swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"sc.exe create %s binpath= \"%s\"", svc_name, binary_path);
        } else {
            swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"sc.exe create %s binpath= \"%s\" depend= %s", svc_name, binary_path, dependency);
        }
        DWORD error_code;
        DWORD exit_code;
        std::vector<char> output = execute::ExecuteProcess(command_line, 5, &error_code, &exit_code); 
        ASSERT_EQ(error_code, ERROR_SUCCESS);
        ASSERT_EQ(exit_code, ERROR_SUCCESS);
        std::cout << "sc.exe create output: " << std::string(output.begin(), output.end()) << std::endl;
        std::wcout << L"Created NOOP service " << svc_name << L" with bin path " << binary_path << std::endl;
    }

    void StartNoopSvc(LPCWSTR svc_name) {
        wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
        swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"sc.exe start %s", svc_name);
        DWORD error_code;
        DWORD exit_code;
        std::vector<char> output = execute::ExecuteProcess(command_line, 5, &error_code, &exit_code); 
        ASSERT_EQ(error_code, ERROR_SUCCESS);
        ASSERT_EQ(exit_code, ERROR_SUCCESS);
        std::cout << "sc.exe start output: " << std::string(output.begin(), output.end()) << std::endl;
        std::wcout << L"Started NOOP service " << svc_name << std::endl;
    }

    void StopNoopSvc(LPCWSTR svc_name) {
        wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
        swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"powershell.exe -command \"stop-service %s -force\"", svc_name);
        DWORD error_code;
        DWORD exit_code;
        std::vector<char> output = execute::ExecuteProcess(command_line, 5, &error_code, &exit_code); 
        EXPECT_EQ(error_code, ERROR_SUCCESS);
        EXPECT_EQ(exit_code, ERROR_SUCCESS);
    }

    void DeleteNoopSvc(LPCWSTR svc_name) {
        wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
        swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"sc.exe delete %s", svc_name);
        DWORD error_code;
        DWORD exit_code;
        std::vector<char> output = execute::ExecuteProcess(command_line, 5, &error_code, &exit_code); 
        EXPECT_EQ(error_code, ERROR_SUCCESS);
        EXPECT_EQ(exit_code, ERROR_SUCCESS);
    }

    std::string GetServiceStatus(LPCWSTR svc_name) {
        std::wcout << L"Getting service status for " << svc_name << std::endl;
        wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
        swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"powershell.exe -command \"get-service %s -erroraction silentlycontinue | foreach { $_.status}\"", svc_name);
        DWORD error_code;
        DWORD exit_code;
        std::vector<char> output = execute::ExecuteProcess(command_line, 5, &error_code, &exit_code); 
        EXPECT_EQ(error_code, ERROR_SUCCESS);
        EXPECT_EQ(exit_code, ERROR_SUCCESS);
        std::string output_str(output.begin(), output.end());
        std::cout << "Got status " << output_str << std::endl;;
        return string_util::rtrim(output_str);
    }

    bool ServiceRunning(LPCWSTR svc_name) {
        return GetServiceStatus(svc_name) == std::string("Running");
    }

    bool ServiceStopped(LPCWSTR svc_name) {
        return GetServiceStatus(svc_name) == std::string("Stopped");
    }

    std::string GetServiceStartType(LPCWSTR svc_name) {
        std::wcout << L"Getting service start type for " << svc_name << std::endl;
        wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
        swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"powershell.exe -command \"get-service %s -erroraction silentlycontinue | foreach { $_.StartType}\"", svc_name);
        DWORD error_code;
        DWORD exit_code;
        std::vector<char> output = execute::ExecuteProcess(command_line, 5, &error_code, &exit_code); 
        EXPECT_EQ(error_code, ERROR_SUCCESS);
        EXPECT_EQ(exit_code, ERROR_SUCCESS);
        std::string output_str(output.begin(), output.end());
        std::cout << "Got start type " << output_str << std::endl;;
        return string_util::rtrim(output_str);
    }

    bool ServiceDisabled(LPCWSTR svc_name) {
        return GetServiceStartType(svc_name) == std::string("Disabled");
    }

    std::wstring GetCurrentExecutablePath() {
        wchar_t buffer[MAX_PATH + 1];
        GetModuleFileNameW(NULL, buffer, MAX_PATH + 1);
        std::wstring path(buffer);
        return std::filesystem::path(path).parent_path().c_str();
    }

    virtual void SetUp() {
        // Create and start services
        std::cout << "Creating and starting test services" << std::endl;

        // standalone service (no dependent services)
        std::wstring standalone_bin = GetCurrentExecutablePath() + L"\\lockbit-test-noop-svc.exe";
        std::wstring dep_bin = GetCurrentExecutablePath() + L"\\lockbit-test-noop-svc-dep.exe";
        CreateNoopSvc(STANDALONE_SVC_NAME, standalone_bin.c_str(), L"");
        StartNoopSvc(STANDALONE_SVC_NAME);

        // service with 2 dependencies. Parent -> child 1 & child 2
        CreateNoopSvc(PARENT_SVC_NAME, standalone_bin.c_str(), L"");
        StartNoopSvc(PARENT_SVC_NAME);
        CreateNoopSvc(DEP1_SVC_NAME, dep_bin.c_str(), PARENT_SVC_NAME);
        StartNoopSvc(DEP1_SVC_NAME);
        CreateNoopSvc(DEP2_SVC_NAME, dep_bin.c_str(), PARENT_SVC_NAME);
        StartNoopSvc(DEP2_SVC_NAME);

        // nested dependencies. Parent -> child level 1 -> child level 2 -> child level 3
        CreateNoopSvc(NESTED_PARENT_SVC_NAME, standalone_bin.c_str(), L"");
        StartNoopSvc(NESTED_PARENT_SVC_NAME);
        CreateNoopSvc(NESTED_L1_SVC_NAME, dep_bin.c_str(), NESTED_PARENT_SVC_NAME);
        StartNoopSvc(NESTED_L1_SVC_NAME);
        CreateNoopSvc(NESTED_L2_SVC_NAME, dep_bin.c_str(), NESTED_L1_SVC_NAME);
        StartNoopSvc(NESTED_L2_SVC_NAME);
        CreateNoopSvc(NESTED_L3_SVC_NAME, dep_bin.c_str(), NESTED_L2_SVC_NAME);
        StartNoopSvc(NESTED_L3_SVC_NAME);

        std::cout << "Created and started test services" << std::endl;
    }

    virtual void TearDown() {
        // Stop and delete services
        std::cout << "Stopping and deleting test services" << std::endl;
        for (std::wstring svc: test_services) {
            if (ServiceRunning(svc.c_str())) {
                StopNoopSvc(svc.c_str());
                std::wcout << L"Stopped test service " << svc << std::endl;
            }
            DeleteNoopSvc(svc.c_str());
            std::wcout << L"Deleted test service " << svc << std::endl;
        }
        std::cout << "Stopped and deleted test services" << std::endl;
    }
};

TEST_F(SvcTest, TestTerminateServices) {
    // Make sure test services are running and are not already disabled before we stop them
    for (std::wstring svc: test_services) {
        ASSERT_TRUE(ServiceRunning(svc.c_str()));
        ASSERT_FALSE(ServiceDisabled(svc.c_str()));
    }

    std::vector<std::wstring> svc_names = {
        L"lockbit-test-svc-dne",
        STANDALONE_SVC_NAME,
        PARENT_SVC_NAME,
        NESTED_PARENT_SVC_NAME
    };
    tamper::TerminateAndDisableServices(svc_names);

    // Make sure services are stopped and disabled. Only the parent/standalone services get disabled.
    std::vector<std::wstring> disabled_services = {
        STANDALONE_SVC_NAME,
        PARENT_SVC_NAME,
        NESTED_PARENT_SVC_NAME
    };
    for (std::wstring svc: test_services) {
        ASSERT_TRUE(ServiceStopped(svc.c_str()));
    }
    for (std::wstring svc: disabled_services) {
        ASSERT_TRUE(ServiceDisabled(svc.c_str()));
    }
}