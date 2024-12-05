# ---------------------------------------------------------------------------
# test_aes_base64_log_decryptor.py - Tests the aes_base64_log_decryptor.py
# file

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

# Usage: python3 test_aes_base64_log_decryptor.py [-l|--log DEBUG|INFO|WARNING|ERROR|CRITICAL]

import argparse
import logging
import os
import random
import string
import subprocess

DECRYPTOR_PATH = os.path.abspath(os.path.dirname(__file__)) + '/aes_base64_log_decryptor.py'

class TestBundle:
    # Pass in key as hex string
    def __init__(self, name, key, mode, encrypted_lines=[], expected_lines=[], expected_exit_code=0):
        self.name = name
        self.key = key
        self.mode = mode
        self.encrypted_lines = encrypted_lines
        self.expected_lines = expected_lines
        self.log_file = self.get_rand_log_name('.testlog')
        self.decrypted_file = self.get_rand_log_name('.testlog')
        self.failure_reason = ''
        self.expected_exit_code = expected_exit_code

    def run_test_suite(self):
        if self.encrypted_lines and self.expected_lines:
            self.write_test_log_file()
        exit_code = self.run_decryptor()
        if exit_code != self.expected_exit_code:
            self.failure_reason = 'Expected exit code %d: got %d' % (self.expected_exit_code, exit_code)
            return False
        elif self.expected_exit_code != 0:
            # Test was expecting failure - return success
            return True
        return self.compare_logs()

    def write_test_log_file(self):
        with open(self.log_file, 'w') as fd:
            for line in self.encrypted_lines:
                fd.write('%s\n' % line)
        logging.debug('Wrote encrypted test log file to %s', self.log_file)

    def delete_test_files(self):
        if self.expected_exit_code == 0:
            os.remove(self.log_file)
            logging.debug('Removed encrypted test log file %s', self.log_file)
            os.remove(self.decrypted_file)
            logging.debug('Removed decrypted test log file %s', self.decrypted_file)

    def run_decryptor(self):
        args = ['python3', DECRYPTOR_PATH, '-i', self.log_file, '-o', self.decrypted_file, '-k', self.key, '--' + self.mode]
        logging.debug('Executing: %s', ' '.join(args))
        proc = subprocess.run(args, check=(self.expected_exit_code == 0), timeout=10, capture_output=True)
        logging.debug('Decryptor script exit code: %d', proc.returncode)
        logging.debug('Decryptor script stdout: %s', proc.stdout)
        logging.debug('Decryptor script stderr: %s', proc.stderr)
        return proc.returncode

    def compare_logs(self):
        with open(self.decrypted_file, 'r') as fd:
            result = fd.read().splitlines()
            if result == self.expected_lines:
                return True
            else:
                self.failure_reason = 'Expected: %s\nGot: %s\n' % (
                    '\n'.join(self.expected_lines),
                    '\n'.join(result),
                )
                return False

    @staticmethod
    def get_rand_log_name(suffix):
        return ''.join(random.choices(string.ascii_letters + string.digits, k=12)) + suffix


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-l', '--log', dest='logLevel', choices=['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'], help='Set logging level', default='INFO')
    args = parser.parse_args()
    logging.basicConfig(level=getattr(logging, args.logLevel))

    # Expected ciphertexts generated using cyberchef: https://gchq.github.io/CyberChef/
    test_suite = [
        TestBundle(
            name='test_aes_128_cbc',
            key='00000000000000000000000000000000',
            mode='aes-128-cbc',
            encrypted_lines=[
                # 0000000000000000000000000000000096f9baec1597665ae346b541b8cc72d6e8b7eda655d53e6502462c359733b955
                'AAAAAAAAAAAAAAAAAAAAAJb5uuwVl2Za40a1QbjMctbot+2mVdU+ZQJGLDWXM7lV',

                # 11111111111111111111111111111111060bc9e1d11b12ae69e4492fed32383ff5737b493d44db055d9a277702cae023
                'EREREREREREREREREREREQYLyeHRGxKuaeRJL+0yOD/1c3tJPUTbBV2aJ3cCyuAj',

                # 2222222222222222222222222222222249c50a6f9444288720d64ecac7018ac5cb43ce90acac994b0b062ca51ec4ecec
                'IiIiIiIiIiIiIiIiIiIiIknFCm+URCiHINZOyscBisXLQ86QrKyZSwsGLKUexOzs',
            ],
            expected_lines=[
                'test log line input',
                'test log line input2',
                'test log line input3',
            ],
        ),
        TestBundle(
            name='test_aes_256_cbc',
            key='1111111111111111111111111111111111111111111111111111111111111111',
            mode='aes-256-cbc',
            encrypted_lines=[
                # 000000000000000000000000000000002c99ee6a905349ee37eb39207c03ef15ff8d924995d7fdefce2f2488b9b3b44d
                'AAAAAAAAAAAAAAAAAAAAACyZ7mqQU0nuN+s5IHwD7xX/jZJJldf9784vJIi5s7RN',

                # 111111111111111111111111111111116bec678164ce644779c6052fd11f3de80605bdebaa7734da9ca143dc84d7ed10
                'EREREREREREREREREREREWvsZ4FkzmRHecYFL9EfPegGBb3rqnc02pyhQ9yE1+0Q',

                # 2222222222222222222222222222222250f695c6cb021a5a31bac7c457dfa46ac0ef4162a943e6788d4766fd27c91b99
                'IiIiIiIiIiIiIiIiIiIiIlD2lcbLAhpaMbrHxFffpGrA70FiqUPmeI1HZv0nyRuZ',
            ],
            expected_lines=[
                'test log line input',
                'test log line input2',
                'test log line input3',
            ],
        ),
        TestBundle(
            name='test_aes_128_ctr',
            key='22222222222222222222222222222222',
            mode='aes-128-ctr',
            encrypted_lines=[
                # 0000000000000000000000000000000041c362b255c70f2de344b51b7001ebae1727be
                'AAAAAAAAAAAAAAAAAAAAAEHDYrJVxw8t40S1G3AB664XJ74=',

                # 111111111111111111111111111111116e4579f8361c8eba6375e707cea36121c905acc5
                'EREREREREREREREREREREW5Fefg2HI66Y3XnB86jYSHJBazF',

                # 2222222222222222222222222222222232e2a694925482d7276655be615e727d020afc09
                'IiIiIiIiIiIiIiIiIiIiIjLippSSVILXJ2ZVvmFecn0CCvwJ',
            ],
            expected_lines=[
                'test log line input',
                'test log line input2',
                'test log line input3',
            ],
        ),
        TestBundle(
            name='test_aes_256_ctr',
            key='2222222222222222222222222222222222222222222222222222222222222222',
            mode='aes-256-ctr',
            encrypted_lines=[
                # 000000000000000000000000000000004d0c90ecb9e63087188fc980a6855689aa1ae4
                'AAAAAAAAAAAAAAAAAAAAAE0MkOy55jCHGI/JgKaFVomqGuQ=',

                # 111111111111111111111111111111114ad4d1459a4ffe41e5070ae339b207e459b71962
                'EREREREREREREREREREREUrU0UWaT/5B5QcK4zmyB+RZtxli',

                # 222222222222222222222222222222224e49ad6f47d895e12238afa2dc7bb57cc5cfc4f6
                'IiIiIiIiIiIiIiIiIiIiIk5JrW9H2JXhIjivotx7tXzFz8T2',
            ],
            expected_lines=[
                'test log line input',
                'test log line input2',
                'test log line input3',
            ],
        ),
        TestBundle(
            name='test_xor',
            key='4ad4d1459a4ffe41e5070ae339b207e459b71962',
            mode='xor',
            encrypted_lines=[
                'PrGiMbojkSbFa2ONXJJuiinCbQ==',
                'K7q+MfIqjGGRYnmXGd5og3nbcAwv',
                'BrujIPdvlzGWcmfDXd1riyuXags+9LAo/zvSYYZoZJBc0XOBLcJrQiuwuDXzPJ0oi2AqhlXbc8h5xHwGarC+Zf8mizKIaG7DTddqlDbFOQskt7gh8yuLL5Enf5cZ3maGNsV8Qi+g8SH1I5EzgCdngl7cZsQ423ATP7X/Zc873iSLbmfDWNYniTDZcA9qorQr8y6TbcV2f4pKkmmLKsNrFy70tD3/PZ0okWZ+ilbcJ5E123gPKbvxKfstkTOMdCqNUMFuxCzDOQMmvaAw8z/eJJ0nb4IZ0WiJNNh9DWq3vivpKo80hHMkw33Hbpd51mwWL/S4N+89m2GBaGaMS5JuinnFfBI4sbkg9CubM4xzKopXknGLNcJpFiugtGXsKpIokSdvkErXJ4cw23UXJ/S1KvYgjCTFYn/DX8dgjTjDOQw/uL0kuj+fM4xmfpZLnCehIdR8Ej6xpDe6PJcvkSdlgFrTYoc4wzkBP6S4Ifs7nzXFaWWNGcJ1izDTfAw++PE27yGKYYxpKoBM3neFecZsC2q7tyPzLJcgxWNvkFzAcootl3QNJri4MboukCiIJ2OHGdd0kHnbeAAlpqQotA==',
            ],
            expected_lines=[
                'test log line input',
                'another test log line',
                'Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.',
            ],
        ),
        TestBundle(
            name='test_invalid_key_format',
            key='nothexstring',
            mode='aes-128-ctr',
            expected_exit_code=2,
        ),
        TestBundle(
            name='test_wrong_key_length',
            key='010203040506070809101112131415',
            mode='aes-128-cbc',
            expected_exit_code=3,
        ),
    ]

    exceptions = 0
    failed = 0
    passed = 0

    for test_bundle in test_suite:
        logging.info('Performing test suite: %s', test_bundle.mode)
        try:
            if test_bundle.run_test_suite():
                logging.info('%s PASSED', test_bundle.mode)
                passed += 1
            else:
                logging.error('%s FAILED: %s', test_bundle.mode, test_bundle.failure_reason)
                failed += 1
        except Exception as e:
            logging.error('Test suite %s failed: %s', test_bundle.mode, e)
            exceptions += 1
        finally:
            test_bundle.delete_test_files()

    # Test proper command line arg verification
    

    logging.info('%d TOTAL TESTS: %d PASSED, %d FAILED, %d EXCEPTIONS', len(test_suite), passed, failed, exceptions)

if __name__ == '__main__':
    main()
