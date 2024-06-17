/// This code was adapted from https://github.com/jonkgrimes/nbtscanner/blob/main/src/lib.rs
/// Original Author: Jon Grimes
/// Original source repository: https://github.com/jonkgrimes/nbtscanner
/// Original repository license: https://github.com/jonkgrimes/nbtscanner/blob/main/LICENSE

use std::error::Error;
use std::io::ErrorKind;
use std::net::Ipv4Addr;
use std::net::UdpSocket;
use std::time::Duration;
use rayon::prelude::*;
use crate::logger::{log_info, log_error, log_debug};
use encryptliterals::esl;

use super::nbt_packet::NetBiosPacket;

const NET_BIOS_PORT: u16 = 137;

// Can obtain this data by performing a packet capture while running nbtstat -A IP_ADDR
const NBSTAT_WILDCARD_QUERY: [u8; 50] = [
    /////////
    // HEADER
    // https://datatracker.ietf.org/doc/html/rfc1002#section-4.2.1.1
    0xA2, 0xE4, // Transaction ID (ideally this will change with each request)
    0x00, 0x00, // OPCODE QUERY, no flags
    0x00, 0x01, // 1 question entry (QDCOUNT)
    0x00, 0x00, // 0 answer resource records (ANCOUNT)
    0x00, 0x00, // 0 authority resource records (NSCOUNT)
    0x00, 0x00, // 0 additional resource records (ARCOUNT)
    
    ////////////////////
    // QUESTION SECTION
    // https://datatracker.ietf.org/doc/html/rfc1002#section-4.2.1.2

    // NODE STATUS REQUEST (copied from what is used in nbtstat -A IP_ADDR)
    // Wildcard "*" question name
    // NetBIOS name blank padded to 16 characters - becomes "*" with 15 blanks
    // Each character in padded name divided into two hex characters
    // * in hex is 2A/ 2 + 0x41 = 0x43, A + 0x41 = 0x4B
    // blank 00 pads become 0x41 0x41, hence the remaining 30 0x41 characters (15 blanks*2)
    // source: http://docshare.tips/315-ip-behavior-iv-microsoft-networking_5859fb47b6d87fc4a18b663b.html
    0x20, // Length of first label in name
    0x43, 0x4b, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 
    0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 
    0x00, // end of name
    
    0x00, 0x21, // NBSTAT (NetBIOS Node status) question type
    0x00, 0x01, // Internet question class
];
const TIMEOUT_SECONDS: u64 = 2;

pub fn scan(ips: Vec<Ipv4Addr>) -> Result<Vec<Ipv4Addr>, Box<dyn Error>> {
    match ips.first() {
        Some(first) => {
            match ips.last() {
                Some(last) => {
                    log_info(format!(
                        "{} {} to {} ({} {})",
                        esl!("Scanning from"),
                        first,
                        last,
                        ips.len(),
                        esl!("total, ignoring own IP addresses")
                    ));
                },
                None => {
                    Err(String::from(esl!("Failed to get last IP from vector of IPs to scan.")))?
                }
            }
        },
        None => {
            Err(String::from(esl!("Failed to get first IP from vector of IPs to scan.")))?
        }
    };

    let mut results: Vec<NetBiosPacket> = ips.into_par_iter().filter_map(|ip| {
        // bind to port 0 and let the OS decide
        let socket = match UdpSocket::bind(esl!("0.0.0.0:0").as_str()) {
            Ok(s) => s,
            Err(e) => {
                log_error(format!("{}: {}", esl!("Failed to bind to UDP socket"), e));
                return None;
            }
        };
        socket.set_read_timeout(Some(Duration::new(TIMEOUT_SECONDS, 0))).ok();

        let mut buf: [u8; 1024] = [0; 1024];
        match socket.connect((ip, NET_BIOS_PORT)) {
            Ok(_) => {},
            Err(e) => {
                log_error(format!("{} {}: {}", esl!("Failed to connect to target IP"), ip, e));
                return None;
            }
        };

        match socket.send(&NBSTAT_WILDCARD_QUERY) {
            Ok(_) => match socket.recv(&mut buf) {
                Ok(number_of_bytes) => {
                    log_debug(format!("{} {}", esl!("Received response from"), ip));
                    let packet = NetBiosPacket::from(ip, buf.clone(), number_of_bytes);
                    Some(packet)
                }
                Err(e) => {
                    match e.kind() {
                        ErrorKind::TimedOut => {
                            log_debug(format!("{} {}", esl!("Timed out when connecting to"), ip));
                        },
                        ErrorKind::ConnectionReset => {
                            log_debug(format!("{} {}", esl!("Connection closed when connecting to"), ip));
                        },
                        _ => {
                            log_error(format!("{} {}: {:?}", esl!("Encountered other error when contacting"), ip, e));
                        }
                    };
                    None
                }
            },
            Err(e) => {
                log_error(format!("{} {}: {}", esl!("Could not send data on the socket to target IP"), ip, e));
                None
            }
        }
    }).collect();
    results.sort_by(|a, b| a.ip.cmp(&b.ip)); // NOTE: This sort is in place hence the `mut` on results

    let mut result_vec = Vec::new();
    log_info(String::from(esl!("Discovered the following hosts:")));
    for result in results {
        log_info(format!(
           "{ip:<16}{group_and_name:<32}{mac:<15}",
            ip = format!("{}", result.ip),
            group_and_name = result.group_and_name(),
            mac = result.mac_address()
        ));
        result_vec.push(result.ip);
    }

    Ok(result_vec)
}
