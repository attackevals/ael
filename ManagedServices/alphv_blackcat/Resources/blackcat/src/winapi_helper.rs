// Reference: https://dlcoder.medium.com/interacting-with-the-windows-registry-using-winapi-in-rust-54c0538b1ea

use std::ffi::OsString;
use std::os::windows::ffi::OsStringExt;
use std::os::windows::ffi::OsStrExt;
use winapi::um::winnt::WCHAR;
use winapi::um::errhandlingapi::GetLastError;
use winapi::shared::minwindef::DWORD;

// convert rust string to WINAPI wstring
pub fn to_wstring(value: &str) -> Vec<WCHAR> {
    return OsString::from(value).encode_wide().chain(Some(0)).collect();
}

// Convert WINAPI wstring to rust String
pub fn from_wstring(value: &[WCHAR]) -> String {
    let len = value.iter().position(|&x| x == 0).unwrap_or(value.len());
    return OsString::from_wide(&value[..len]).to_string_lossy().into_owned();
}

pub fn get_last_error() -> DWORD {
    unsafe { 
        return GetLastError();
    }
}