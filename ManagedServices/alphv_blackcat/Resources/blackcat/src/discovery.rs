use std::error::Error;
use regex::Regex;
use uuid::Uuid;
use crate::logger::{log_error, log_info};
use encryptliterals::esl;

#[cfg(target_os = "windows")]
use {
    std::str,
    windows::core::PWSTR,
    windows::Win32::Foundation::{CloseHandle, HANDLE},
    windows::Win32::NetworkManagement::NetManagement::UNLEN,
    windows::Win32::Security::Authentication::Identity::{GetUserNameExW, NameDnsDomain, NameSamCompatible},
    windows::Win32::Security::{GetTokenInformation, TokenElevation, TOKEN_ELEVATION, TOKEN_QUERY},
    windows::Win32::System::Threading::{GetCurrentProcess, OpenProcessToken},
    windows::Win32::System::WindowsProgramming::{GetComputerNameW, MAX_COMPUTERNAME_LENGTH},
    crate::winapi_helper::*,
    crate::proc::exec_program,
    crate::windowscrate_helper::windows_get_last_error,
};

#[cfg(unix)]
use {
    std::ffi::{CStr, CString},
    std::fs::File,
    std::io::Read,
    libc::{geteuid, gethostname, getpwuid, _SC_HOST_NAME_MAX},
    crate::logger::log_debug,
};

/**
 * pub fn is_elevated() -> bool {

 *      About:
 *          Returns true if running under an elevated context, false otherwise or if an error occurs when checking permissions.
 *      Result:
 *          true if running under an elevated context, false otherwise or if an error occurs when checking permissions
 *      MITRE ATT&CK Techniques:
 *          T1033: System Owner/User Discovery
 *      CTI:
 *          https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/
 */
pub fn is_elevated() -> bool {
    #[cfg(target_os = "windows")] {
        let mut token_handle: HANDLE = HANDLE(0);
        let mut elevated = false;
        unsafe { 
            match OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &mut token_handle) {
                Ok(_) => {},
                Err(e) => {
                    log_error(format!("{} {}. {}.", esl!("Failed to call OpenProcessToken: Error code"), e.code().0, esl!("Assuming not elevated")));
                    return elevated;
                }
            }
        }

        let mut required_size = 0;
        unsafe {
            // Get required size
            match GetTokenInformation(
                token_handle, 
                TokenElevation, 
                Some(std::ptr::null_mut()),
                0,
                &mut required_size
            ) {
                Err(e) => {
                    if required_size == 0 {
                        log_error(format!("{} {}. {}.", esl!("Failed to get required size from GetTokenInformation: Error code"), e.code().0, esl!("Assuming not elevated")));
                    } else {
                        let mut buffer = vec![0; required_size as usize];
                        match GetTokenInformation(
                            token_handle, 
                            TokenElevation, 
                            Some(buffer.as_mut_ptr() as *mut core::ffi::c_void),
                            required_size,
                            &mut required_size
                        ) {
                            Ok(_) => {
                                let token_elev_info: TOKEN_ELEVATION = std::ptr::read(buffer.as_ptr() as *const _);
                                elevated = token_elev_info.TokenIsElevated != 0;
                            },
                            Err(e) => {
                                log_error(format!("{} {}. {}.", esl!("Failed to call GetTokenInformation (2nd time): Error code"), e.code().0, esl!("Assuming not elevated")));
                            }
                        }
                    }
                },
                Ok(_) => {
                    log_error(String::from(esl!("Initial call to GetTokenInformation did not return expected error. Assuming not elevated.")));
                }
            }
        }

        // Clean up
        unsafe { let _ = CloseHandle(token_handle); }
        return elevated;
    }

    #[cfg(unix)] {
        return unsafe { geteuid() } == 0;
    }
}

/**
 * get_username
 *      About:
 *          Returns the username of the current user running the program. If the user is a domain user, the domain name will be included.
 *      Result:
 *          Username string or error.
 *      MITRE ATT&CK Techniques:
 *          T1033: System Owner/User Discovery
 *      CTI:
 *          https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/
 */
