#include "execute.hpp"
#include "util/string_util.hpp"
#include "test_utils.hpp"
#include <gtest/gtest.h>

class ExecuteTest : public ::testing::Test {
protected:
    virtual void TearDown() {
        test_utils::KillProcess(L"lockbit-test-noop-exe");
    }
};

TEST_F(ExecuteTest, TestSpawnProcessNoWait) {
    wchar_t process[] = L"lockbit-test-noop-exe.exe";
    EXPECT_EQ(execute::SpawnProcessNoWait(process), ERROR_SUCCESS);
    EXPECT_EQ(1, test_utils::GetNumProcesses(L"lockbit-test-noop-exe"));
}
