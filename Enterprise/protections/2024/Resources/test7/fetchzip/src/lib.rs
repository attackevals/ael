use proc_macro::TokenStream;
use syn::{parse_macro_input, LitStr, Token, parse::{Parse, ParseStream}};
use std::fs::{File, create_dir_all};
use std::io::prelude::*;
use std::path::Path;
use zip::{ZipArchive};

struct FetchZipMacroInput {
    file_path: LitStr,
    _comma: Token![,],
    url: LitStr,
    _comma2: Token![,],
    zip_dest: LitStr,
    _comma3: Token![,],
    extract_dir: LitStr
}

impl Parse for FetchZipMacroInput {
    fn parse(input: ParseStream) -> syn::Result<Self> {
        Ok(Self {
            file_path: input.parse()?,
            _comma: input.parse()?,
            url: input.parse()?,
            _comma2: input.parse()?,
            zip_dest: input.parse()?,
            _comma3: input.parse()?,
            extract_dir: input.parse()?
        })
    }
}

/// Download zip from URL and extract into specified directory
#[proc_macro]
pub fn fetch_zip(input: TokenStream) -> TokenStream {
    let parsed = parse_macro_input!(input as FetchZipMacroInput);
    let src_on_disk = String::from((&parsed.file_path).value());
    let zip_url = String::from((&parsed.url).value());
    let dest_file_name = String::from((&parsed.zip_dest).value());
    let extract_dir_name = String::from((&parsed.extract_dir).value());
    let extract_dir_path = Path::new(&extract_dir_name);

    // Download and save if not already existing
    if Path::new(&dest_file_name).exists() {
        println!("{} already exists. Skipping download.", dest_file_name);
    } else if !src_on_disk.is_empty() && Path::new(&src_on_disk).exists() {
        // If file path specified and exists, use it and skip downloading
        println!("Using existing zip file on disk at path {}. Skipping download.", src_on_disk);
        std::fs::copy(src_on_disk, dest_file_name.clone()).unwrap();
    } else {
        println!("Could not find zip file on disk at path {}.", src_on_disk);
        println!("Downloading {} and saving as {}", zip_url, dest_file_name);
        let mut downloaded_file = File::create(dest_file_name.clone()).unwrap();
        let zip_data = reqwest::blocking::get(zip_url)
            .unwrap()
            .bytes()
            .unwrap();
        println!("Downloaded {} bytes", zip_data.len());
        downloaded_file.write_all(&zip_data).unwrap();
        println!("Saved zip file");
        drop(downloaded_file);
    }
    
    // Extract
    if !extract_dir_path.exists() {
        std::fs::create_dir(extract_dir_path).unwrap();
    }
    let zip_file = File::open(dest_file_name).unwrap();
    println!("Extracting zip contents to {} dir", extract_dir_name);
    let mut archive = ZipArchive::new(zip_file).unwrap();
    for i in 0..archive.len() {
        let mut file = archive.by_index(i).unwrap();
        let outpath = match file.enclosed_name() {
            Some(path) => path.to_owned(),
            None => continue,
        };
        if (*file.name()).ends_with('/') {
            println!("File {} extracted to {}", i, outpath.display());
            create_dir_all(&outpath).unwrap();
        } else {
            let dest = extract_dir_path.join(outpath);
            println!("File {} extracted to {} ({} bytes)", i, dest.display(), file.size());
            let mut outfile = File::create(dest).unwrap();
            std::io::copy(&mut file, &mut outfile).unwrap();
        }
    }
    println!("Finished extracting zip contents.");
    
    TokenStream::new()
}