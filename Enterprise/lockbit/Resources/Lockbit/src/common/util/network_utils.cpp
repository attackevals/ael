#include <format>
#include "crypto/xor_obfuscation.hpp"
#include "util/network_util.hpp"

namespace network_util {

// Only for non-negative exponents
int IntPow(int a, unsigned int exp) {
    int result = 1;
    for (int i = 1; i <= exp; i++) 
        result *= a;
    return result;
}

// Converts IPAddr into dotted-quad IPv4 string (e.g. "10.1.2.3")
std::string IPAddrToString(IPAddr ip_addr) {
    IN_ADDR in_addr;
    in_addr.S_un.S_addr = ip_addr;
    return std::format(
        "{}.{}.{}.{}", 
        in_addr.S_un.S_un_b.s_b1, // octet 1
        in_addr.S_un.S_un_b.s_b2, // octet 2
        in_addr.S_un.S_un_b.s_b3, // octet 3
        in_addr.S_un.S_un_b.s_b4  // octet 4
    );
}

// Converts the dotted-quad net mask IP string into the corresponding CIDR mask size
// For instance, 255.255.255.0 is converted to 24 to represent the /24 CIDR mask
uint32_t GetNetMaskSize(const std::string& net_mask_str) {
    // Netmask will look something like 255.255.255.0
    ULONG net_mask_int = ntohl(inet_addr(net_mask_str.c_str()));
    uint32_t mask_size = 0;
    while (net_mask_int > 0) {
        mask_size += 1;
        net_mask_int = net_mask_int << 1;
    }
    return mask_size;
}

// Given an IPv4 address value in host-byte order and a CIDR netmask size, returns the
// host-byte order value of the first IPv4 address in the corresponding subnet.
ULONG GetSubnetStart(ULONG addr, uint32_t netmask_size) {
    if (netmask_size > 32) {
        throw std::invalid_argument(std::format("{} {}", XOR_LIT("Bad netmask size"), netmask_size));
    } else if (netmask_size == 32) {
        return addr;
    }
    ULONG mask = 0xFFFFFFFF << (32 - netmask_size);
    return addr & mask;
}

// Given an IPv4 address value in host-byte order and a CIDR netmask size, returns the
// host-byte order value of the last IPv4 address in the corresponding subnet.
ULONG GetSubnetEnd(ULONG addr, uint32_t netmask_size) {
    if (netmask_size > 32) {
        throw std::invalid_argument(std::format("{} {}", XOR_LIT("Bad netmask size"), netmask_size));
    } else if (netmask_size == 32) {
        return addr;
    }
    ULONG mask = 0xFFFFFFFF >> netmask_size;
    return addr | mask;
}

// Returns true if address (host byte order) is a loopback address (within 127.0.0.0/8), false if not.
bool IsLoopback(ULONG addr) {
    return (addr >= 0x7F000000 && addr <= 0x7FFFFFFF);
}


// Returns true if address (host byte order) is a multicast address (within 224.0.0.0 through 239.255.255.255),
// false if not.
bool IsMulticast(ULONG addr) {
    return (addr >= 0xE0000000 && addr <= 0xEFFFFFFF);
}

// Converts a CIDR range string (e.g. "10.0.2.3/24") to a vector containing the 
// IP addresses (network byte order) in the corresponding range.
std::vector<IPAddr> ConvertCidrRangeStringToIPAddrs(const std::string& cidr_str) {
    auto index = cidr_str.find("/");
    if (index == cidr_str.npos) {
        throw std::invalid_argument(std::format("{} {}", XOR_LIT("Bad CIDR format for input"), cidr_str));
    }
    std::string network_addr_str = cidr_str.substr(0, index);
    IPAddr network_addr = inet_addr(network_addr_str.c_str());
    if (network_addr == INADDR_NONE) {
        throw std::invalid_argument(std::format("{} {}", XOR_LIT("Bad network address format for input"), cidr_str));
    }
    ULONG network_addr_ho = ntohl(network_addr); // network byte order to host byte order
    std::string mask_str = cidr_str.substr(index + 1);
    uint32_t mask = std::stoul(mask_str);
    ULONG start_val = GetSubnetStart(network_addr_ho, mask);
    ULONG end_val = GetSubnetEnd(network_addr_ho, mask);
    std::vector<ULONG> ret;
    ret.reserve(IntPow(2, 32 - mask));
    for (ULONG i = start_val; i <= end_val; i++) {
        ret.push_back(htonl(i));
    }
    return ret;
}

} // namespace