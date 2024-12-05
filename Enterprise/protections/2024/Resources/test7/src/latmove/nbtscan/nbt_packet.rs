/// This code was adapted from https://github.com/jonkgrimes/nbtscanner/blob/main/src/nbt_packet.rs
/// Original Author: Jon Grimes
/// Original source repository: https://github.com/jonkgrimes/nbtscanner
/// Original repository license: https://github.com/jonkgrimes/nbtscanner/blob/main/LICENSE

use std::fmt;
use std::fmt::Display;
use std::net::Ipv4Addr;
use crate::logger::log_error;
use encryptliterals::esl;

// https://datatracker.ietf.org/doc/html/rfc1002#section-4.2.1.1
// https://datatracker.ietf.org/doc/html/rfc1002#section-4.2.18
const NETBIOS_HEADER_LEN: usize = 12;
const WILDCARD_RR_NAME_LEN: usize = 34; // 1 byte for name length, 32 bytes for compressed wildcard name, 1 byte for 0x00 terminator
const RESP_DATA_LEN_OFFSET: usize = NETBIOS_HEADER_LEN + WILDCARD_RR_NAME_LEN + 8; // 2 bytes for response type, 2 bytes for response class, 4 bytes for TTL
const RESP_NUM_NAMES_OFFSET: usize = RESP_DATA_LEN_OFFSET + 2; // 2 bytes for response data length
const RESP_FIRST_NAME_OFFSET: usize = RESP_NUM_NAMES_OFFSET + 1; // 1 byte for number of names in response. Offset should be 57 for standard response
const RESPONSE_NAME_LEN: usize = 15; // excluding NetBIOS name suffix
const RESPONSE_NAME_BLOCK_LEN: usize = 18; // 16 bytes for name (including suffix), and 2 bytes for name flags
const GROUP_FLAG: u8 = 0x80;

pub struct NetBiosPacket {
    pub ip: Ipv4Addr,
    pub data: [u8; 1024],
    pub length: usize,
    pub name: String,
    pub group: Option<String>,
}

impl Display for NetBiosPacket {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let mut values = String::new();
        for byte in self.data[0..self.length].iter() {
            values.push_str(&format!("0x{:01$X}, ", byte, 2));
        }
        write!(f, "[{}]", values)
    }
}

impl NetBiosPacket {
    pub fn from(ip: Ipv4Addr, data: [u8; 1024], length: usize) -> NetBiosPacket {
        // Calculate name and group
        let num_names = data[RESP_NUM_NAMES_OFFSET];
        let mut i = 0;
        let mut start = RESP_FIRST_NAME_OFFSET;
        let mut group_name: Option<String> = None;
        let mut netbios_name: String = String::from("");
        while i < num_names {
            let upper_bound = start + RESPONSE_NAME_LEN;
            let name_bytes = Vec::from(&data[start..upper_bound]);
            let name_flag_start = data[upper_bound + 1];

            // Check flags to see if the name is for a group or not
            if (name_flag_start & GROUP_FLAG) != 0 {
                match String::from_utf8(name_bytes) {
                    Ok(group) => {
                        let trimmed_group = group.trim_matches('\u{0}').trim_end();
                        group_name = Some(String::from(trimmed_group));
                    }
                    Err(_) => {
                        log_error(format!("{} {}", esl!("Could not decode the group name for IP"), ip));
                        group_name = None;
                    }
                }
            } else {
                // Prioritize workstation name (suffix of 0x00)
                let name_suffix = data[upper_bound];
                if name_suffix == 0x00 || netbios_name.is_empty() {
                    match String::from_utf8(name_bytes) {
                        Ok(matched_name) => {
                            netbios_name = String::from(matched_name.trim_end()); // remove any right-space padding for names < 15 characters
                        }
                        Err(_) => {
                            log_error(format!("{} {}", esl!("Could not decode the NetBIOS name for IP"), ip));
                        }
                    }
                }
            }
            i += 1;
            start += RESPONSE_NAME_BLOCK_LEN;
        }
        if group_name.is_none() {
            log_error(format!("{} {}", esl!("No NetBIOS group name found IP"), ip));
        }
        if netbios_name.is_empty() {
            log_error(format!("{} {}", esl!("Could not find a valid NetBIOS name for IP"), ip));
            netbios_name = String::from("N/A")
        }

        NetBiosPacket {
            ip,
            data,
            length,
            name: netbios_name,
            group: group_name,
        }
    }

