#pragma once
#include "errors.hpp"
#include "handles.hpp"
#include <expected>

namespace common {

std::expected<unique_handle, windows_error>
create_transaction();

std::expected<void, windows_error>
rollback_transaction(HANDLE transaction);

std::expected<void, windows_error>
commit_transaction(HANDLE transaction);

} // namespace common
