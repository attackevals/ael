#include "transactions.hpp"
#include "registry.hpp"
#include <gtest/gtest.h>
#include <vector>

class RegistryTests : public testing::Test {
protected:
    common::unique_handle m_transaction = INVALID_HANDLE_VALUE;
    const wchar_t* m_key = L"SDBBot-Test";
    const wchar_t* m_value = L"Test-Value";
    common::unique_hkey m_write = nullptr;
    common::unique_hkey m_read = nullptr;

    void SetUp() override {
        auto transaction = common::create_transaction();
        ASSERT_TRUE(transaction.has_value());
        m_transaction = transaction.value().release();

        auto write_handle = common::create_registry_key(
            m_transaction.get(),
            m_key,
            KEY_WRITE,
            HKEY_CURRENT_USER
        );
        ASSERT_TRUE(write_handle.has_value());
        m_write = write_handle.value().release();

        auto read_handle = common::create_registry_key(
            m_transaction.get(),
            m_key,
            KEY_READ,
            HKEY_CURRENT_USER
        );
        ASSERT_TRUE(read_handle.has_value());
        m_read = read_handle.value().release();
    }

    void TearDown() override {
        common::rollback_transaction(m_transaction.get());
    }
};

class EarlyRegistryTests : public testing::Test {
protected:
    std::vector<std::byte> bytes{
    std::byte(0x00), std::byte(0x11), std::byte(0x22), std::byte(0x33),
    std::byte(0x44), std::byte(0x55), std::byte(0x66), std::byte(0x77),
    std::byte(0x88), std::byte(0x99), std::byte(0xaa), std::byte(0xbb),
    std::byte(0xcc), std::byte(0xdd), std::byte(0xee), std::byte(0xff)
    };
    const wchar_t* m_key_early = L"SDBBot-Test-EarlyReadBinary";
    const wchar_t* m_value_early = L"Test-Value-Early";
    const wchar_t* m_key_early_fail = L"SDBBot-Test-EarlyReadBinary-Fail";
    const wchar_t* m_value_early_fail = L"Test-Value-Early-Fail";
    HKEY key = nullptr;
    LSTATUS status = 0;

    void SetUp() override {
        status = RegCreateKeyW(
            HKEY_CURRENT_USER,
            m_key_early,
            &key
        );
        ASSERT_TRUE(status == ERROR_SUCCESS);

        status = RegSetKeyValueW(
            key,
            nullptr,
            m_value_early,
            REG_BINARY,
            bytes.data(),
            bytes.size()
        );
        ASSERT_TRUE(status == ERROR_SUCCESS);
    }

    void TearDown() override {
        status = RegDeleteKeyW(
            HKEY_CURRENT_USER,
            m_key_early
        );
        ASSERT_TRUE(status == ERROR_SUCCESS);
    }
};

TEST_F(RegistryTests, ReadWriteString) {
    const wchar_t* test_data = L"0123456789abcdef0123456789abcdef";

    ASSERT_TRUE(common::write_registry(
        m_write.get(),
        m_value,
        test_data
    ).has_value());

    auto data = common::read_registry_string(
        m_read.get(),
        m_value
    );
    ASSERT_TRUE(data.has_value());

    EXPECT_EQ(0, data.value() == test_data);
}

TEST_F(RegistryTests, ReadWriteBinary) {
    std::vector<std::byte> bytes{
        std::byte(0x00), std::byte(0x11), std::byte(0x22), std::byte(0x33),
        std::byte(0x44), std::byte(0x55), std::byte(0x66), std::byte(0x77),
        std::byte(0x88), std::byte(0x99), std::byte(0xaa), std::byte(0xbb),
        std::byte(0xcc), std::byte(0xdd), std::byte(0xee), std::byte(0xff)
    };

    ASSERT_TRUE(common::write_registry(
        m_write.get(),
        m_value,
        bytes
    ).has_value());

    auto data = common::read_registry_binary(
        m_read.get(),
        m_value
    );
    ASSERT_TRUE(data.has_value());

    EXPECT_EQ(0, memcmp(data.value().data(), bytes.data(), 16));
}

TEST_F(RegistryTests, ReadWriteDWORD) {
    unsigned long test_data = 0x01234567;

    ASSERT_TRUE(common::write_registry(
        m_write.get(),
        m_value,
        test_data
    ).has_value());

    auto data = common::read_registry_dword(
        m_read.get(),
        m_value
    );
    ASSERT_TRUE(data.has_value()) << data.error().what();

    EXPECT_EQ(data.value(), test_data);
}

TEST_F(EarlyRegistryTests, EarlyReadBinary) {

    auto ret = common::early_read_registry_binary(
        HKEY_CURRENT_USER,
        m_key_early,
        m_value_early
    );
    ASSERT_TRUE(ret.has_value());
    EXPECT_EQ(0, memcmp(ret.value().data(), bytes.data(), 16));

    auto ret_fail = common::early_read_registry_binary(
        HKEY_CURRENT_USER,
        m_key_early_fail,
        m_value_early_fail
    );
    ASSERT_TRUE(!ret_fail.has_value());
}