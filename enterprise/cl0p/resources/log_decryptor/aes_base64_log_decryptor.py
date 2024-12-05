# ---------------------------------------------------------------------------
# aes_base64_log_decryptor.py - Decrypts the a given log file with a specific
# key and encryption mode and saves decrypted contents to the specified output
# file.
# Log files are expected to be of a certain format, where each line
# was encrypted using a symmetric encryption algorithm, and
# the iv+ciphertext is then base64-encoded: base64(IV + aes(plaintext))
# The following underlying symmetric encryption algorithms are supported:
#   - 256- or 128-bit AES in CTR or CBC mode
#   - XOR encryption

# Requires PyCryptodome: pip install pycryptodome

# Copyright 2023 MITRE Engenuity. Approved for public release. Document number TBD.
# Licensed under the Apache License, Version 2.0 (the "License"); 
# you may not use this file except in compliance with the License. 
# You may obtain a copy of the License at 
# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software 
# distributed under the License is distributed on an "AS IS" BASIS, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
# See the License for the specific language governing permissions and 
# limitations under the License.

# This project makes use of ATT&CK®
# ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 

# Usage: 
# python3 aes_base64_log_decryptor.py -i /path/to/encrypted/log 
#   -o /path/to/save/output -k [KEY IN HEX] 
#   [--aes-256-cbc|--aes-128-cbc|--aes-256-ctr|--aes-128-ctr|--xor]

# Revision History:

# ---------------------------------------------------------------------------

import argparse
import base64
import logging
import sys
from Crypto.Util.Padding import unpad
from Crypto.Cipher import AES
from Crypto.Util import Counter

iv_len = 16

def aes_ctr_decrypt(ciphertext, key, iv):
    iv_int = int.from_bytes(iv, 'big')
    counter = Counter.new(128, initial_value=iv_int)
    cipher = AES.new(key, AES.MODE_CTR, counter=counter)
    return cipher.decrypt(ciphertext)

def aes_cbc_decrypt(ciphertext, key, iv):
    cipher = AES.new(key, AES.MODE_CBC, iv=iv)
    return unpad(cipher.decrypt(ciphertext), AES.block_size, style='pkcs7')

def xor_decrypt(ciphertext, key, *_):
    plaintext = bytearray()
    key_len = len(key)
    for index, b in enumerate(ciphertext):
        plaintext.append(b ^ key[index % key_len])
    return plaintext

def decode_and_decrypt(decrypt_func, blob, key, iv_len):
    # Base64 decode
    iv_and_ciphertext = bytes(base64.b64decode(blob))

    # Parse the IV and ciphertext
    if iv_len:
        iv = iv_and_ciphertext[:iv_len]
        ciphertext = iv_and_ciphertext[iv_len:]
    else:
        # No IV
        iv = []
        ciphertext = iv_and_ciphertext

    # decrypt ciphertext
    return decrypt_func(ciphertext, key, iv)

def main():
    # Command line arguments for input and output file and encryption options
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input-file', required=True, help='Log file to decrypt')
    parser.add_argument('-o', '--output-file', required=True, help='File to write decrypted contents to')
    parser.add_argument('-k', '--key', required=True, help='decryption key in hex format')
    cipher_group = parser.add_mutually_exclusive_group(required=True)
    cipher_group.add_argument('--aes-256-cbc', action='store_true', help='Toggles AES-CBC decryption with a 256-bit key')
    cipher_group.add_argument('--aes-128-cbc', action='store_true', help='Toggles AES-CBC decryption with a 128-bit key')
    cipher_group.add_argument('--aes-256-ctr', action='store_true', help='Toggles AES-CTR decryption with a 256-bit key')
    cipher_group.add_argument('--aes-128-ctr', action='store_true', help='Toggles AES-CTR decryption with a 256-bit key')
    cipher_group.add_argument('--xor', action='store_true', help='Toggles XOR decryption with a variable-size key')
    parser.add_argument('-l', '--log', dest='logLevel', choices=['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'], help='Set logging level', default='INFO')
    args = parser.parse_args()

    logging.basicConfig(level=getattr(logging, args.logLevel))

    decrypt_func = None
    key_size = 32
    key = None
    iv_len = 0

    # get required key size and decryption function
    if args.aes_256_cbc:
        key_size = 32
        iv_len = 16
        decrypt_func = aes_cbc_decrypt
        logging.debug('Using AES-256-CBC decryption')
    elif args.aes_128_cbc:
        key_size = 16
        iv_len = 16
        decrypt_func = aes_cbc_decrypt
        logging.debug('Using AES-128-CBC decryption')
    elif args.aes_256_ctr:
        key_size = 32
        iv_len = 16
        decrypt_func = aes_ctr_decrypt
        logging.debug('Using AES-256-CTR decryption')
    elif args.aes_128_ctr:
        key_size = 16
        iv_len = 16
        decrypt_func = aes_ctr_decrypt
        logging.debug('Using AES-128-CTR decryption')
    elif args.xor:
        key_size = 0
        decrypt_func = xor_decrypt
        logging.debug('Using XOR decryption')
    else:
        logging.error('No decryption mode specified.')
        sys.exit(1)

    # verify key length and format
    try:
        key = bytearray.fromhex(args.key)
    except Exception as e:
        logging.error('Invalid format for key %s: %s', args.key, e)
        sys.exit(2)
    if key_size and len(key) != key_size:
        logging.error('Key size of %d required', key_size)
        sys.exit(3)
    logging.debug('Using decryption key (hex): %s', args.key)
    
    # decode and decrypt log file
    logging.info('Reading in log file from: %s', args.input_file)
    logging.info('Output path set to: %s', args.output_file)
    with open(args.input_file, 'rb') as infile:
        with open(args.output_file, 'wb') as outfile:
            for line in infile:
                try:
                    plaintext = decode_and_decrypt(decrypt_func, line, key, iv_len)
                    outfile.write(plaintext)
                    outfile.write('\n'.encode('utf-8'))
                except Exception as e:
                    logging.exception('Caught exception while decoding and decrypting log line: %s', e)

if __name__ == '__main__':
    main()
