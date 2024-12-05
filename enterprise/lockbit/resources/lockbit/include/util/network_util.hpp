#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <string>
#include <vector>
#include <Windows.h>
#include <winsock2.h>
#include <iphlpapi.h>

// Defined in src/common/util/network_utils.cpp
namespace network_util {

// Raises a to the exp power.
int IntPow(int a, unsigned int exp);

// Converts IPAddr into dotted-quad IPv4 string (e.g. "10.1.2.3")
std::string IPAddrToString(IPAddr ip_addr);

// Given an IPv4 address value in host-byte order and a CIDR netmask size, returns the
// host-byte order value of the first IPv4 address in the corresponding subnet.
ULONG GetSubnetStart(ULONG addr, uint32_t netmask_size);

// Given an IPv4 address value in host-byte order and a CIDR netmask size, returns the
// host-byte order value of the last IPv4 address in the corresponding subnet.
ULONG GetSubnetEnd(ULONG addr, uint32_t netmask_size);

// Returns true if address (host byte order) is a loopback address (within 127.0.0.0/8), false if not.
bool IsLoopback(ULONG addr);

// Returns true if address (host byte order) is a multicast address (within 224.0.0.0 through 239.255.255.255),
// false if not.
bool IsMulticast(ULONG addr);

// Converts the dotted-quad net mask IP string into the corresponding CIDR mask size
// For instance, 255.255.255.0 is converted to 24 to represent the /24 CIDR mask
uint32_t GetNetMaskSize(const std::string& net_mask_str);

// Converts a CIDR range string (e.g. "10.0.2.3/24") to a vector containing the 
// IP addresses (network byte order) in the corresponding range.
std::vector<IPAddr> ConvertCidrRangeStringToIPAddrs(const std::string& cidr_str);

} // namespace