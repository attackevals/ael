#include "transactions.hpp"
#include <gtest/gtest.h>

TEST(TransactionTests, CreateTransaction) {
    ASSERT_TRUE(common::create_transaction().has_value());
}

TEST(TransactionTests, CommitTransaction) {
    auto transaction = common::create_transaction();

    ASSERT_TRUE(common::commit_transaction(
        transaction.value().get()).has_value()
    );
}

TEST(TransactionTests, RollbackTransaction) {
    auto transaction = common::create_transaction();

    ASSERT_TRUE(common::rollback_transaction(
        transaction.value().get()).has_value()
    );
}
