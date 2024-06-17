use std::ffi::CStr;
use std::error::Error;
use std::mem;
use std::net::Ipv4Addr;
use std::str;
use windows::Win32::Foundation::{ERROR_BUFFER_OVERFLOW, ERROR_SUCCESS};
use windows::Win32::NetworkManagement::IpHelper::{GetAdaptersInfo, IP_ADAPTER_INFO};
use encryptliterals::esl;
use crate::logger::{log_debug, log_error};


/**
 * get_local_ipv4_and_cidr
 *      About:
 *          Finds the local IP addresses and CIDR masks for the current machine.
 *      Result:
 *          Vector of IP address and CIDR mask tuples.
 *      MITRE ATT&CK Techniques:
 *          T1016: System Network Configuration Discovery
 */
pub fn get_local_ipv4_and_cidr() -> Result<Vec<(Ipv4Addr, u32)>, Box<dyn Error>> {
    // Reference: https://learn.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersinfo
    let mut result_vec = Vec::<(Ipv4Addr, u32)>::new();
    let mut buf_len = mem::size_of::<IP_ADAPTER_INFO>() as u32;
    let mut adapter_info_buf = vec![0; buf_len as usize];
    let mut p_adapter_info_buf: *mut IP_ADAPTER_INFO = adapter_info_buf.as_mut_ptr() as *mut IP_ADAPTER_INFO;
    let mut p_curr_adapter_info: *mut IP_ADAPTER_INFO;
    let mut result = unsafe {
        GetAdaptersInfo(Some(p_adapter_info_buf), &mut buf_len)
    };

    // Reallocate buffer if needed
    if result == ERROR_BUFFER_OVERFLOW.0 {
        log_debug(format!("{} {}", esl!("Resizing buffer for GetAdaptersInfo to"), buf_len));
        adapter_info_buf = vec![0; buf_len as usize];
        p_adapter_info_buf = adapter_info_buf.as_mut_ptr() as *mut IP_ADAPTER_INFO;
        result = unsafe {
            GetAdaptersInfo(Some(p_adapter_info_buf), &mut buf_len)
        };
        if result != ERROR_SUCCESS.0 {
            Err(format!("{} {}", esl!("GetAdaptersInfo: Error code"), result))?
        }
    } else if result != ERROR_SUCCESS.0 {
        Err(format!("{} {}", esl!("GetAdaptersInfo: Error code"), result))?
    }

    // Loop through adapters to get IP address and netrange info
    log_debug(String::from(esl!("Walking through adapter info linked list")));
    p_curr_adapter_info = p_adapter_info_buf;
    while p_curr_adapter_info != std::ptr::null_mut() {
        unsafe {
            let adapter_name = match CStr::from_ptr(((*p_curr_adapter_info).AdapterName).as_ptr() as *const i8).to_str() {
                Ok(s) => s,
                Err(e) => {
                    log_error(format!("{}: {}", esl!("Failed to obtain adapter name"), e));
                    "Unknown"
                }
            };
            log_debug(format!("{} {}", esl!("Found adapter name"), adapter_name));

            // Get just the first IP address for the adapter for now
            match CStr::from_ptr(((*p_curr_adapter_info).IpAddressList.IpAddress.String).as_ptr() as *const i8).to_str() {
                Ok(ip_addr_str) => {
                    match CStr::from_ptr(((*p_curr_adapter_info).IpAddressList.IpMask.String).as_ptr() as *const i8).to_str() {
                        Ok(mask_str) => {
                            match convert_netmask_str_to_u32(mask_str) {
                                Ok(net_mask) => {
                                    log_debug(format!("{} {}/{} for adapter {}", esl!("Found IP address and CIDR"), ip_addr_str, net_mask, adapter_name));
                                    match ip_addr_str.parse::<Ipv4Addr>() {
                                        Ok(ip) => {
                                            result_vec.push((ip, net_mask));
                                        },
                                        Err(e) => {
                                            log_error(format!("{} {} {} {}: {}", esl!("Failed to parse IP address string"), ip_addr_str, esl!("to IPv4 addr for adapter"), adapter_name, e));
                                        }
                                    }
                                },
                                Err(e) => {
                                    log_error(format!("{} {} {} {}: {}", esl!("Failed to convert netmask string"), mask_str, esl!("to int for adapter"), adapter_name, e));
                                }
                            }
                        },
                        Err(e) => {
                            log_error(format!("{} {}, IP {}: {}", esl!("Failed to get netmask string for adapter"), adapter_name, ip_addr_str, e));
                        }
                    }
                },
                Err(e) => {
                    log_error(format!("{} {}: {}", esl!("Failed to get IP address string for adapter"), adapter_name, e));
                }
            };

            // Move on to next adapter
            p_curr_adapter_info = (*p_curr_adapter_info).Next;
        }
    }
    Ok(result_vec)
}

fn convert_netmask_str_to_u32(mask_str: &str) -> Result<u32, Box<dyn Error>> {
    // Netmask will look something like 255.255.255.0
    let octets = mask_str.parse::<Ipv4Addr>()?.octets();
    if octets.len() != 4 {
        Err(format!("{} {}", esl!("Invalid netmask"), mask_str))?
    }
    let mut mask_size: u32 = 0;
    for octet in octets.iter() {
        let mut val = *octet;
        while val > 0 {
            mask_size += 1;
            val = val << 1;
        }
    }
    Ok(mask_size)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_convert_netmask_str_to_u32() {
        let mut result = convert_netmask_str_to_u32("128.0.0.0").unwrap();
        assert_eq!(result, 1);

        result = convert_netmask_str_to_u32("192.0.0.0").unwrap();
        assert_eq!(result, 2);

        result = convert_netmask_str_to_u32("255.0.0.0").unwrap();
        assert_eq!(result, 8);

        result = convert_netmask_str_to_u32("255.224.0.0").unwrap();
        assert_eq!(result, 11);

        result = convert_netmask_str_to_u32("255.240.0.0").unwrap();
        assert_eq!(result, 12);

        result = convert_netmask_str_to_u32("255.248.0.0").unwrap();
        assert_eq!(result, 13);

        result = convert_netmask_str_to_u32("255.252.0.0").unwrap();
        assert_eq!(result, 14);

        result = convert_netmask_str_to_u32("255.254.0.0").unwrap();
        assert_eq!(result, 15);

        result = convert_netmask_str_to_u32("255.255.255.0").unwrap();
        assert_eq!(result, 24);

        result = convert_netmask_str_to_u32("255.255.255.252").unwrap();
        assert_eq!(result, 30);

        result = convert_netmask_str_to_u32("255.255.255.255").unwrap();
        assert_eq!(result, 32);
    }

    #[cfg(target_os = "windows")]
    #[test]
    fn test_get_local_ipv4_and_cidr() {
        get_local_ipv4_and_cidr().unwrap();
    }
}