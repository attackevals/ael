# Cl0p

Cl0p is a ransomware tool used to find and encrypt files of interest.

## Features

- XOR-encrypts important strings at compile-time using a hardcoded XOR key and decrypts them at run time to hinder
  static analysis.<sup>[5](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)</sup>
  - Strings are encrypted using the following XOR hex key:

  ```text
  59874251a9b8b07f679a467e559bf8852216da44ce78f463d6bf037c15826ba098c215439d3abe34688a28ce538e1c8d6fc719d97d0ef73ffb893c79cf8bd850
  ```

- Checks victim's language<sup>[5](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)</sup>
  - Only continues execution if keyboard layout does not belong to Russia or the CIS countries Georgia or Azerbijan.
  - Also checks character set via the `GetTextCharset` API call to make sure it doesn't have the value 0xCC (RUSSIAN_CHARSET)
  - If using one of these keyboard layouts or character sets, will delete itself from disk and terminate itself using `TerminateProcess`
- Creates file `Favbug.txt` in the same directory as the malware<sup>[5](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)</sup>
  - If file was successfully created (`GetLastError` returns 0), sleeps for 5 seconds
- Performs dummy call loop 678678 times<sup>[5](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)</sup>
  - Contains a dummy call to `EraseTape` with a handle of 0
  - Contains a dummy call to `DefineDosDeviceA` with an invalid name that returns another error
