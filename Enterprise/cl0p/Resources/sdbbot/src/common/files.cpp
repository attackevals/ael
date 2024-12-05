#include "files.hpp"

namespace common {

/*
 * read_file:
 *      About:
 *          Fetches specified file contents using the ReadFile API call
 *      Result:
 *          vector of std::bytes representing file contents
 */
std::expected<std::vector<std::byte>, windows_error>
read_file(HANDLE handle) {
    LARGE_INTEGER size{};
    if (!GetFileSizeEx(handle, &size)) {
        return std::unexpected(get_last_error());
    }
    
    std::vector<std::byte> buf{ static_cast<uint64_t>(size.QuadPart) };
    unsigned long count;
    if (!ReadFile(handle, buf.data(), size.QuadPart, &count, nullptr)) {
        return std::unexpected(get_last_error());
    }

    return buf;
}

/*
 * create_symlink:
 *      About:
 *          Creates a symbolic link using the specified paths and flags.
 *          Relevant API call: CreateSymbolicLinkTransactedW
 *      Result:
 *          Symbolic link created at link_path that points to file_path.
 */
std::expected<void, windows_error>
create_symlink(
    HANDLE transaction,
    const std::filesystem::path& link_path,
    const std::filesystem::path& file_path,
    unsigned long flags
) {
    if (!CreateSymbolicLinkTransactedW(
        link_path.c_str(),
        file_path.c_str(),
        flags,
        transaction
    )) {
        return std::unexpected(get_last_error());
    }

    return {};
}

} // namespace common
