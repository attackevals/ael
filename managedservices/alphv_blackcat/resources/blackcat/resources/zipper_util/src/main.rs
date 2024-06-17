use miniz_oxide::deflate::compress_to_vec;
use std::io::Write;
use std::fs::File;

fn main() {
    let zip = include_bytes!("..\\..\\PsExec64.exe");
    let compressed = compress_to_vec(zip, 6);
    let mut file = match File::create("..\\psexec64.zip") {
        Ok(s) => s,
        Err(e) => panic!("Error: {}", e),
    };
    let _result = match file.write_all(&compressed) {
        Ok(s) => s,
        Err(e) => panic!("Error: {}", e),
    };
}
