#include "sessions.hpp"
#include "logger.hpp"
#include "string_util.hpp"
#include "xor_obfuscation.hpp"
#include <lm.h>
#include <format>
#include <set>

namespace sessions {

// Reference: https://learn.microsoft.com/en-us/windows/win32/api/lmshare/nf-lmshare-netsessionenum
std::vector<std::wstring> GetConnectedHosts() {
    LPSESSION_INFO_0 p_buf = NULL;
    LPSESSION_INFO_0 p_tmp_buf;
    DWORD entries_read = 0;
    DWORD total_entries = 0;
    DWORD total_processed = 0;
    std::set<std::wstring> sessions;

    Logger::LogInfo(XOR_LIT("Enumerating net sessions."));

    NET_API_STATUS n_status = NetSessionEnum(
        NULL, // local computer
        NULL, // all sessions
        NULL, // all users
        0, // get computer names
        (LPBYTE*)&p_buf,
        MAX_PREFERRED_LENGTH,
        &entries_read,
        &total_entries,
        NULL
    );
    do {
        if ((n_status == NERR_Success) || (n_status == ERROR_MORE_DATA)) {
            if ((p_tmp_buf = p_buf) != NULL) {
                Logger::LogInfo(std::format(
                    "{} {} {} {} {}", 
                    XOR_LIT("NetSessionEnum found"), 
                    entries_read,
                    XOR_LIT("out of"),
                    total_entries,
                    XOR_LIT("sessions")
                ));

                // Process entries
                for (DWORD i = 0; i < entries_read; i++) {
                    if (p_tmp_buf == NULL) {
                        Logger::LogError(XOR_LIT("Encountered null pointer while processing NetSessionEnum entries."));
                        break;
                    }
                    std::wstring session_host(p_tmp_buf->sesi0_cname);
                    sessions.insert(session_host);
                    
                    Logger::LogDebug(std::format(
                        "{} {}", 
                        XOR_LIT("Found net session from host"), 
                        string_util::wstring_to_string(session_host)
                    ));
                    p_tmp_buf++;
                    total_processed++;
                }
            } else {
                Logger::LogError(XOR_LIT("NetSessionEnum provided null buffer"));
            }
        } else {
            Logger::LogError(std::format(
                "{} {}", 
                XOR_LIT("NetSessionEnum failed with status value of"), 
                n_status
            ));
        }
        if (p_buf != NULL) {
            NetApiBufferFree(p_buf);
            p_buf = NULL;
        }
    } while (n_status == ERROR_MORE_DATA);

    Logger::LogDebug(std::format(
        "{} {} {}", 
        XOR_LIT("Processed a total of"), 
        total_processed,
        XOR_LIT("sessions.")
    ));
    
    if (p_buf != NULL) {
        NetApiBufferFree(p_buf);
    }
    return std::vector<std::wstring>(sessions.begin(), sessions.end());
}

} // namespace
