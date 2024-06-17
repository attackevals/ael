# ---------------------------------------------------------------------------
# decryptquasarlogs.py - Decrypts the provided Quasar implant log and saves to the specified output file.
# Requires PyCryptodome: pip install pycryptodome

# Copyright 2023 MITRE Engenuity. Approved for public release. Document number TBD.
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

# This project makes use of ATT&CK®
# ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 

# Usage: python3 decryptquasarlogs.py -i /path/to/encrypted/log -o /path/to/save/output

# Revision History:

# ---------------------------------------------------------------------------

import argparse
import base64
import logging
from Crypto.Util.Padding import unpad
from Crypto.Cipher import AES

key = b'\xcc\x4a\xcd\xe9\x28\x6d\x68\xe6\xa5\x40\xc9\x35\x76\xd2\x16\x91\x06\xd8\xaf\xfd\xa5\x12\x3f\x3d\xfb\xdf\x9e\xb2\xfd\x32\x78\xe6'
iv_len = 16

def aes_decrypt(decoded):
    # First 16 bytes is IV
    iv = decoded[:iv_len]
    ciphertext = decoded[iv_len:]
    cipher = AES.new(key, AES.MODE_CBC, iv=iv)
    return unpad(cipher.decrypt(ciphertext), AES.block_size, style='pkcs7')

if __name__ == '__main__':
    parser = argparse.ArgumentParser('Quasar Implant Log Decryptor')
    parser.add_argument('-i', '--input', required=True, help='Provide path to the input log file to decrypt.')
    parser.add_argument('-o', '--output', required=True, help='Provide output path for decrypted log file.')
    parser.add_argument('-l', '--log', dest='logLevel', choices=['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'], help='Set logging level', default='INFO')
    args = parser.parse_args()

    logging.basicConfig(level=getattr(logging, args.logLevel))
    logging.info('Reading in log file from %s', args.input)
    logging.info('Output path set to %s', args.output)

    with open(args.output, 'wb') as output_file:
        with open(args.input) as file:
            for line in file:
                decoded = base64.b64decode(line)
                if len(decoded) > iv_len:
                    decrypted_line = aes_decrypt(decoded)
                    output_file.write(decrypted_line + b'\n')
