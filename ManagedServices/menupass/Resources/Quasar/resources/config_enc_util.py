import argparse
import base64
import hashlib
import hmac
import logging
from Crypto import Random
from Crypto.Util.Padding import pad
from Crypto.Cipher import AES
from Crypto.Protocol.KDF import PBKDF2
from Crypto.Hash import SHA256

salt = bytes.fromhex('b0a0f82bbff1d2b5dab062a0bfe4312e3d9efaf34cd929292bc2a93d323b09ef')

def aes_encrypt_and_hmac(key_derivation_password, input):
    # derive keys
    derived_bytes = PBKDF2(key_derivation_password, salt, 96, count=50000, hmac_hash_module=SHA256)
    aes_key = derived_bytes[0:32]
    hmac_key = derived_bytes[32:]
    logging.debug('AES key: %s', aes_key.hex())
    logging.debug('HMAC key: %s', hmac_key.hex())

    # Generate random IV
    iv = Random.new().read(AES.block_size)

    # Pad and encrypt
    cipher = AES.new(aes_key, AES.MODE_CBC, iv=iv)
    ciphertext = cipher.encrypt(pad(input, AES.block_size))

    # HMAC of IV + ciphertext
    ciphertext_blob = iv + ciphertext
    hmac_hash = hmac.new(hmac_key, ciphertext_blob, hashlib.sha256).digest()
    
    return hmac_hash + ciphertext_blob

if __name__ == '__main__':
    parser = argparse.ArgumentParser('Quasar Setting Value Encryptor')
    parser.add_argument('-p', '--password', required=True, help='Password to derive AES and HMAC keys for encryption')
    parser.add_argument('-i', '--input', required=True, help='Input string to encrypt and encode')
    parser.add_argument('-l', '--log', dest='logLevel', choices=['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'], help='Set logging level', default='INFO')
    args = parser.parse_args()

    logging.basicConfig(level=getattr(logging, args.logLevel))

    print(base64.b64encode(aes_encrypt_and_hmac(str.encode(args.password), str.encode(args.input))).decode('utf-8'))