pub fn get_username() -> Result<String, Box<dyn Error>> {
    #[cfg(target_os = "windows")] {
        let mut username_buf_len = UNLEN + 1;
        let mut username_buf: Vec::<u16> = vec![0 as u16; username_buf_len as usize];
        unsafe {
            if !(GetUserNameExW(NameDnsDomain, PWSTR::from_raw(username_buf.as_mut_ptr()), &mut username_buf_len).as_bool()) {
                let mut last_error = windows_get_last_error();
                log_error(format!("{}: {}.", esl!("GetUserNameExW (domain) failed with error code"), last_error));
                log_info(String::from(esl!("Attempting to get username information without domain info.")));
                if !(GetUserNameExW(NameSamCompatible, PWSTR::from_raw(username_buf.as_mut_ptr()), &mut username_buf_len).as_bool()) {
                    last_error = windows_get_last_error();
                    Err(format!("{}: {}.", esl!("GetUserNameExW (non-domain) failed with error code"), last_error))?
                } else {
                    let username = from_wstring(username_buf.as_slice());
                    return Ok(username);
                }
            } else {
                let username = from_wstring(username_buf.as_slice());
                return Ok(username);
            }
        }
    }
    
    #[cfg(unix)] {
        let euid = unsafe { geteuid() };
        let pw = unsafe { getpwuid(euid) };
        if pw.is_null() {
            let err_code = std::io::Error::last_os_error().raw_os_error().unwrap_or(0); // instead of errno
            Err(format!("{} {} {}: {}.", esl!("getpwuid failed for effective user ID"), euid, esl!("with error code"), err_code))?
        }
        unsafe {
            match CString::from_raw((*pw).pw_name).into_string() {
                Ok(s) => Ok(s),
                Err(e) => Err(format!("{}: {:?}", esl!("Failed to convert username CString to string"), e))?
            }
        }
    }
}

/**
 * get_hostname
 *      About:
 *          Returns the hostname of the local machine.
 *      Result:
 *          Hostname string or error.
 *      MITRE ATT&CK Techniques:
 *          T1082: System Information Discovery
 *      CTI:
 *          https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/
 */
pub fn get_hostname() -> Result<String, Box<dyn Error>> {
    #[cfg(target_os = "windows")] {
        let mut hostname_buf_len = MAX_COMPUTERNAME_LENGTH + 1;
        let mut hostname_buf: Vec::<u16> = vec![0 as u16; hostname_buf_len as usize];
        unsafe {
            match GetComputerNameW(PWSTR::from_raw(hostname_buf.as_mut_ptr()), &mut hostname_buf_len) {
                Err(e) => {
                    Err(format!("{}: {}.", esl!("GetComputerNameW failed with error code"), e.code().0))?
                },
                Ok(_) => {
                    let hostname = from_wstring(hostname_buf.as_slice());
                    return Ok(hostname);
                }
            }
        }
    }

    #[cfg(unix)] {
        const HOSTNAME_BUF_LEN: i32 = _SC_HOST_NAME_MAX + 1; // account for null terminator
        let mut hostname_buf = vec![0 as i8; HOSTNAME_BUF_LEN as usize];
        let result = unsafe { gethostname(hostname_buf.as_mut_ptr(), HOSTNAME_BUF_LEN as usize) };
        if result != 0 {
            let err_code = std::io::Error::last_os_error().raw_os_error().unwrap_or(0); // instead of errno
            Err(format!("{}: {}.", esl!("gethostname failed with error code"), err_code))?
        } else {
            unsafe { 
                Ok(CStr::from_ptr(hostname_buf.as_ptr()).to_str()?.to_string()) 
            }
        }
    }
}

/// Generate a random UUID
fn generate_uuid() -> String{
    let uuid = Uuid::new_v4().to_string();
    log_info(format!("{} {}", esl!("Generated new randomized UUID"), uuid));
    uuid
}

/**
 * get_uuid
 *      About:
 *          Get the system UUID by calling "wmic csproduct get UUID" for Windows, or by reading the contents of the /sys/class/dmi/id/product_uuid
 *          (if elevated) or /etc/machine-id (if not elevated) file for Linux. Returns a randomly generated UUID upon failure.
 *      Result:
 *          System UUID, or random uuid upon failure.
 *      MITRE ATT&CK Techniques:
 *          T1082: System Information Discovery
 *      CTI:
 *          https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware
 *          https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/
 *          https://www.varonis.com/blog/blackcat-ransomware
 */
