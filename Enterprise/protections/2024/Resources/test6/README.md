# Protections Test 6

## Features

Bash script that utilizes OpenSSL to encrypt files

Note: the encryption implementations have been removed for public release

### Quickstart

#### Usage & Arguments

Test6 takes one positional argument `[Path to starting directory/]`.

Example 1 (start in /home/ and recursively encrypt:

```cmd
test6.sh /home/
```

## Troubleshoot

test6 does not generate any logging or stdout to reduce ioc's that are not relevant to the test.
Ensure the encryption path exists and the script has permissions to modify the file.
