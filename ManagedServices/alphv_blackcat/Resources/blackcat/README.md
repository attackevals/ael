# BlackCat

# Introduction
This ransomware emulates the BlackCat ransomware family, which is written in Rust and targets Windows and Linux<sup>[4](https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB),[2](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/),[5](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware)</sup>. Its behavior changes depending on whether it is compiled as a Windows binary or as a Linux binary.

To make troubleshooting and testing easier, the debug-compiled version of the emulated BlackCat comes with cleanup options that will decrypt any encrypted files.

:heavy_exclamation_mark: Note: the publicly released version of this code does not contain the actual encryption functionality, so any encryption/decryption simply rewrites the original plaintext. Users will need to incorporate the encryption themselves.

# Features
- As an anti-sandboxing measure, BlackCat requires a valid access token to be passed via the `--access-token` argument<sup>[3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf),[6](https://news.sophos.com/en-us/2022/07/14/blackcat-ransomware-attacks-not-merely-a-byproduct-of-bad-luck/)</sup>. For this emulated version, the provided access token must start with `15742aa362a84ba3`.
- Determines if running as an elevated or unelevated user.<sup>[2](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)</sup>
- Sets up threadpools for encryption (Linux/Windows) and for network scanning (Windows-only).<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>
- Deletes shadow copies before and after encrypting files (Windows only).<sup>[1](https://www.varonis.com/blog/blackcat-ransomware),[2](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/),[5](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware)</sup>
- Disables bootloader recovery by executing `bcdedit /set {default} recoveryenabled no`.<sup>[2](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)</sup>
- Discovers machine UUID by executing `wmic csproduct get UUID` (Windows) or by reading from the `/sys/class/dmi/id/product_uuid` (Linux, elevated) or `/etc/machine-id` (Linux, unelevated) files.<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>
- Discovers current username and hostname<sup>[2](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)</sup>
- Enables remote-to-local and remote-to-remote symbolic link evaluation using `fsutil` (Windows only).<sup>[1]( https://www.varonis.com/blog/blackcat-ransomware),[2](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/),[5](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware)</sup>
- Empties the recycle bin if not already empty (Windows only).<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>
- Performs automated propagation to remote hosts on the local network (Windows only)
  - Increases the max number of concurrent network requests by setting the`MaxMpxCt` registry value to `65535` for the registry key `HKLM\SYSTEM\CurrentControlSet\Services\LanmanServer\Parameters` to increase concurrent network requests<sup>[1](https://www.varonis.com/blog/blackcat-ransomware),[2](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)</sup>
  - Performs NetBIOS lookups on the local netrange to find remote Windows targets. Will propagate to these targets by writing an embedded PsExec binary to disk as `pmanager.exe` in the user's temp directory, and then using it to execute itself remotely using embedded credentials in the config<sup>[1](https://www.varonis.com/blog/blackcat-ransomware),[2](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/),[5](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware)</sup>
  - The PsExec command executed is equivalent to the following:
    ```
    PsExec64.exe -accepteula \\target1,target2,... -u <username> -p <password> -s -d -f -c <path to self> --access-token <blackcat access token> --no-prop
    ```
  - Note that for execution purposes, the BlackCat configuration includes a `strict_include_targets` setting, which is a list of IP addresses that the ransomware is allowed to propagate to. Change these according to your specific environment if you want to control the propagation. Providing a blank entry for this configuration value will allow all discovered target hosts.
  - To avoid endless propagation, any spawned BlackCat processes via PsExec will be run with the `--no-prop` flag, disabling any further propagation.
- Enumerate local partitions and mounts those that don't have volume paths, as they may be hidden partitions (Windows only). This will allow the encryption process to target these partitions, potentially including the recovery partition.<sup>[1](https://www.varonis.com/blog/blackcat-ransomware),[5](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware)</sup>
  - Partitions are mounted to randomly generated subdirectories of `C:\` so that they are included in the encryption routine.
- For Linux encryption, the ransomware will target VMs running on the target KVM server (KVM targeted instead of ESXi)<sup>[1](https://www.varonis.com/blog/blackcat-ransomware),[4](https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB),[6](https://news.sophos.com/en-us/2022/07/14/blackcat-ransomware-attacks-not-merely-a-byproduct-of-bad-luck/)</sup>:
  - Checks if KVM is installed on the target machine by running `virsh version`
  - Enumerates and shuts down all VMs on the KVM server
  - Enumerates and deletes VM snapshots
  - Enumerate VM volumes
  - Stops `libvirtd`, `virsh`, `libvirt-dbus` processes to prevent potential interference with encryption
  - Stops `libvirtd` service to prevent potential interference with encryption
  - Encrypts VM volumes. Encrypted files will have the custom ransomware extension appended to the filename, and affected directories will have the ransom note dropped.
- For Windows encryption, the ransomware will stop processes and services that may interfere with encryption and then iterate through the file system and encrypt valid target files.<sup>[1](https://www.varonis.com/blog/blackcat-ransomware),[3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf),[4](https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB),[5](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware),[6](https://news.sophos.com/en-us/2022/07/14/blackcat-ransomware-attacks-not-merely-a-byproduct-of-bad-luck/)[7](https://securityintelligence.com/posts/blackcat-ransomware-levels-up-stealth-speed-exfiltration/)</sup>
  - If the strict include directories configuration setting is set, BlackCat will only iterate through those directories. Otherwise, BlackCat will iterate through all valid directories, starting at the `C:\` root drive.
  - Any excluded directories, filenames, or file extensions specified in the configuration will be skipped. 
  - If a directory name is excluded, no child directories will be affected. For strict include directories, child directories will not be affected, either. 
  - Encrypted files will have the custom ransomware extension appended to the filename, and affected directories will have the ransom note dropped.
- Changes the wallpaper to a ransomware message image for all logged-in users (Windows only).<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>
  - The image is saved in each user's desktop folder (`C:\\Users\\<USERNAME>\\Desktop\\RECOVER-SKYFL2E-FILES.txt.png`), and registry updates are made to change the wallpaper accordingly<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>. Since the `HKEY_USERS` registry hive is used to enumerate the target users, only users with active logon sessions will be affected. The default wallpaper is not changed.
- Unmounts any partitions that were mounted earlier (Windows only)
- Clears Windows event logs (Windows only).<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>
- To hamper string-based detection in static analysis, string literals within the binary are encrypted at compile-time and are decrypted in memory at run-time.

## Ransomware Encryption
When encrypting files, BlackCat uses AES-CTR-128, using the same randomly generated key for all files. 

For small files (less than 128MB), the files are simply encrypted entirely.

For large files (over 128MB), a chunked encryption approach is used to avoid encrypting the entire file:
  - The file is broken up into chunks that are 1MB in size
  - For each 1MB chunk, the first 8KB are encrypted

The actual BlackCat ransomware has also used a chunking approach when encrypting large files.<sup>[3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)</sup>

:heavy_exclamation_mark: Note: the publicly released version of this code does not contain the actual encryption functionality, so any encryption/decryption simply rewrites the original plaintext. Users will need to incorporate the encryption themselves.

## Configuration
BlackCat's settings are configured using the [config.json](./config/config.json) JSON configuration file. The contents of this configuration file are AES-CTR-128 encrypted and then base64-encoded before being embedded in the executable during compilation. BlackCat uses the `--access-token` argument to decrypt the embedded configuration when executing<sup>[1](https://www.varonis.com/blog/blackcat-ransomware),[3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf),[4](https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB)</sup>

:heavy_exclamation_mark: Note: the publicly released version of this code only contains a base64-encoded configuration rather than an AES-encrypted configuration. Users will need to incorporate the encryption themselves.

Because the configuration settings are hardcoded in the source code, any adjustments will require recompilation, 
such as when enabling/disabling features, adjusting propagation credentials, or restricting where encryption and propagation occurs.
Note that if you're only changing the JSON file, `cargo build` may only reference the previously cached build since actual source code hasn't changed. In that case,
you can run `cargo clean` prior to re-running `cargo build` to force a fresh compilation.

The following settings are used:
- `kill_processes` - (Windows only) processes to kill prior to encrypting files<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>.
- `kill_services` - (Windows only) services to terminate prior to encrypting files<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>.
- `kill_processes_linux` - (Linux only) processes to kill prior to encrypting VM volumes and disrupting VM operations.
- `kill_services_linux` - (Linux only) services to terminate prior to encrypting VM volumes and disrupting VM operations.
- `exclude_directory_names` - (Windows only) directories to skip during encryption<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>.
- `exclude_file_names` - (Windows only) file names to skip during encryption<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>.
- `exclude_file_extensions` - (Windows only) file extensions to skip during encryption<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>.
- `strict_include_paths` -  (Windows only) only encrypt files in these directories<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>.
- `enable_set_wallpaper` - (Windows only) toggle changing the wallpaper after encrypting files<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>.
- `enable_network_discovery` - (Windows only) toggle remote target discovery via NetBIOS<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>.
- `enable_self_propagation` - (Windows only) toggle propagation via PsExec to discovered hosts (also requires `enable_network_discovery` to be enabled).<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>
- `enable_vm_kill` - (Linux only) toggles killing VMs on the target KVM server.<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>
- `enable_vm_snapshot_kill` - (Linux only) toggles deleting VM snapshots on the KVM server.
- `enable_enc` - toggles encryption of files (Windows) or VM disks (Linux).
- `enable_recovery_hampering` - (Windows only) toggles deleting shadow copies and disabling bootloader recovery.
    "enable_event_del": true,
    "enable_event_del": true,
- `enable_event_del` - (Windows only) toggles deleting Windows event logs.
- `enable_hidden_partitions` - (Windows only) toggles finding and mounting hidden partitions for encryption.
- `unmount_hidden_partitions` - (Windows only) toggles unmounting hidden partitions after encryption.
- `extension` - specifies the custom file extension to append to encrypted files<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>.
- `note_file_name` - specifies the ransom note file name<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>.
- `note_full_text` - specifies the ransom note contents<sup>[1](https://www.varonis.com/blog/blackcat-ransomware)</sup>.
- `empty_recycle_bin` - (Windows only) toggles emptying the recycle bin
- `psexec_username` - (Windows only) username to use for PsExec propagation.
- `psexec_password` - (Windows only) password to use for PsExec propagation.
- `strict_include_targets` - (Windows only) only propagate to these target hosts if discovered.
- `strict_include_scan_ranges` - (Windows only) only scan the intersection of these ranges with the local ranges. Used to avoid scanning the entirety of large networks

Below is an example JSON configuration file:
```json
{
    "kill_processes": ["firefox", "notepad", "msedge"],
    "kill_services": ["testsvc1", "testsvc2", "TestServ"],
    "kill_processes_linux": ["libvirtd", "virsh", "libvirt-dbus"],
    "kill_services_linux": ["libvirtd"],
    "exclude_directory_names": ["system volume information","intel","$windows.~ws","application data","$recycle.bin", "mozilla","program files (x86)","program files","$windows.~bt","public","msocache","windows","default","all users", "tor browser", "programdata","boot","config.msi","google","perflogs","appdata","windows.old","WindowsAzure"],
    "exclude_file_names": ["desktop.ini","autorun.inf","ntldr","bootsect.bak","thumbs.db","boot.ini","ntuser.dat", "iconcache.db","bootfont.bin","ntuser.ini","ntuser.dat.log"],
    "exclude_file_extensions": ["themepack","nls","diagpkg", "msi","lnk","exe","cab","scr","bat","drv","rtp","msp","prf","msc", "ico", "key","ocx","diagcab","diagcfg", "pdb","wpx","hlp","icns","rom","dll","msstyles","mod","ps1","ics","hta","bin","cmd","ani", "386","lock","cur","idx","sys","com","deskthemepack","shs","ldf","theme","mpa","nomedia", "spl","cpl","adv","icl","msu","xtlog"],
    "strict_include_paths": [],
    "enable_set_wallpaper": false,
    "enable_network_discovery": false,
    "enable_self_propagation": false,
    "enable_vm_kill": false,
    "enable_vm_snapshot_kill": false,
    "enable_enc": false,
    "enable_recovery_hampering": false,
    "enable_event_del": false,
    "enable_hidden_partitions": false,
    "unmount_hidden_partitions": false,
    "extension": ".skyfl2e",
    "note_file_name": "RECOVER-SKYFL2E-FILES.txt",
    "note_full_text": ">>Introduction\nImportant files on your machine were ENCRYPTED and now they have the \"SKYFL2E\" extension.\nIn order to recover your files, you need to follow the instructions below.\n\n>>CAUTION\nDO NOT MODIFY ENCRYPTED FILES YOURSELF.\nDO NOT USE THIRD PARTY SOFTWARE TO RESTORE YOUR DATA.\nYOU MAY DAMAGE YOUR FILES, RESULTING IN PERMANENT DATA LOSS.\nYOUR DATA IS STRONGLY ENCRYPTED, YOU CANNOT DECRYPT IT WITHOUT CIPHER KEY.\n\n>>What Should I do Next?\nFollow these simple steps to get everything back to normal:\n1) Download and install Tor browser from https://www.torproject.org/download/\n2) Navigate to: askfjejtqekjge0et1lkjasdq09gji13jgkdajv.testonion/?access-key=2646AEF615CD1126\n",
    "empty_recycle_bin": false,
    "psexec_username": "domain\\test_user",
    "psexec_password": "test_password",
    "strict_include_targets": ["10.20.20.102", "10.20.20.103"],
    "strict_include_scan_ranges": ["10.20.10.0/24", "10.20.20.0/24"]
}
```

## Troubleshooting and Logging
Logging is provided for troubleshooting purposes and operator usability. Logs are written to `clog.xtlog` in the current working directory on Windows systems, and to `bc.log` in the current working directory on Linux systems. 

:heavy_exclamation_mark: Note: the publicly released version of this code only logs in plaintext and will also output log lines to the console if run from there.

BlackCat will log each action taken and will provide success or error messages. The verbosity may vary depending on whether or not it was compiled for debug or release, but operators can search for any `ERROR` messages to look for things that went wrong.

Below are some potential issues and how to address them:
- BlackCat immediately terminates, no log file created
  - If the vendor didn't block the process, chances are the incorrect access token was provided, or none was provided at all via command-line. Check your command and try again.
- Several actions skipped due to lack of privileges
  -  Ensure you are running BlackCat under an elevated context and try again.
-  Privileges are correct, but BlackCat still didn't perform certain actions
  - Ensure the JSON configuration file in `config/config.json` has everything enabled that you want performed
- BlackCat failed to propagate
  - Check the JSON configuration to make sure `enable_network_discovery` and `enable_self_propagation` are set to `true` and that the correct credentials are provided
    in `psexec_username` and `psexec_password`
  - If `strict_include_targets` is set in the JSON configuration, make sure the values are what you expect. If a non-empty list is provided, BlackCat will only propagate to these hosts.
  - Check the logs to make sure the correct local network CIDR was used
  - If the correct network was scanned, make sure the starting host can talk to other hosts on the network via NetBIOS, as this connection is required in order for the propagation target scan to succeed.
  - Check the logs to make sure PsExec was dropped and executed properly

## Cleanup
When compiled in debug mode, BlackCat comes with cleanup capabilities, which are enabled by using the `--clean-up` and `--key` arguments.
Since BlackCat will generate a random AES key each time it is executed, you will have to go through the BlackCat logs on the affected hosts to
determine the AES keys to use for cleanup. Pass the AES key in as a hex string for BlackCat to decrypt affected files.

The following cleanup tasks are performed:
- Decrypt encrypted files
- Remove ransom notes from affected directories

Note that registry changes, killed processes/services, wallpaper changes, deleted events, files deleted from the recycle bin, etc will not be restored.

# Run
BlackCat can be run via `cargo` or as a precompiled executable (see the [Build section](#build) for build information). 

Note that `--access-token` is required and must be at least 16 characters long. The first 16 characters are used to decrypt the configuration<sup>[3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)</sup>.

:heavy_exclamation_mark: WARNING - do not run BlackCat on any system that you do not want to tamper with. BlackCat will perform various recovery hampering and impact TTPs, including encrypting files, deleting shadow copies, and clearing windows event logs. Use and test with caution.

Via `cargo`:
```
# Debug
cargo run -- --access-token 15742aa362a84ba3

# Release
cargo run --release -- --access-token 15742aa362a84ba3
```

As an executable:
```
blackcat.exe --access-token 15742aa362a84ba3
```

The following command-line flags can override config settings or enable other features:
- `--prop` - force propagation via PsExec.
- `--no-prop` - force disable propagation via PsExec.
- `--clean-up` - toggle cleanup mode. Requires the `--key` argument
- `--key` - hex string containing the AES key used to encrypt files. Used with the `--clean-up` flag to decrypt encrypted files.

Examples:
```
# No propagation, even if specified in internal config
blackcat.exe --access-token 15742aa362a84ba3 --no-prop

# Cleanup mode (debug compilation only)
blackcat.exe --access-token 15742aa362a84ba3 --clean-up --key [AES key hex string]
```

# Build
When compiled, binaries are automatically placed in `./target/[release|debug]/` for Windows and in `./target/x86_64-unknown-linux-musl/release` for Linux (when using the `x86_64-unknown-linux-musl` target). Pre-compiled binaries are already available in `./executables`.

When building in Linux for the first time, you will have to install the appropriate target:
```
rustup target add x86_64-unknown-linux-musl
```

For Debug builds:
```
# Windows
cargo build

# Linux
cargo build --target x86_64-unknown-linux-musl
```

For Release builds:
```
# Windows
cargo build --release

# Linux
cargo build --target x86_64-unknown-linux-musl --release
```

For best results, run the `cargo build` commands on a Linux machine when compiling a Linux version of BlackCat, and on a Windows machine when compiling a Windows version. Cross-compilation has not been fully tested.

## Required Software
- Rust v1.70.0 or greater - Installed via `rustup`. Available from [the official Rust website](https://www.rust-lang.org/tools/install).
  - Windows installer Notes
    - Chocolatey (`choco install rustup.install`), or Winget (`winget install Rustlang.Rustup`) are also viable installers
    - Rustup installer should prompt you to install Visual C++ Build Tools but if not, you will need to install that too

## Windows
- [Visual C++ Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/) should be prompted to install when running `rustup`. If not, install independently. 
- To avoid vcruntime issues, the `crt-static` flag is passed to the compiler. This is already set in `.cargo/config.toml`

# Testing

## Detonating In Test Environment
When detonating the BlackCat ransomware in a test environment, the following considerations need to be taken into account:
- Do I want encryption to happen?
- Which machines do I want to propagate to? Just one or two for testing? Or every machine possible in the environment?

Perform the following checklist prior to detonating in a test environment:
- Ensure any anti-virus products (e.g. Defender) are OFF in every machine in the test environment, even the ones you aren't planning to propagate to, just in case
the propagation restriction feature has an unknown bug.
- If planning on terminating specific processes or services, ensure they are running on the target machines
- If clearing the recycling bin, ensure there are files in the recycling bin on the target machines
- If deleting shadow copies, ensure there are shadow copies on the target machines
- Ensure there are VMs on the Linux KVM server
  - If stopping VMs, ensure the VMs are running
  - If deleting VM snapshots, ensure that the VMs have snapshots to delete
- If enabling encryption, ensure there are valid files to encrypt
  - If using `strict_include_paths`, ensure those paths have valid files to encrypt
- Ensure the BlackCat configuration is set correctly according to your environment and what you plan on testing
  - Correct credentials for PsExec propagation
  - Correct IP addresses for `strict_include_targets`, if applicable
  - Correct paths for `strict_include_paths`, if applicable
  - Features enabled/disabled as desired
- Ensure there are snapshots for the target machines that include all of the above, so you can revert after detonating BlackCat

Once everything is set, you will have to rebuild BlackCat and transfer it to your target machines
- If building to target the KVM server, you will need to build on a Linux machine, such as your Kali attacker machine
- If building to target Windows, you will need to build on a Windows dev machine and then transfer to the target environment, e.g. via GitHub dev branch

Detonate and inspect logs to see what succeeded or failed.

## Unit Tests
Unit tests are currently written within the same `.rs` files as the code being tested. 
Note that the Windows and Linux variants have different unit tests.

To run Debug tests:
```
# Windows
cargo test

# Linux
cargo test --target x86_64-unknown-linux-musl
```

To run Release tests:
```
# Windows
cargo test --release

# Linux
cargo test --target x86_64-unknown-linux-musl --release
```

# Minimal Blackcat
To support alternative steps, a minimal version of BlackCat was created that only performs encryption for Linux/Windows. 

This source code was removed prior to public release.

# Developer Notes

## File and Directory Structure
| File/Directory | Description |
| ---- | ----------- |
| `config/` | provides JSON configuration files for BlackCat |
| `config/config.json` | Production configuration file for BlackCat. Edit this to change actual BlackCat behavior. |
| `config/unit_test_config.json` | Configuration file specifically for unit tests. Do not edit unless adjusting configuration logic in source code or fixing unit tests. |
| `encryptliterals/` | provides custom macros to read in and AES-encrypt the JSON configuration and to XOR-encrypt string literals within source code |
| `executables/` | provides pre-compiled debug and release binaries |
| `resources/` | contains resources and other helper files |
| `resources/zipper_util/` | contains logic to zlib-compress the PsExec64.exe binary |
| `resources/decrypt_log.py` | Python script that will decrypt the BlackCat log file |
| `resources/PsExec64.exe` | PsExec v2.43 binary downloaded from Microsoft |
| `resources/psexec64.zip` | zipped version of the PsExec v2.43 binary. Will be embedded in BlackCat binary on compilation |
| `resources/ransom_note.png` | image used to replace user wallpapers after encrypting files |
| `src/latmove/nbtscan/` | contains a modified version of the [nbtscanner Rust crate](https://github.com/jonkgrimes/nbtscanner) | 
| `src/latmove/local_ips.rs` | contains logic to find local IPs and netranges |
| `src/latmove/propagate.rs` | contains logic to discover remote targets and propagate to them via PsExec |
| `src/cleanup.rs` | contains main cleanup logic for BlackCat's cleanup variant |
| `src/config.rs` | contains BlackCat configuration logic and configuration-related auxiliary methods |
| `src/discovery.rs` | contains basic discovery logic for current username and privilege level, current hostname, and machine UUID |
| `src/encrypt.rs` | provides AES encryption/decryption routines |
| `src/enumerate.rs` | provides logic to enumerate files and encrypt them on Windows targets |
| `src/eventdel.rs` | provides logic to clear Windows event logs |
| `src/kvm.rs` | provides logic to perform ransomware tasks against Linux KVM servers (stop VMs, delete snapshots, encrypt VM volumes) |
| `src/logger.rs` | provides logging functionality |
| `src/main.rs` | contains the main high-level logic and argument parsing for BlackCat |
| `src/proc.rs` | provides functionality to execute and terminate processes |
| `src/psexec.rs` | provides functionality to unzip the embedded PsExec binary, execute PsExec, and remove PsExec from disk |
| `src/recovery.rs` | provides functionality to disable bootloader recovery and delete shadow copies | 
| `src/recycling.rs` | provides functionality to empty the recycling bin on Windows targets |
| `src/service.rs` | provides funcionality to terminate services |
| `src/wallpaper.rs` | provides functionality to change the wallpaper on Windows targets | 
| `src/winapi_helper.rs` | provides helper methods for using the `winapi` crate |
| `src/windowscrate_helper.rs` | provides helper methods for using the `windows` crate |

## Documentation
- Generate and open build docs by running `cargo doc --open`
- Follow code documentation instructions [here](https://doc.rust-lang.org/rust-by-example/meta/doc.html) in order to add your own comments to the docs

## Rust Help
- Learning Rust? [Start here](https://www.rust-lang.org/learn). The official Rust book is an excellent resource and should be your first step.
- [rust-analyzer](https://rust-analyzer.github.io/) - A Language Server Protocol that provides useful features like auto-completion and type hints. 
  - Packaged as a native [VSCode Plugin](https://marketplace.visualstudio.com/items?itemName=rust-lang.rust-analyzer) (recommended)
  - Refer to the website for other supported editors and IDE's.

# CTI References
1. https://www.varonis.com/blog/blackcat-ransomware
2. https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/ 
3. https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf
4. https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB
5. https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware
6. https://news.sophos.com/en-us/2022/07/14/blackcat-ransomware-attacks-not-merely-a-byproduct-of-bad-luck/
7. https://securityintelligence.com/posts/blackcat-ransomware-levels-up-stealth-speed-exfiltration/
