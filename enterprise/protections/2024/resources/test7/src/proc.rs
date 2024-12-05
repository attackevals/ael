use std::error::Error;
use std::process::{Command,Output};
use std::str;
use crate::logger::{log_debug};
use encryptliterals::esl;

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
        hello = "hello\r\n".as_bytes().to_vec();
        program = "cmd";
        args = ["/C", "echo hello"];

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