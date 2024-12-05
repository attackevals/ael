#include "transactions.hpp"
#include "errors.hpp"
#include <ktmw32.h> 

using std::expected, std::unexpected;

namespace common {

// Create a new transaction object, or return error on failure
expected<unique_handle, windows_error> create_transaction() {
    common::unique_handle transaction = CreateTransaction(
        nullptr,
        nullptr,
        0,
        0,
        0,
        0,
        nullptr
    );
    if (!transaction) {
        return unexpected(get_last_error("Failed to start transaction"));
    }
    return transaction;
}

// Roll back the specified transaction, or return error on failure
expected<void, windows_error> rollback_transaction(HANDLE transaction) {
    if (!RollbackTransaction(transaction)) {
        return unexpected(get_last_error("Failed to rollback transaction"));
    }
    return {};
}

// Commit the specified transaction, or return error on failure
expected<void, windows_error> commit_transaction(HANDLE transaction) {
    if (!CommitTransaction(transaction)) {
        return unexpected(get_last_error("Failed to commit transaction"));
    }
    return {};
}

}  // namespace common