pub fn get_uuid(_elevated: bool) -> String {
    #[cfg(target_os = "windows")] {
        // Set up our uuid regex
        let re = match Regex::new(r"[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}"){
            Ok(o) => o,
            Err(e) => {
                log_error(format!("{}: {}", esl!("Regex failed to initialize"), e));
                return generate_uuid();
            }
        };
        // spawn wmic process and extract stdout
        let stdout = match exec_program(esl!("wmic").as_str(), &[esl!("csproduct").as_str(), esl!("get").as_str(), esl!("UUID").as_str()]){
            Ok(s) => s.stdout,
            Err(_) => {
                log_error(String::from(esl!("wmic failed to execute")));
                return generate_uuid();
            }
        };
        // convert stdout to bytes
        let stdout = match str::from_utf8(&stdout){
            Ok(o) => o,
            Err(e) => {
                log_error(format!("{}: {}", esl!("Malformed utf-8 vector in stdout"), e));
                return generate_uuid();
            }
        };
        // Find the UUID in stdout
        match re.find(stdout){
            Some(s) => {
                let uuid = s.as_str();
                log_info(format!("{} {}", esl!("CSProduct UUID is"), uuid));
                return uuid.to_string();
            },
            None => {
                log_error(String::from(esl!("Failed to extract UUID from stdout")));
                return generate_uuid();
            }
        };
    }
    #[cfg(unix)] {
        if _elevated {
            log_debug(String::from(esl!("Elevated: getting UUID from /sys/class/dmi/id/product_uuid.")));
            // Set up our uuid regex
            let re = match Regex::new(r"[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}"){
                Ok(o) => o,
                Err(e) => {
                    log_error(format!("{} {}", esl!("Regex failed to initialize:"), e));
                    return generate_uuid();
                }
            };
            // print uuid and get output
            let mut file = match File::open(esl!("/sys/class/dmi/id/product_uuid")) {
                Ok(s) => s,
                Err(e) => {
                    log_error(format!("{}: {}", esl!("Failed to open /sys/class/dmi/id/product_uuid"), e));
                    return generate_uuid();
                }
            };
            let mut stdout = String::new();
            match file.read_to_string(&mut stdout) {
                Ok(_) => (),
                Err(e) => {
                    log_error(format!("{}: {}", esl!("Failed to read /sys/class/dmi/id/product_uuid"), e));
                    return generate_uuid();
                }
            };
            // Find the UUID in stdout
            match re.find(&stdout.trim()){
                Some(s) => {
                    let uuid = s.as_str();
                    log_info(format!("{} {}", esl!("CSProduct UUID is"), uuid));
                    return uuid.to_string();
                },
                None => {
                    log_error(String::from(esl!("Failed to extract UUID from /sys/class/dmi/id/product_uuid")));
                    return generate_uuid();
                }
            };
        } else {
            log_debug(String::from(esl!("Not elevated: getting machine ID from /etc/machine-id.")));
            let mut file = match File::open(esl!("/etc/machine-id")) {
                Ok(s) => s,
                Err(e) => {
                    log_error(format!("{}: {}", esl!("Failed to open /etc/machine-id"), e));
                    return generate_uuid();
                }
            };
            let mut contents = String::new();
            match file.read_to_string(&mut contents) {
                Ok(_) => return contents.trim().to_string(),
                Err(e) => {
                    log_error(format!("{}: {}", esl!("Failed to read /etc/machine-id"), e));
                    return generate_uuid();
                }
            };
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::str;
    use crate::proc::exec_program;

    #[test]
    fn test_get_username() {
        let output = exec_program("whoami", &[]).unwrap();
        let want = str::from_utf8(&(output.stdout)).unwrap().trim_end().to_string();
        let result = get_username().unwrap();
        assert_eq!(result, want)
    }

    #[test]
    fn test_get_hostname() {
        let output = exec_program("hostname", &[]).unwrap();
        let want = str::from_utf8(&(output.stdout)).unwrap().trim_end().to_string();
        let result = get_hostname().unwrap();
        assert_eq!(result, want)
    }
}