    pub fn group_and_name(&self) -> String {
        if let Some(netbios_group) = &self.group {
            if !netbios_group.is_empty() {
                return format!("{}\\{}", netbios_group, &self.name);
            }
        }
        String::from(&self.name)
    }

    pub fn mac_address(&self) -> String {
        let num_names = self.data[RESP_NUM_NAMES_OFFSET] as usize;
        let mut name_bytes: [u8; 6] = [0; 6];
        let start = RESP_FIRST_NAME_OFFSET + RESPONSE_NAME_BLOCK_LEN * num_names;
        for n in 0..6 {
            let offset = start + n;
            name_bytes[n] = &self.data[offset] & 0xFF;
        }
        format!(
            "{:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}",
            name_bytes[0],
            name_bytes[1],
            name_bytes[2],
            name_bytes[3],
            name_bytes[4],
            name_bytes[5]
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn create_nbt_packet_from_data_slice() { // non-domain-joined, <15 char NetBIOS name
        let mut data = [0u8; 1024];
        let packet = [
            0xa2,0xe7, // transaction ID
            0x84,0x00, // name query response, server is authority for domain.  no flags, no error
            0x00,0x00, // 0 questions
            0x00,0x01, // 1 answer resource record
            0x00,0x00, // 0 authority resource records
            0x00,0x00, // 0 additional resource records
            
            // wildcard resource record name
            0x20,0x43,0x4b,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
            0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x00,

            0x00,0x21, // NBSTAT (NetBIOS Node status) resource record type
            0x00,0x01, // Internet resource record class
            0x00,0x00,0x00,0x00, // TTL
            0x00,0x65, // Data length
            0x03, // number of names
            
            // name 1 (suffix 0x00)
            0x57,0x4f,0x52,0x4b,0x53,0x54,0x41,0x54,0x49,0x4f,0x4e,0x31,0x20,0x20,0x20,0x00,
            0x04,0x00, // name 1 flags
            
            // name 2 (suffix 00)
            0x57,0x4f,0x52,0x4b,0x47,0x52,0x4f,0x55,0x50,0x20,0x20,0x20,0x20,0x20,0x20,0x00,
            0x84,0x00, // name 2 flags
            
            // name 3 (suffix 0x20)
            0x57,0x4f,0x52,0x4b,0x53,0x54,0x41,0x54,0x49,0x4f,0x4e,0x31,0x20,0x20,0x20,0x20,
            0x04,0x00, // name 3 flags
  
            0x47,0x43,0xbc,0xa6,0xf3,0x6c, // mac address
            
            // other statistics
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        ];
        for (i, elem) in packet.iter().enumerate() {
            data[i] = *elem;
        }
        let actual = NetBiosPacket::from(Ipv4Addr::from([127, 0, 0, 1]), data, 157 as usize);
        assert_eq!(actual.name, "WORKSTATION1");
        assert_eq!(actual.group, Some(String::from("WORKGROUP")));
        assert_eq!(actual.mac_address(), "47:43:BC:A6:F3:6C");
        assert_eq!(actual.group_and_name(), "WORKGROUP\\WORKSTATION1");
    }

    #[test]
    fn create_nbt_packet_from_data_slice2() { // domain-joined, 15-char NetBIOS name, <15-char domain name
        let mut data = [0u8; 1024];
        let packet = [
            0xa3,0xe2, // transaction ID
            0x84,0x00, // name query response, server is authority for domain.  no flags, no error
            0x00,0x00, // 0 questions
            0x00,0x01, // 1 answer resource record
            0x00,0x00, // 0 authority resource records
            0x00,0x00, // 0 additional resource records
            
            // wildcard resource record name
            0x20,0x43,0x4b,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
            0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x00,

            0x00,0x21, // NBSTAT (NetBIOS Node status) resource record type
            0x00,0x01, // Internet resource record class
            0x00,0x00,0x00,0x00, // TTL
            0x00,0x65, // Data length
            0x03, // number of names
            
            // name 1 (suffix 0x00)
            0x57,0x4f,0x52,0x4b,0x53,0x54,0x41,0x54,0x49,0x4f,0x4e,0x4e,0x41,0x4d,0x45,0x00,
            0x04,0x00, // name 1 flags
            
            // name 2 (suffix 00)
            0x52,0x45,0x44,0x44,0x45,0x56,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,
            0x84,0x00, // name 2 flags
            
            // name 3 (suffix 0x20)
            0x57,0x4f,0x52,0x4b,0x53,0x54,0x41,0x54,0x49,0x4f,0x4e,0x4e,0x41,0x4d,0x45,0x20,
            0x04,0x00, // name 3 flags
  
            0x47,0x43,0xbc,0xa6,0xf3,0x6c, // mac address
            
            // other statistics
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        ];
        for (i, elem) in packet.iter().enumerate() {
            data[i] = *elem;
        }
        let actual = NetBiosPacket::from(Ipv4Addr::from([127, 0, 0, 1]), data, 157 as usize);
        assert_eq!(actual.name, "WORKSTATIONNAME");
        assert_eq!(actual.group, Some(String::from("REDDEV")));
        assert_eq!(actual.mac_address(), "47:43:BC:A6:F3:6C");
        assert_eq!(actual.group_and_name(), "REDDEV\\WORKSTATIONNAME");
    }

    #[test]
    fn create_nbt_packet_from_data_slice3() { // domain-joined, 15-char NetBIOS name, 15-char domain name
        let mut data = [0u8; 1024];
        let packet = [
            0xa3,0xe2, // transaction ID
            0x84,0x00, // name query response, server is authority for domain.  no flags, no error
            0x00,0x00, // 0 questions
            0x00,0x01, // 1 answer resource record
            0x00,0x00, // 0 authority resource records
            0x00,0x00, // 0 additional resource records
            
            // wildcard resource record name
            0x20,0x43,0x4b,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
            0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x00,

            0x00,0x21, // NBSTAT (NetBIOS Node status) resource record type
            0x00,0x01, // Internet resource record class
            0x00,0x00,0x00,0x00, // TTL
            0x00,0x65, // Data length
            0x03, // number of names
            
            // name 1 (suffix 0x00)
            0x57,0x4f,0x52,0x4b,0x53,0x54,0x41,0x54,0x49,0x4f,0x4e,0x4e,0x41,0x4d,0x45,0x00,
            0x04,0x00, // name 1 flags
            
            // name 2 (suffix 00)
            0x4d,0x59,0x4f,0x57,0x4e,0x44,0x4f,0x4d,0x41,0x49,0x4e,0x4e,0x41,0x4d,0x45,0x00,
            0x84,0x00, // name 2 flags
            
            // name 3 (suffix 0x20)
            0x57,0x4f,0x52,0x4b,0x53,0x54,0x41,0x54,0x49,0x4f,0x4e,0x4e,0x41,0x4d,0x45,0x20,
            0x04,0x00, // name 3 flags
  
            0x47,0x43,0xbc,0xa6,0xf3,0x6c, // mac address
            
            // other statistics
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        ];
        for (i, elem) in packet.iter().enumerate() {
            data[i] = *elem;
        }
        let actual = NetBiosPacket::from(Ipv4Addr::from([127, 0, 0, 1]), data, 157 as usize);
        assert_eq!(actual.name, "WORKSTATIONNAME");
        assert_eq!(actual.group, Some(String::from("MYOWNDOMAINNAME")));
        assert_eq!(actual.mac_address(), "47:43:BC:A6:F3:6C");
        assert_eq!(actual.group_and_name(), "MYOWNDOMAINNAME\\WORKSTATIONNAME");
    }


    #[test]
    fn parse_name_and_group_from_data_correctly() {
        let mut data = [0u8; 1024];
        let packet = [
            0xA2, 0x48, // transaction ID
            0x84, 0x00, // name query response, server is authority for domain.  no flags, no error
            0x00, 0x00, // 0 questions
            0x00, 0x01, // 1 answer resource record
            0x00, 0x00, // 0 authority resource records
            0x00, 0x00, // 0 additional resource records

            // wildcard resource record name
            0x20, 0x43, 0x4B, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
            0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 
            0x41, 0x00, 
            
            0x00, 0x21, // NBSTAT (NetBIOS Node status) resource record type
            0x00, 0x01, // Internet resource record class
            0x00, 0x00, 0x00, 0x00, // TTL
            0x00, 0x77, // data length
            0x04, // number of names
            
            // name 1 (suffix 0x00)
            0x41, 0x4C, 0x45, 0x58, 0x4B, 0x2D, 0x50, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 
            0x44, 0x00, // name 1 flags
            
            // name 2 (suffix 00)
            0x53, 0x50, 0x49, 0x43, 0x45, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 
            0xC4, 0x00, // name 2 flags
            
            // name 3
            0x41, 0x4C, 0x45, 0x58, 0x4B, 0x2D, 0x50, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
            0x44, 0x00, // name 3 flags
            
            // name 4
            0x53, 0x50, 0x49, 0x43, 0x45, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1E,
            0xC4, 0x00, // name 4 flags
            
            0xD0, 0xBF, 0x9C, 0xE4, 0x24, 0x90, // mac address
            
            // other statistics
            0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        ];
        for (i, elem) in packet.iter().enumerate() {
            data[i] = *elem;
        }
        let expected = "SPICE\\ALEXK-PC";
        let actual = NetBiosPacket::from(Ipv4Addr::from([127, 0, 0, 1]), data, 175 as usize);
        assert_eq!(expected, actual.group_and_name());
    }

    #[test]
    fn parse_mac_from_data_correctly() {
        let mut data = [0u8; 1024];
        let packet = [
            0xA2, 0x48, // transaction ID
            0x84, 0x00, // name query response, server is authority for domain.  no flags, no error
            0x00, 0x00, // 0 questions
            0x00, 0x01, // 1 answer resource record
            0x00, 0x00, // 0 authority resource records
            0x00, 0x00, // 0 additional resource records
            
            // wildcard resource record name
            0x20, 0x43, 0x4B, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
            0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 
            0x41, 0x00, 
            
            0x00, 0x21, // NBSTAT (NetBIOS Node status) resource record type
            0x00, 0x01, // Internet resource record class
            0x00, 0x00, 0x00, 0x00, // TLLD
            0x00, 0x77, // data length
            0x04, // number of names

            // name 1
            0x4A, 0x41, 0x43, 0x4B, 0x49, 0x45, 0x47, 0x2D, 0x57, 0x53, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
            0x44, 0x00, // name 1 flags
            
            // name 2
            0x4A, 0x41, 0x43, 0x4B, 0x49, 0x45, 0x47, 0x2D, 0x57, 0x53, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 
            0x44, 0x00, // name 2 flags
            
            // name 3
            0x53, 0x50, 0x49, 0x43, 0x45, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00,
            0xC4, 0x00, // name 3 flags
            
            // name 4
            0x53, 0x50, 0x49, 0x43, 0x45, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1E, 
            0xC4, 0x00, // name 4 flags
            
            0x2C, 0x41, 0x38, 0xBA, 0xC3, 0x64, // mac address
            
            // other statistics
            0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        ];
        for (i, elem) in packet.iter().enumerate() {
            data[i] = *elem;
        }
        let expected = "2C:41:38:BA:C3:64";
        let actual = NetBiosPacket::from(Ipv4Addr::from([127, 0, 0, 1]), data, 175 as usize);
        assert_eq!(expected, actual.mac_address());
    }
}