- Deletes shadow copies and resizes shadow storage to avoid the shadow volumes being recreated<sup>[3](https://sectrio.com/deconstructing-cl0p-ransomware-moveit-2023-breach/)[5](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)</sup>
  - `vssadmin Delete Shadows /all /quiet`
  - `vssadmin resize shadowstorage /for=c: /on=c: /maxsize=401MB`
  - `vssadmin resize shadowstorage /for=c: /on=c: /maxsize=unbounded`
  - `vssadmin resize shadowstorage /for=d: /on=d: /maxsize=401MB`
  - `vssadmin resize shadowstorage /for=d: /on=d: /maxsize=unbounded`
  - `vssadmin resize shadowstorage /for=e: /on=e: /maxsize=401MB`
  - `vssadmin resize shadowstorage /for=e: /on=e: /maxsize=unbounded`
  - `vssadmin resize shadowstorage /for=f: /on=f: /maxsize=401MB`
  - `vssadmin resize shadowstorage /for=f: /on=f: /maxsize=unbounded`
  - `vssadmin resize shadowstorage /for=g: /on=g: /maxsize=401MB`
  - `vssadmin resize shadowstorage /for=g: /on=g: /maxsize=unbounded`
  - `vssadmin resize shadowstorage /for=h: /on=h: /maxsize=401MB`
  - `vssadmin resize shadowstorage /for=h: /on=h: /maxsize=unbounded`
  - `vssadmin Delete Shadows /all /quiet`
- Uses `bcedit` to disable boot recovery options and set to ignore any failure in the boot warning the user<sup>[5](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)</sup>
  - `bcdedit /set {default} recoveryenabled No`
  - `bcdedit /set {default} bootstatuspolicy IgnoreAllFailures`
- Creates a mutex to check if already running<sup>[2](https://www.cybereason.com/blog/research/cybereason-vs.-clop-ransomware)[5](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)</sup>
  - Uses the mutex name `Best-Fan-666`
  - Checks if the mutex already exists via `WaitForSingleObject` API call
    - If so, terminates early and self-deletes
- Attempts to kill certain services related to backups or security products via the `net stop` command<sup>[1](https://www.securin.io/articles/all-about-clop-ransomware/),[2](https://www.cybereason.com/blog/research/cybereason-vs.-clop-ransomware),[3](https://sectrio.com/deconstructing-cl0p-ransomware-moveit-2023-breach/),[4](https://unit42.paloaltonetworks.com/clop-ransomware/)</sup>
  - `SQLAgent$SYSTEM_BGC`
  - `SQLAgent$ECWDB2`
  - `"Zoolz 2 Service"`
  - `EraserSvc11710`
  - `SQLAgent$PROFXENGAGEMENT`
  - `MSSQLFDLauncher$TPSAMA`
  - `SstpSvc`
  - `MSOLAP$SQL_2008`
  - `sacsvr`
  - `VeeamNFSSvc`
  - `VeeamTransportSvc`
  - `MSSQLSERVER`
  - `MSSQLServerADHelper100`
  - `wbengine`
  - `SQLWriter`
  - `MSSQLFDLauncher$TPS`
  - `ReportServer$TPSAMA`
  - `MSSQL$SYSTEM_BGC`
  - `VeeamBrokerSvc`
  - `MSSQLFDLauncher$PROFXENGAGEMENT`
  - `VeeamDeploymentService`
  - `SQLAgent$TPS`
  - `DCAgent`
  - `MSSQLFDLauncher$SBSMONITORING`
  - `MySQL80`
  - `MSOLAP$SYSTEM_BGC`
  - `ReportServer$TPS`
  - `MSSQL$ECWDB2`
  - `SQLSERVERAGENT`
  - `SMTPSvc`
  - `MSSQL$VEEAMSQL2008R2`
  - `MSExchangeSA`
  - `MSSQLServerADHelper`
  - `SQLTELEMETRY`
  - `POP3Svc`
  - `MSOLAP$TPSAMA`
  - `"Veeam Backup Catalog Data Service"`
  - `MSSQL$SBSMONITORING`
  - `ReportServer$SYSTEM_BGC`
  - `MySQL57`
  - `VeeamMountSvc`
  - `MsDtsServer110`
  - `SQLAgent$BKUPEXEC`
  - `UI0Detect`
  - `ReportServer`
  - `SQLTELEMETRY$ECWDB2`
  - `MSSQLFDLauncher$SYSTEM_BGC`
  - `MSSQL$BKUPEXEC`
  - `SQLAgent$PRACTTICEBGC`
  - `MSExchangeSRS`
  - `SQLAgent$VEEAMSQL2008R2`
  - `VeeamCatalogSvc`
  - `SQLAgent$SHAREPOINT`
  - `NetMsmqActivator`
  - `MsDtsServer`
  - `SQLAgent$SQL_2008`
  - `SDRSVC`
  - `IISAdmin`
  - `SQLAgent$PRACTTICEMGT`
  - `SQLAgent$VEEAMSQL2008R2`
  - `VeeamHvIntegrationSvc`
  - `W3Svc`
  - `"SQLsafe Backup Service"`
  - `SQLAgent$CXDB`
  - `SQLBrowser`
  - `MSSQLFDLauncher$SQL_2008`
  - `VeeamBackupSvc`
  - `SQLAgent$VEEAMSQL2012`
  - `MSExchangeMGMT`
  - `SamSs`
  - `MSExchangeES`
  - `MSSQL$TPSAMA`
  - `VeeamCloudSvc`
  - `MSSQL$PRACTICEMGT`
  - `SQLAgent$TPSAMA`
  - `SQLAgent$SBSMONITORING`
  - `MSSQL$VEEAMSQL2012`
  - `SQLSafeOLRService`
  - `VeeamEnterpriseManagerSvc`
  - `SQLAgent$SQLEXPRESS`
  - `OracleClientCache80`
  - `MSSQL$PROFXENGAGEMENT`
  - `IMAP4Svc`
  - `MSExchangeIS`
  - `MSSQLFDLauncher`
  - `MSExchangeMTA`
  - `"SQL Backups"`
  - `MSSQL$TPS`
  - `MsDtsServer100`
  - `MSSQL$SHAREPOINT`
  - `msftesql$PROD`
  - `mozyprobackup`
  - `MSSQL$SQL_2008`
  - `SNAC`
  - `ReportServer$SQL_2008`
  - `MSSQL$SQLEXPRESS`
  - `MSSQL$PRACTTICEBGC`
  - `VeeamRESTSvc`
  - `MMS`
  - `RESvc`
  - `MSSQL$VEEAMSQL2008R2`
  - `MSSQLFDLauncher$SHAREPOINT`
  - `"SQLsafe Filter Service"`
  - `MSSQL$PROD`
  - `SQLAgent$PROD`
  - `MSOLAP$TPS`
  - `VeeamDeploySvc`
  - `MSSQLServerOLAPService`
- Uses `SetErrorMode`, setting it to 1 to avoid error dialog being shown to user in case of crash<sup>[5](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)</sup>
- Creates 2 threads - one to encrypt files, another to search for processes to kill<sup>[5](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)</sup>
  - One thread enumerates all processes of the system, creates the name of each process in upper case, calculates a hash with that name via `std::hash`, and
    compares it against a big list of hashes. If it finds one of them it will terminate it with the `TerminateProcess` function after opening with the rights
    to make this action with `OpenProcess` function. This thread loops infinitely, killing the set of processes every minute.
    - `MSFTESQL.EXE`
    - `SQLAGENT.EXE`
    - `SQLBROSWER.EXE`
    - `SQLWRITER.EXE`
    - `OCSSD.EXE`
    - `DBSNMP.EXE`
    - `SYNCTIME.EXE`
    - `AGNTSVC.EXEISQLPLUSSVC.EXE`
    - `XFSSVCCON.EXE`
    - `SQLSERVR.EXE`
    - `MYDESKTOPSERVICE.EXE`
    - `OCAUTOUPDS.EXE`
    - `AGNTSVC.EXEAGNTSVC.EXE`
    - `AGNTSVC.EXEENCSVC.EXE`
    - `FIREFOXCONFIG.EXE`
    - `TBIRDCONFIG.EXE`
    - `MYDESKTOPQOS.EXE`
    - `OCOMM.EXE`
    - `MYSQLD.EXE`
    - `MYSQLD-NT.EXE`
    - `MYSQLD-OPT.EXE`
    - `DBENG50.EXE`
    - `SQBCORESERVICE.EXE`
    - `SCALC.EXE`
    - `INFOPATH.EXE`
    - `SBASE.EXE`
    - `ONENOTE.EXE`
    - `OUTLOOK.EXE`
    - `SIMPRESS.EXE`
    - `STEAM.EXE`
    - `THEBAT.EXE`
    - `THEBAT64.EXE`
    - `THUNDERBIRD.EXE`
    - `SDRAW.EXE`
    - `SWRITER.EXE`
    - `WORDPAD.EXE`
    - `SOFFICE.BIN`
  - The second thread recursively searches for files, converting the name of the folder/file to uppercase and checking it against a harcoded list of hashes to avoid
    - Skips the following folders:
      - `WINDOWS`
      - `BOOT`
      - `PROGRAM FILES`
      - `PROGRAM FILES (X86)`
      - `ALL USERS`
      - `LOCAL SETTINGS`
      - `PROGRAMDATA`
      - `TOR BROWSER`
      - `APPDATA`
      - `CHROME`
      - `SYSTEM VOLUME INFORMATION`
      - `PERFLOGS`
      - `MOZILLA`
      - `OPERA`
      - `GOOGLE`
      - `WINDOWS NT`
      - `MICROSOFT`
      - `INTEL`
      - `APPLICATION DATA`
      - `INTERNET EXPLORER`
      - `COMMON FILES`
      - `$RECYCLE.BIN`
      - `ALL USERS`
      - `$WINDOWS.~BT`
      - `$WINDOWS.~WS`
      - `WINDOWS JOURNAL`
      - `MSBUILD`
    - Skips the following filenames:
      - `README!_CI0P!.TXT`
      - `NTDLR`
      - `BOOT.INI`
      - `NTUSER.INI`
      - `AUTOEXEC.BAT`
      - `NTDETECT.COM`
      - `DESKTOP.INI`
      - `AUTORUN.INF`
      - `NTUSER.DAT`
      - `ICONCACHE.DB`
      - `BOOTSECT.BAK`
      - `NTUSER.DATA.LOG`
      - `THUMBS.DB`
    - Skips the following file extensions:
      - `.C_I0P`
      - `.DLL`
      - `.EXE`
      - `.SYS`
      - `.OCX`
      - `.LNK`
  - The second thread drops ransom note, `Readme!_CI0P!.txt`, in affected folders<sup>[2](https://www.cybereason.com/blog/research/cybereason-vs.-clop-ransomware),[3](https://sectrio.com/deconstructing-cl0p-ransomware-moveit-2023-breach/),[4](https://unit42.paloaltonetworks.com/clop-ransomware/),[5](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)</sup>
    - Only drops note if it doesn't already exist in folder
  - The second thread then encrypts files
    - Note: For public release encryption capabilities have been removed.
    - Adds the `.C_I0p` extension to encrypted files
    - Sets the "archive" file attribute on the file
- Clears event viewer log files<sup>[3](https://sectrio.com/deconstructing-cl0p-ransomware-moveit-2023-breach/)</sup>
- Terminates itself and deletes the binary<sup>[3](https://sectrio.com/deconstructing-cl0p-ransomware-moveit-2023-breach/)</sup>
  - Uses the `TerminateProcess` API call
  - `cmd.exe /c timeout 7 & del path_to_binary`

## Build

Cl0p can be built in Debug or Release mode using the included
`CMakePreset.json` configurations.

The following cmake options in `cmake\config.cmake` are also provided to customize the build process:

- `CL0P_DEBUG`: Enable debug logging for Cl0p, which will log to both terminal and the log file (default `ON`)

### Dependencies

- `CMake` version `3.26`
- `CMakePresets.json` version `6` support (Visual Studio 17.7)

### Development

To develop using Visual Studio, I had to:

- Upgrade to the latest Visual Studio 2022 (`17.8.6`)
- Install the following components:
  - Desktop Development with C++
  - Test Adapter for Google Test
- Uninstall the following components:
  - Intellicode

If you installed CMake only in Visual Studio using the C++ component described
above, you can open a terminal for the commands in the next section by going to
`Tools` -> `Command Line` -> `Developer Command Prompt`.

To calculate hashes of process names to be terminated, a utility has been
included. Run `build\src\Debug\proc-hash.exe` to print out the list of hashes,
which can then copied into `defense.cpp`. To calculate hashes for an arbitrary
list of process names, run the same program and pass in the process names as
arguments, e.g. `proc-hash.exe calc.exe firefox.exe notepad.exe`.

### Quickstart

#### Command Line

Build both Debug and Release configurations of Cl0p. This will also kick off
the GoogleTest unit tests.

```PowerShell
cd Resources\Cl0p
cmake.exe --workflow --preset cicd-debug
cmake.exe --workflow --preset cicd-release
```

For simplicity, all of the generated artifacts can be bundled into the
top-level `install/` directory using the CMake installation facility.

```PowerShell
cmake.exe --install ./build --config release
cmake.exe --install ./build --config debug
```

You can then run Cl0p from the command line:

```PowerShell
.\install\Debug\main.exe

.\install\Release\main.exe
```

## Install

Installing Cl0p simply copies all of the artifacts from both Release
and Debug builds into the top-level `install/` directory. This is simply for
ease of use and distributability.

The final product should be a top-level directory named `install/` with the
following layout:

```PowerShell

Cl0p/
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

For operator usability, Cl0p will log XOR-encrypted and base64-encoded logs to `cbug.log`
in the current working directory. The current XOR key is `a44ed853c20210fc17c789c7cf226ea361180879f89f4cd54eea1f7fb7ea6dcc646a39c922959f63bae696838e068966f73af671ab242ec875b427fda9ffc10f`

You can decrypt these logs using the [log_decryptor](../log_decryptor/):

```bash
python3 aes_base64_log_decryptor.py --xor -i cbug.log  -o decrypted.log -k `a44ed853c20210fc17c789c7cf226ea361180879f89f4cd54eea1f7fb7ea6dcc646a39c922959f63bae696838e068966f73af671ab242ec875b427fda9ffc10f`
```

## CTI

1. <https://www.securin.io/articles/all-about-clop-ransomware/>
1. <https://www.cybereason.com/blog/research/cybereason-vs.-clop-ransomware>
1. <https://sectrio.com/deconstructing-cl0p-ransomware-moveit-2023-breach/>
1. <https://unit42.paloaltonetworks.com/clop-ransomware/>
1. <https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/>
1. <https://www.trendmicro.com/vinfo/us/security/news/ransomware-spotlight/ransomware-spotlight-clop>
