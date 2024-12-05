use aes::Aes256;
use aes::cipher::{generic_array::{typenum::U32, GenericArray}, KeyIvInit, StreamCipher};
use proc_macro::TokenStream;
use rand::{random};
use syn::{parse_macro_input, LitStr, Token, parse::{Parse, ParseStream}};
use miniz_oxide::deflate::compress_to_vec;
use std::fs::{File, read};
use std::io::prelude::*;
use std::path::Path;

struct ObfuscateResourceMacroInput {
    input: LitStr,
    _comma: Token![,],
    output: LitStr,
    _comma2: Token![,],
    key_str: LitStr
}

impl Parse for ObfuscateResourceMacroInput {
    fn parse(input: ParseStream) -> syn::Result<Self> {
        Ok(Self {
            input: input.parse()?,
            _comma: input.parse()?,
            output: input.parse()?,
            _comma2: input.parse()?,
            key_str: input.parse()?
        })
    }
}

fn aes_encrypt(input: &mut Vec<u8>, key: GenericArray<u8,U32>) -> Vec<u8> {
    type Aes256Ctr128BE = ctr::Ctr128BE<Aes256>;

    let iv = random::<[u8;16]>();
    let mut cipher = Aes256Ctr128BE::new(&key.into(), &iv.into());
    cipher.apply_keystream(input);

    // Prepend IV to ciphertext
    let mut ciphertext: Vec<u8> = Vec::new();
    ciphertext.extend_from_slice(&iv);
    ciphertext.append(input);
    return ciphertext;
}

/// Compress input file, AES-encrypt it using the key string, and then write
/// the IV + ciphertext to the output file
#[proc_macro]
pub fn obfuscate_resource(input: TokenStream) -> TokenStream {
    let parsed = parse_macro_input!(input as ObfuscateResourceMacroInput);
    let input = String::from((&parsed.input).value());
    let output = String::from((&parsed.output).value());
    let input_path = Path::new(&input);
    let output_path = Path::new(&output);

    let key_str = String::from((&parsed.key_str).value());
    let key_bytes = hex::decode(key_str.clone()).unwrap();

    if key_bytes.len() != 32 {
        panic!("Expected key size of 32 bytes, got {}", key_bytes.len());
    }

    let mut key = [0u8; 32];
    for i in 0..32 {
        key[i] = key_bytes[i];
    }

    println!("Compressing and encrypting {} with key: {}.", input, key_str);

    // Compress
    let data = read(input_path).unwrap();
    let mut compressed = compress_to_vec(&data, 6);

    // Encrypt
    let encrypted = aes_encrypt(&mut compressed, GenericArray::from(key));

    // Save to output
    let mut output_file = File::create(output_path).unwrap();
    output_file.write_all(&encrypted).unwrap();
    println!("Wrote compressed and encrypted resource to {}.", output);
    
    TokenStream::new()
}