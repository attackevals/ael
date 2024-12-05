# Protections Test 6

## Features

PowerShell script that use windows libraries to encrypt files.

Note: the encryption implementations have been removed for public release

### Quickstart

#### Usage & Arguments

Test4 takes two positional arguments `-action [encrypt/decrypt] -startFolder [C:\Folder\]`.

Example 1 (start in C:\Users and recursively encrypt:

```cmd
test4.ps1 -action encrypt -startFolder C:\Users\
```

#### Decrypting an encrypted file

Replace the filepath and key in the command below to decrypt any file that was encrypted
using the script.

```bash
test4.ps1 -action decrypt -startFolder C:\Users\ -key [decryption key]
```

## Troubleshoot

test4 does not generate any logging or stdout to reduce ioc's that are not relevant to the test.
Ensure the encryption path exists and the script has permissions to modify the file.
