use std::error::Error;
use log::{LevelFilter, debug, error, info};
use log4rs::{
    append::{
        console::{ConsoleAppender, Target},
        file::FileAppender,
    },
    config::{Appender, Config, Root},
    encode::pattern::PatternEncoder,
    filter::threshold::ThresholdFilter, 
};

use encryptliterals::esl;

#[cfg(target_os = "windows")]
const LOGFILE: &str = "clog.xtlog";

#[cfg(unix)]
const LOGFILE: &str = "bc.log";

pub fn config_logger() -> Result<(), Box<dyn Error>> {
 
    // Build a stdout logger.
    let stdout = ConsoleAppender::builder().target(Target::Stdout).build();

    // Logging to log file.
    let logfile = FileAppender::builder()
        .encoder(Box::new(PatternEncoder::new(esl!("[{l}] [{d(%Y-%m-%d %H:%M:%S)}] {m}\n").as_str())))
        .build(LOGFILE)?;

    // Set up logging to stdout and a logfile
    let config = Config::builder()
        .appender(Appender::builder().build("logfile", Box::new(logfile)))
        .appender(
            Appender::builder()
                .filter(Box::new(ThresholdFilter::new(LevelFilter::Debug)))
                .build("stdout", Box::new(stdout)),
        )
        .build(
            Root::builder()
                .appender("logfile")
                .appender("stdout")
                .build(LevelFilter::Trace),
        )?;

    // Configure global logger
    let _handle = log4rs::init_config(config)?;

    Ok(())
}


// log to console unencrypted and log to file unencrypted

pub fn log_info(message: String) {
    info!("{}", message);
}

pub fn log_error(message: String) {
    error!("{}", message);
}

pub fn log_debug(message: String) {
    debug!("{}", message);
}