#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#include <string>
#include <vector>

namespace sessions {

/*
 * GetConnectedHosts:
 *      About:
 *          Returns vector of remote hosts that have an active net session established on the server, such as via
 *          the "net use" command to access a share.
 *          Session information is obtained using the NetSessionEnum API.
 *      Result:
 *          vector of connected IP addresses or hostnames
 *      MITRE ATT&CK Techniques:
 *          T1049: System Network Connections Discovery
 *          T1018: Remote System Discovery
 */
std::vector<std::wstring> GetConnectedHosts();

} // namespace