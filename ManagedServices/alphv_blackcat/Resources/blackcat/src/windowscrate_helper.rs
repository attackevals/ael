pub fn windows_get_last_error() -> i32 {
    unsafe { 
        match windows::Win32::Foundation::GetLastError() {
            Ok(_) => { return 0; },
            Err(e) => { return e.code().0; }
        };
    }
}