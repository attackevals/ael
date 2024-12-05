#include "files.hpp"
#include "transactions.hpp"
#include "handles.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <print>

class FileTests : public testing::Test {
protected:
    common::unique_handle m_transaction = INVALID_HANDLE_VALUE;
    const wchar_t* m_path = L"SDBBot-Test";

    void SetUp() override {
        auto transaction = common::create_transaction();
        ASSERT_TRUE(transaction.has_value());
        m_transaction = transaction.value().release();
    }

    void TearDown() override {
        common::rollback_transaction(m_transaction.get());
    }
};

TEST_F(FileTests, WriteBytes) {
    std::vector<std::byte> bytes{
        std::byte(0x00), std::byte(0x11), std::byte(0x22), std::byte(0x33),
        std::byte(0x44), std::byte(0x55), std::byte(0x66), std::byte(0x77),
        std::byte(0x88), std::byte(0x99), std::byte(0xaa), std::byte(0xbb),
        std::byte(0xcc), std::byte(0xdd), std::byte(0xee), std::byte(0xff)
    };

    auto write = common::write_file(
        m_transaction.get(),
        bytes,
        m_path
    );
    ASSERT_TRUE(write.has_value());

    auto read = common::read_file(write.value().get());
    ASSERT_TRUE(read.has_value());

    EXPECT_EQ(bytes.size(), read.value().size());
}

TEST_F(FileTests, CreateSymlink) {
    auto linked = common::create_symlink(
        m_transaction.get(),
        LR"(./symlink.test)",
        LR"(C:\Windows\temp\nonexistent.txt)",
        SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
    );

    // Symlinks require admin privileges.
    // 1314 is the code you will get when running as a normal user.
    EXPECT_TRUE(linked.has_value() || linked.error().code().value() == 1314);
}
